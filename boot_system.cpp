#include "boot_system.h"

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

  //Called to create a timer that will start my system
void BootSystem::registerSystem()
{
      //Register to start the system
  QTimer::singleShot(200, this, SLOT(bootSystem()));
}


  //Boot up the system
void BootSystem::bootSystem()
{
    //Start the tracker's server
  if ( !Fuse_Tracker->startServer( FuseCppInterface::CONNECT_PORT ) )
  {
    qCritical( "Error starting TCP Server on port: %d",
                FuseCppInterface::CONNECT_PORT );
    return;
  }

    //set the port that we will talk over
  Fuse_Interface->setPort( Fuse_Tracker->getPort() );

    //Start my main fuse loop
  if ( fork() == 0 )
    Fuse_Interface->startMain();
}
