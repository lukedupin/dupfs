#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <stdio.h>
#include <stdlib.h>

#include "fuse_tracker.h"
#include "svn_xml_reader.h"

  //Called to run a CLI program
int FuseTracker::runCmd( QString prog )
{
  int result;

    //Run the users command
  result = system( QString("/bin/bash -c \"%1 &> /dev/null\"").arg( prog ).toAscii().data() );
  qDebug( "%s", QString("/bin/bash -c \"%1 &> /dev/null\"").arg( prog ).toAscii().data() );

  return result;
}

  //Init my desktop tracker
FuseTracker::FuseTracker()
{
    //Ensure all my pointers are safely set
  Config = NULL;
  Server = NULL;
  Svn_Update = NULL;
  Status_Timer = NULL;

    //My other variables
  Last_Task_Count = 0;
  My_Port = -1;
  Timer_Commit_Started = false;
  Timer_Count = 0;
  Thread_Running = false;
  Thread_Idle = true;

    //create my common regex
  RegEx_Dup.setPattern("^/[.]dupfs_sync");
  RegEx_Special.setPattern("([!$# ])");
  
    //Set my status to not doing anything
  Status = WATCHING;
  New_Status = WATCHING;

    //Set my operation mode to normal
  Op_Mode = OP_SYNC_MODE;
}

  //Return the status of the system
int FuseTracker::status()
{
  return Status;
}

  //Return the operation mode of the system
FuseTracker::OperationMode FuseTracker::opMode()
{
  return Op_Mode;
}

  //Return the config
OrmLight* FuseTracker::config()
{
  return Config;
}

  //Store my new config
OrmLight* FuseTracker::setConfig( OrmLight* config )
{
    //Delete any configs I already had
  if ( Config != NULL ) delete Config;

    //Store my config and return it
  return Config = config;
}

  //Start a server on the requested port
bool FuseTracker::startServer( int port )
{
    //Don't let the user create more than one
  if ( Server != NULL )
    return false;

    //Create my server
  Server = new QTcpServer(this);

    //Connect my new connection slot
  connect( Server, SIGNAL(newConnection()), this, SLOT(newConnection()) );
  
    //Open up a port
  My_Port = port;
  while ( !Server->listen( QHostAddress::Any, My_Port ) )
    My_Port++;

    //Craete my svn update timer
  Svn_Update = new QTimer();
  connect( Svn_Update, SIGNAL(timeout()), this, SLOT(updateSVN()) );
  
    //Start my timer
  Svn_Update->start( SVN_TIMEOUT );

    //Start my status update timer
  Status_Timer = new QTimer();
  connect( Status_Timer, SIGNAL(timeout()), this, SLOT(updateStatus()) );

    //Start my timer
  Status_Timer->start( STATUS_TIMEOUT );

    //Create my UDP listen socket
  Udp_Socket = new QUdpSocket(this);
  Udp_Socket->bind( (*Config)["external_udp_port"].toInt() );

    //Connect my udp listen data ready event
  connect(Udp_Socket, SIGNAL(readyRead()), this, SLOT(readPendingUdpRequest()));

    //Emit my operation mode when the system starts
  emit opModeChanged( Op_Mode );

    //Connect my loaded fuse timer
  QTimer::singleShot( LOADED_FUSE_COUNT, this, SLOT(loadAfterFuseBooted()));

  return true;
}

  //Return the port we connect on
int FuseTracker::getPort()
{
  return My_Port;
}

  //Called when a new command is sent to me
void FuseTracker::gotCommand( FuseCppInterface::NotableAction action, 
                              QString path )
{
  int rm;
  int add;
  QString my_path;
  bool avoid_update = false;

    //If the path is doubling up on the dupfs sync, then kill one
  path = path.replace( RegEx_Dup, "");
  my_path = QString("%1%2").arg( Mounted).arg( path).replace( RegEx_Special, "\\\\\\1");

    //handle an actino
  switch ( action )
  {
      //Delete
    case FuseCppInterface::UNLINK:
    case FuseCppInterface::RMDIR:
      addStatus( ADDING_ITEMS );

        //Run the svn command
      rm = runCmd( QString("/usr/bin/svn remove %1").arg(my_path) );

        //Add this new item
      if ( rm != 0 )
        Updated_Items.remove(my_path);
      break;

      //Add new files
    case FuseCppInterface::SYMLINK:
    case FuseCppInterface::HARDLINK:

    case FuseCppInterface::MKNOD:
    case FuseCppInterface::MKDIR:
    case FuseCppInterface::CREATE:
    case FuseCppInterface::OPEN:
      addStatus( ADDING_ITEMS );

        //Run the svn command
      add = runCmd( QString("/usr/bin/svn add %1").arg(my_path) );

        //Add this new item
      if ( add == 0 )
        Updated_Items[my_path] = OrmLight();
      break;

      //These operations mean we need to update our data at some point
    case FuseCppInterface::CHMOD:
    case FuseCppInterface::CHOWN:
    case FuseCppInterface::TRUNCATE:
    case FuseCppInterface::FTRUNCATE:
    case FuseCppInterface::UTIMES:
    case FuseCppInterface::WRITE:
    case FuseCppInterface::FLUSH:
    case FuseCppInterface::CLOSE:
    case FuseCppInterface::FSYNC:
    case FuseCppInterface::SETXATTR:
    case FuseCppInterface::REMOVEXATTR:
        //Add this new item for update
      Updated_Items[my_path] = OrmLight();
      break;

      //Handle an svn update
    case FuseCppInterface::SVN_COMMIT:
      //qDebug("Running svn commit");
        //Don't allow this action to cause a new commit
      avoid_update = true;

      removeStatus( SYNC_PUSH_REQUIRED );
      addStatus( SYNC_PUSH );

        //If there aren't any updated items, quit out now
      if ( Updated_Items.count() <= 0 )
        return;

        //handle our sync mode
      if ( Op_Mode == OP_SYNC_MODE )
      {
        updateRev();

          //Conduct an update before we commit to avoid conflicts
        QStringList list = updateFiles();
        if ( list.size() > 0 )
          runCmd( QString("/usr/bin/svn update --force --non-interactive --accept mine-full %1").arg(list.join(" ")));

          //Make a list of all the files to be updated
        QString files = QStringList( Updated_Items.keys() ).join(" ");

          //Make it happen
        runCmd( QString("/usr/bin/svn ci -m \\\"Updated %1 Items\\\" --non-interactive --depth immediates %2").arg( Updated_Items.count() ).arg( files ) );

          //Clear out all the now updated items
        QFile::remove( QString("%1/.dupfs_action_log").
                              arg((*Config)["svn_dir"]));
        Updated_Items.clear();
      }
            //Store a local file of the changes to be made at a later time
      else if ( Op_Mode == OP_OFFLINE_MODE )
      {
          //save my list of files requiring a change to the FS
        QString filename = QString("%1/.dupfs_action_log").
                                arg((*Config)["svn_dir"]);
        Updated_Items.saveToFile( filename );
      }

        //Remove my push stats
      removeStatus( SYNC_PUSH );
      break;

      //Check if its a special command
    case FuseCppInterface::SVN_UPDATE:
      //qDebug("Running svn update");
        //Don't allow this action to cause a new commit
      avoid_update = true;

        //Only do this if we aren't offline
      if ( Op_Mode != OP_OFFLINE_MODE )
      {
        removeStatus( SYNC_PULL_REQUIRED );
        addStatus( SYNC_PULL );

          //Issue my update command
        QStringList list = updateFiles();
        runCmd( QString("/usr/bin/svn update --force --non-interactive --accept mine-full %1").arg(list.join(" ")));

        removeStatus( SYNC_PULL );
      }
      break;


      //not sure, just ignore
    case FuseCppInterface::RENAME:
    default:
      return;
      break;
  };

    //If we got here, we know we got a valid command
    //If timer isn't started, start it and then quit out
  if ( !avoid_update && !Timer_Commit_Started )
  {
    addStatus( SYNC_PUSH_REQUIRED );
    Timer_Count = 0;
    Timer_Commit_Started = true;
  }
}

  //Store the moutned directories
void FuseTracker::setMounted( QString mounted )
{
  Mounted = mounted;
}

  //Runs a thread
void FuseTracker::run()
{
  QPair<FuseCppInterface::NotableAction, QString> item;
  QStringList list;
  int fails = 0;

    //My main thread loopto handle data
  Thread_Running = true;
  Thread_Idle = true;
  while ( Thread_Running )
  {
      //Run these commands while there is data
    while ( Data_Read.count() > 0 )
    {
        //Set that the thread isn't idel right now
      Thread_Idle = false;
  
        //Reset my failed read count
      fails = 0;

        //Pull a line from the data list
      if ( (item = Data_Read.takeFirst()).second.isEmpty() )
        continue;

        //Remove any existances of this guy from the hash
      Data_Hash.remove( item.second );

        //Handle the command
      gotCommand( item.first, item.second );
    }

      //Set that the thread isn't doing anything
    Thread_Idle = true;

      //Remove any status stuff that might exists
    removeStatus( SYNC_PUSH );
    removeStatus( SYNC_PULL );
    removeStatus( ADDING_ITEMS );

      //Sleep for a little while waiting for more data
    usleep( THREAD_SLEEP );

      //Check if we've failed too many times
    if ( ++fails >= THREAD_FAILED )
      Thread_Running = false;
  }

    //Ensure that the thread is idle, this is over kill and probably not needed
  Thread_Idle = true;
}

  //Called one time after the fuse interface has booted
void FuseTracker::loadAfterFuseBooted()
{
    //Attempt to load an old sync file from another time
  loadSyncLog( true );

    //Issue an svn update to ensure we are synced
  forceUpdate();
}

  //Load up a sync log file
void FuseTracker::loadSyncLog( bool change_state )
{
  OrmLight *result = NULL;
  QString filename = QString("%1/.dupfs_action_log").arg((*Config)["svn_dir"]);

    //Attempt to load our sync_log file
  try {
    result = OrmLight::loadFromFile( filename, &Updated_Items);
  } 
  catch ( QString str ) {
    qDebug( "%s", str.toAscii().data() );
  }

    //If we aren't going to change states, just quit now
  if ( !change_state || result == NULL )
    return;

    //Update my state information since we got new data
  setOpMode( OP_OFFLINE_MODE );

    //If timer isn't started, start it and then quit out
  if ( !Timer_Commit_Started )
  {
    addStatus( SYNC_PUSH_REQUIRED );
    Timer_Count = 0;
    Timer_Commit_Started = true;
  }
}

  //Called to touch the rev file
void FuseTracker::updateRev()
{
  QString my_path = QString("%1/%2").arg(Mounted)
                                    .arg((*Config)["track_filename"])
                                    .replace( RegEx_Special, "\\\\\\1");
  QFile file( my_path );
  int add;

    //Write out the file
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

    //Update the file with some shiz
  file.write( QDateTime::currentDateTime().toString().toAscii() );

  file.close();

    //Ensure this file is added
  add = runCmd( QString("/usr/bin/svn add %1").arg(my_path) );

    //Add myself to the update shizzel
  Updated_Items[my_path] = OrmLight();
}

  //Called to figure out what files are out of date
QStringList FuseTracker::updateFiles()
{
  int i;
  int r;
  QString my_path = QString("%1/%2").arg(Mounted)
                                    .arg((*Config)["track_filename"])
                                    .replace( RegEx_Special, "\\\\\\1");
  OrmLight revs;
  QStringList files;
  QHash<QString, bool> dup;
  QString value;

    //Figure out what files we need
  revs = SvnXmlReader::readSvnXml( 
            QString("/usr/bin/svn log --xml -v -r BASE:HEAD %1").arg(my_path),
            SvnXmlReader::SVN_XML_LOG );

    //Load up the revs into a string list and sort it
  QStringList list = QStringList( revs.keys() );
  //qDebug( "%s", list.join(" ").toAscii().data() );
  list.sort();

    //Now we go through all revs that we need to update nad make a file list
    //We skip the first one since that is always the current log entry
  for ( i = 1; i < list.size(); i++ )
    for ( r = 0; r < revs[list[i]]["paths"].size(); r++ )
    {
      value = QString("%1%2").arg(Mounted).arg(revs[list[i]]["paths"][r]).replace( RegEx_Special, "\\\\\\1");
      if ( !dup.contains( value ) )
      {
        dup[value] = true;
        files.push_back( value );
      }
    }

    //Always push my track file onto the list of updates
  files.push_back( my_path );

  return files;
}

  //Set the oepration mode of the system
void FuseTracker::setOpMode( OperationMode mode )
{
    //Check for invalid modes
  if ( mode == OPERATION_MODE_COUNT )
    return;

    //Check if we should emit a signal
  if ( Op_Mode != mode )
    emit opModeChanged( mode );
    
    //Store the new op mode
  Op_Mode = mode;
}

  //Set the status of our system
void FuseTracker::setStatus( SystemStatus status )
{
    //If this is a new status, set it
  if ( New_Status != status )
    New_Status = status;
}

  //Add a status to the current one
void FuseTracker::addStatus( SystemStatus status )
{
  if ( !(New_Status & status) )
    New_Status = (New_Status | status);
}

  //Remove a status to the current one
void FuseTracker::removeStatus( SystemStatus status )
{
  if ( (New_Status & status) )
    New_Status = (New_Status ^ status);
}

  //Got a new connection
void FuseTracker::newConnection()
{
  Client = Server->nextPendingConnection();
  connect( Client, SIGNAL(readyRead()), this, SLOT(readyRead()) );
}

  //Called when there is new data from the filesystem
void FuseTracker::readyRead()
{
  FuseCppInterface::NotableAction action;
  QStringList list;
  QString path;
  QString from;
  int i;
  int len;

    //Loop while there is data to read
  do {
    QStringList list;

      //Read a line of data from the socket
    QString line = QString( Client->readLine()).replace(QRegExp("[\n\r]"), "");
    //qDebug("%s", line.toAscii().data() );

      //Quit if this line isn't valid
    if ( line.indexOf( QRegExp("^[0-9]+,.+$")) < 0 )
      continue;

      //Split up the data and ensure we have enough items, otherwise read more
    list = line.split(QRegExp(","));
    if ( list.count() < 2 )
      continue;

      //If my thread isn't started then start it
    if ( !Thread_Running )
      this->start();

      //Pull out my action and path
    action = (FuseCppInterface::NotableAction)list[0].toInt();
    path = list[1];

      //Run a first pass on my actino type
    switch ( action )
    {
        //Handle the from clause for a rename first, then the normal way
      case FuseCppInterface::RENAME:
          //Now we lie and say the action is a create action
        action = FuseCppInterface::CREATE;

          //If we have the from param, handle that before we handle path
        if ( list.count() == 3 )
        {
          from = list[2];

            //If this guy exists, remove any instance of him first
          if ( Data_Hash.contains( from ) )
          {
              //Kill any occurance of path existing
            for ( i = 0, len = Data_Read.count(); i < len; i++ )
              if ( Data_Read[i].second == from )
              {
                Data_Read.removeAt(i);
                --i;
                --len;
              }
          }

            //Set that this path does currently have a dangerous action pending
          Data_Hash[from] = true;

            //Add the dangerous action that is pending
          Data_Read.push_back( qMakePair( FuseCppInterface::UNLINK, from) );
        }

        //None safe operations
      case FuseCppInterface::UNLINK:
      case FuseCppInterface::RMDIR:

      case FuseCppInterface::SYMLINK:
      case FuseCppInterface::HARDLINK:
      case FuseCppInterface::MKNOD:
      case FuseCppInterface::MKDIR:
      case FuseCppInterface::CREATE:
      case FuseCppInterface::OPEN:
          //If this guy exists, remove any instance of him first
        if ( Data_Hash.contains( path ) )
        {
            //Kill any occurance of path existing
          for ( i = 0, len = Data_Read.count(); i < len; i++ )
            if ( Data_Read[i].second == path )
            {
              Data_Read.removeAt(i);
              --i;
              --len;
            }
        }

          //Set that this path does currently have a dangerous action pending
        Data_Hash[path] = true;

        //The standard way of handling everything
      default:
        Data_Read.push_back( qMakePair( action, path) );
        break;
    }

  } while ( Client->canReadLine() );
}

  //Called when we are suppose to update svn
void FuseTracker::updateSVN()
{
    //Increate my counter if it should be updated
  if ( Timer_Commit_Started && Timer_Count < TIMER_COUNT_MAX )
    Timer_Count++;

    //If my timer count is too big, issue and svn update
  if ( Timer_Count >= TIMER_COUNT_MAX && Thread_Idle )
    forceCommit();
}

  //Called to update any listeners to changes in the trackers status info
void FuseTracker::updateStatus()
{
    //If our new status isn't know, then set it to watching
  if ( New_Status == UNKNOWN )
    New_Status = WATCHING;

    //Update my status variable
  if ( Status != New_Status )
  {
    Status = New_Status;
    emit statusChanged( Status );
  }

    //If there was a change in the task count alert the user
  if ( Last_Task_Count != Data_Read.count() )
  {
    Last_Task_Count = Data_Read.count();
    emit tasksRemaining( Last_Task_Count );
  }
}

  //When called, a commit command is issued right now
void FuseTracker::forceCommit()
{
  //qDebug("Fork Commit has been cizalled");

    //Push a special command onto the stack
  Data_Read.push_back( qMakePair( FuseCppInterface::SVN_COMMIT, 
                                  QString::fromUtf8("NONE")) );

    //If my thread isn't started then start it
  if ( !Thread_Running )
    this->start();

    //Reset my variables
  Timer_Commit_Started = false;
  Timer_Count = 0;
}

  //When called, a commit command is issued right now
void FuseTracker::forceUpdate()
{
    //Push a special command onto the stack
  addStatus( SYNC_PULL_REQUIRED );
  Data_Read.push_back( qMakePair( FuseCppInterface::SVN_UPDATE, 
                                  QString::fromUtf8("NONE")) );

    //If my thread isn't started then start it
  if ( !Thread_Running )
    this->start();
}

  //Read pending udp request
void FuseTracker::readPendingUdpRequest()
{
    //Loop while we have valid data
  while (Udp_Socket->hasPendingDatagrams()) 
  {
    QHostAddress sender;
    quint16 senderPort;
    QByteArray datagram;
    datagram.resize( Udp_Socket->pendingDatagramSize());

      //Read the datagram waiting for me
    Udp_Socket->readDatagram(datagram.data(), datagram.size(),
                             &sender, &senderPort);

      //Check if we are requested to update
    QRegExp rx("^SVN UPDATE REQUESTED (\\d+)");
    if ( rx.indexIn(QString(datagram.data())) >= 0 )
    {
      QStringList list = rx.capturedTexts();

      //TODO protect agains a dick sending a bunch of udp update commands

        //Push a special command onto the stack
      addStatus( SYNC_PULL_REQUIRED );
      Data_Read.push_back( qMakePair( FuseCppInterface::SVN_UPDATE, rx.cap(1)));

        //If my thread isn't started then start it
      if ( !Thread_Running )
        this->start();
    }
  }
}
