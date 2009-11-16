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
                              QString path, QString from )
{
  int rm;
  int add;
  QString my_path;
  QString my_from;

    //If the path is doubling up on the dupfs sync, then kill one
  path = path.replace( RegEx_Dup, "");
  my_path = QString("%1%2").arg( Mounted).arg( path).replace( RegEx_Special, "\\\\\\1");

    //Handle the from information
  if ( !from.isEmpty() )
  {
    from = from.replace( RegEx_Dup, "");
    my_from = QString("%1%2").arg( Mounted).arg( from).replace( RegEx_Special, "\\\\\\1");
  }

    //handle an actino
  switch ( action )
  {
      //Rename
    case FuseCppInterface::RENAME:
      addStatus( ADDING_ITEMS );

        //Run the svn comands
      add = runCmd( QString("/usr/bin/svn add %1").arg(my_path) );
      rm = runCmd( QString("/usr/bin/svn remove --force %1").arg(my_from) );
      
        //Add these to my list of changes
      if ( add == 0 )
        Updated_Items[my_path] = OrmLight();
      if ( rm != 0 )
        Updated_Items.remove(my_from);
      break;

      //links
    case FuseCppInterface::SYMLINK:
    case FuseCppInterface::HARDLINK:
      addStatus( ADDING_ITEMS );

        //Run the svn comands
      add = runCmd( QString("/usr/bin/svn add %1").arg(my_path) );

        //Add this new item
      if ( add == 0 )
        Updated_Items[my_path] = OrmLight();
      break;

      //Delete
    case FuseCppInterface::UNLINK:
    case FuseCppInterface::RMDIR:
      addStatus( ADDING_ITEMS );

        //Run the svn command
      rm = runCmd( QString("/usr/bin/svn remove --force %1").arg(my_path) );

        //Add this new item
      if ( rm != 0 )
        Updated_Items.remove(my_path);
      break;

      //Add new files
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

      //not sure, just ignore
    default:
      return;
      break;
  };

    //If we got here, we know we got a valid command
    //If timer isn't started, start it and then quit out
  if ( !Timer_Commit_Started )
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
  QString line;
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
      if ( (line = Data_Read.takeFirst()).isEmpty() )
        continue;

      //qDebug("%s", line.toAscii().data() );

        //Check if its a special command
      if ( line.indexOf( QRegExp("SVN COMMIT")) >= 0 )
      {
        removeStatus( SYNC_PUSH_REQUIRED );
        addStatus( SYNC_PUSH );

          //Depending on my mode, we will handle our commits differently
        if ( Updated_Items.count() > 0 )
          switch ( Op_Mode )
          {
              //Connect to the server and upload our changes
            case OP_SYNC_MODE: {
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
            } break;

              //Store a local file of the changes to be made at a later time
            case OP_OFFLINE_MODE: {
               //save my list of files requiring a change to the FS
              QString filename = QString("%1/.dupfs_action_log").
                                  arg((*Config)["svn_dir"]);
              Updated_Items.saveToFile( filename );
            } break;

              //not sure what op we are in
            default: break;
          }

          //Remove my push stats
        removeStatus( SYNC_PUSH );
      }

        //Check if its a special command
      else if ( line.indexOf( QRegExp("SVN UPDATE")) >= 0 )
      {
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
      }

        //Run a normal command passing it to the list of svn command handlers
      else
      {
          //split up the data and read out the command sent to us
        list = line.split(QRegExp(","));

          //Got the command
        if ( list.count() == 2 )
          gotCommand( (FuseCppInterface::NotableAction)list[0].toInt(), list[1] );
        else if ( list.count() == 3 )
          gotCommand( (FuseCppInterface::NotableAction)list[0].toInt(), list[1], list[2] );
      }
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

    //Figure out what files we need
  revs = SvnXmlReader::readSvnXml( 
            QString("/usr/bin/svn log --xml -v -r BASE:HEAD %1").arg(my_path),
            SvnXmlReader::SVN_XML_LOG );

    //Load up the revs into a string list and sort it
  QStringList list = QStringList( revs.keys() );
  qDebug( "%s", list.join(" ").toAscii().data() );
  list.sort();

    //Now we go through all revs that we need to update nad make a file list
    //We skip the first one since that is always the current log entry
  for ( i = 1; i < list.size(); i++ )
    for ( r = 0; r < revs[list[i]]["paths"].size(); r++ )
      files.push_back( QString("%1%2").arg(Mounted).arg(revs[list[i]]["paths"][r]).replace( RegEx_Special, "\\\\\\1") );

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
  do {
    QStringList list;

      //Read a line of data from the socket
    QString line = QString( Client->readLine()).replace(QRegExp("[\n\r]"), "");

      //Quit if this line isn't valid
    if ( line.indexOf( QRegExp("^[0-9]+,.+$")) < 0 )
      continue;

      //If my thread isn't started then start it
    if ( !Thread_Running )
      this->start();

      //Push this data onto my data list
    Data_Read.push_back( line );

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
    //Push a special command onto the stack
  Data_Read.push_back( QString::fromUtf8("SVN COMMIT") );

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
  Data_Read.push_back( QString::fromUtf8("SVN UPDATE") );

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
      Data_Read.push_back( QString::fromUtf8("SVN UPDATE") );

        //If my thread isn't started then start it
      if ( !Thread_Running )
        this->start();
    }
  }
}
