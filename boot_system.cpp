#include "boot_system.h"

#include "svn_xml_reader.h"
#include <QCoreApplication>

#include <QTimer>

  //Called to add the interface
void BootSystem::addInterface( FuseCppInterface* interface )
{
  Fuse_Interface = interface;
}

  //Called to add the tracker
void BootSystem::addTracker( FuseTracker* tracker )
{
  Fuse_Tracker = tracker;
}

  //Assigns my configuration information
void BootSystem::addConfig( OrmLight* config )
{
  Config = config;
}

  //Called to create a timer that will start my system
void BootSystem::registerSystem()
{
      //Register to start the system
  QTimer::singleShot(200, this, SLOT(bootSystem()));
}


  //Boot up the system
void BootSystem::bootSystem()
{
  bool port_ok;
  int port = -1;

    //Figure out waht port we are going to start on
  port = (*Config)["internal_server_port"].toInt( &port_ok );
  if ( !port_ok || port <= 0 )
    port = FuseCppInterface::CONNECT_PORT;

    //Start the tracker's server
  if ( !Fuse_Tracker->startServer( port ) )
  {
    qCritical( "Error starting TCP Server on port: %d", port );
    return;
  }

    //set the port that we will talk over
  Fuse_Interface->setPort( Fuse_Tracker->getPort() );

    //Start my main fuse loop
  if ( fork() == 0 )
    Fuse_Interface->startMain();
}
