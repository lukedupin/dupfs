#include "fuse_tracker.h"

#include <QMessageBox>
#include <QRegExp>
#include <stdlib.h>

  //Called to run a CLI program
void FuseTracker::runCmd( QString prog )
{
  int result;
  result = system(QString("/bin/bash -c \"%1\"").arg( prog ).toAscii().data());
  //result = system( QString("/bin/bash -c \"%1 &> /dev/null\"").arg( prog ).toAscii().data() );
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
  Timer_Started = false;
  Timer_Count = 0;
  Thread_Running = false;
  Thread_Idle = true;
  
    //Set my status to not doing anything
  Status = WATCHING;
  New_Status = WATCHING;
}

  //Return the status of the system
int FuseTracker::status()
{
  return Status;
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

  return true;
}

  //Return the port we connect on
int FuseTracker::getPort()
{
  return My_Port;
}

  //Called when a new command is sent to me
void FuseTracker::gotCommand( FuseCppInterface::NotableAction action, QString path, QString from )
{
  //qDebug("*%d*", action);

    //If the path is doubling up on the dupfs sync, then kill one
  path = path.replace(QRegExp("^/[.]dupfs_sync"), "");
  if ( !from.isEmpty() )
    from = from.replace(QRegExp("^/[.]dupfs_sync"), "");
   //qDebug( "%s", QString("svn * %1%2").arg(Mounted).arg(path).toAscii().data() );

    //handle an actino
  switch ( action )
  {
      //Rename
    case FuseCppInterface::RENAME:
      addStatus( ADDING_ITEMS );

        //Run the svn comands
      runCmd( QString("/usr/bin/svn add %1%2").arg(Mounted).arg(path) );
      runCmd( QString("/usr/bin/svn remove %1%2").arg(Mounted).arg(from) );
      break;

      //links
    case FuseCppInterface::SYMLINK:
    case FuseCppInterface::HARDLINK:
      addStatus( ADDING_ITEMS );

        //Run the svn comands
      runCmd( QString("/usr/bin/svn add %1%2").arg(Mounted).arg(path) );
      runCmd( QString("/usr/bin/svn add %1%2").arg(Mounted).arg(path) );
      break;

      //Delete
    case FuseCppInterface::UNLINK:
    case FuseCppInterface::RMDIR:
      addStatus( ADDING_ITEMS );

        //Run the svn command
      runCmd( QString("/usr/bin/svn remove %1%2").arg(Mounted).arg(path) );
      break;

      //Add new files
    case FuseCppInterface::MKNOD:
    case FuseCppInterface::MKDIR:
    case FuseCppInterface::CREATE:
      addStatus( ADDING_ITEMS );

        //Run the svn command
      runCmd( QString("/usr/bin/svn add %1%2").arg(Mounted).arg(path) );
      break;

    case FuseCppInterface::OPEN:
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
        //If my timer isn't start then start it now
      if ( !Timer_Started )
      {
        addStatus( SYNC_PUSH_REQUIRED );
        Timer_Count = 0;
        Timer_Started = true;
      }
      break;

      //not sure, just ignore
    default:
      break;
  };
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
      if ( line.indexOf( QRegExp("SVN UPDATE")) >= 0 )
      {
        removeStatus( SYNC_PUSH_REQUIRED );
        addStatus( SYNC_PUSH );
        runCmd( QString("/usr/bin/svn ci -m \"Filesync\" %1").arg(Mounted) );
        removeStatus( SYNC_PUSH );
      }

        //Run a normal command passing it to the list of svn command handlers
      else
      {
          //split up the data and read out the command sent to us
        line = line.replace(QRegExp("@"), "").replace(QRegExp("#"), "");
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
    if ( line.indexOf( QRegExp("^@[0-9]+,.+#$")) < 0 )
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
  if ( Timer_Started && Timer_Count < TIMER_COUNT_MAX )
    Timer_Count++;

    //If my timer count is too big, issue and svn update
  if ( Timer_Count >= TIMER_COUNT_MAX && Thread_Idle )
  {
      //Push a special command onto the stack
    Data_Read.push_back( QString::fromUtf8("SVN UPDATE") );

      //If my thread isn't started then start it
    if ( !Thread_Running )
      this->start();

      //Reset my variables
    Timer_Started = false;
    Timer_Count = 0;
  }
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