#include "fuse_tracker.h"

#include <QMessageBox>
#include <QRegExp>
#include <QStringList>
#include <QTimer>
#include <stdlib.h>

  //Init my desktop tracker
FuseTracker::FuseTracker()
{
    //Ensure all my pointers are safely set
  Config = NULL;
  Server = NULL;

    //My other variables
  My_Port = -1;
  Timer_Started = false;
  
    //Set my status to not doing anything
  Status = WATCHING;
}

  //Return the status of the system
FuseTracker::SystemStatus FuseTracker::status()
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
  qDebug("*%d*", action);
  int result;

    //handle an actino
  switch ( action )
  {
      //Rename
    case FuseCppInterface::RENAME:
      qDebug( "%s", QString("svn add %1%2").arg(Mounted).arg(path).toAscii().data() );
      result = system( QString("svn add %1%2").arg(Mounted).arg(path).toAscii().data() );
      qDebug( "%s", QString("svn remove %1%2").arg(Mounted).arg(from).toAscii().data() );
      result = system(QString("svn remove %1%2").arg(Mounted).arg(from).toAscii().data());
      break;

      //links
    case FuseCppInterface::SYMLINK:
    case FuseCppInterface::HARDLINK:
      qDebug( "%s", QString("svn add %1%2").arg(Mounted).arg(path).toAscii().data() );
      result = system( QString("svn add %1%2").arg(Mounted).arg(path).toAscii().data() );
      break;

      //Delete
    case FuseCppInterface::UNLINK:
    case FuseCppInterface::RMDIR:
      qDebug( "%s", QString("svn remove %1%2").arg(Mounted).arg(path).toAscii().data() );
      result = system(QString("svn remove %1%2").arg(Mounted).arg(path).toAscii().data());
      break;

      //Add new files
    case FuseCppInterface::MKNOD:
    case FuseCppInterface::MKDIR:
    case FuseCppInterface::CREATE:
      qDebug( "%s", QString("svn add %1%2").arg(Mounted).arg(path).toAscii().data() );
      result = system(QString("svn add %1%2").arg(Mounted).arg(path).toAscii().data());
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
      qDebug( "%s", QString("Update required %1%2").arg(Mounted).arg(path).toAscii().data() );
      if ( !Timer_Started )
      {
        Timer_Started = true;
        QTimer::singleShot( TIMEOUT, this, SLOT(updateSVN()));
      }
      break;

      //not sure, just ignore
    default:
      qDebug( "%s", QString("Not implemented %1%2").arg(Mounted).arg(path).toAscii().data() );
      break;
  };
}

  //Store the moutned directories
void FuseTracker::setMounted( QString mounted )
{
  Mounted = mounted;
}

  //Set the status of our system
void FuseTracker::setStatus( SystemStatus status )
{
    //If this is a new status, set it
  if ( Status != status )
  {
    Status = status;
    emit statusChanged( Status );
  }
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
    QString line = QString( Client->readLine()).replace(QRegExp("[\n\r]"), "");
    QStringList list;

      //Quit if this line isn't valid
    if ( line.indexOf( QRegExp("^@[0-9]+,.+#$")) < 0 )
      continue;

      //split up the data and read out the command sent to us
    line = line.replace(QRegExp("@"), "").replace(QRegExp("#"), "");
    list = line.split(QRegExp(","));

      //Got the command
    if ( list.count() == 2 )
      gotCommand( (FuseCppInterface::NotableAction)list[0].toInt(), list[1] );
    else if ( list.count() == 3 )
      gotCommand( (FuseCppInterface::NotableAction)list[0].toInt(), list[1], list[2] );
  } while ( Client->canReadLine() );
}

  //Called when we are suppose to update svn
void FuseTracker::updateSVN()
{
  {
    int result;
    result = system( QString("cd %1; svn ci -m \"Filesync\"").arg(Mounted).toAscii().data() );
  }

  Timer_Started = false;
}
