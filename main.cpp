#include <unistd.h>

#include <QtGui>

#include "boot_system.h"
#include "window.h"
#include "fuse_tracker.h"
#include "fuse_cpp_interface.h"
#include "version.h"
#include "orm_light/orm_light.h"

    //Make number of attempts to get the systray
const int MAX_ATTEMPTS = 15;

int main(int argc, char *argv[])
{
  int app_argc;
  char** app_argv;
  FuseTracker* tracker;
  FuseCppInterface* fuse;
  BootSystem* boot_system;
  int attempt;
  int res;

    //If the user didn't give me a config file then quit
  if ( argc != 2 )
  {
    qCritical("Usage: %s CONFIG_FILE", argv[0] );
    return -1;
  }

    //Alloc my new tracker
  tracker = new FuseTracker();

    //Load the config file and store it into my system config
  try {
    tracker->setConfig( OrmLight::loadFromFile( argv[1] ) );
  } catch ( QString str ) { 
    qCritical("Invalid Configuration File (%s)\n\t*%s\n", argv[1],
              str.toAscii().data() ); 
    return 2;
  }

    //Mount bind my data and mount folder
  if ( (res = system((*tracker->config())["mount_bind"].toAscii().data())) != 0)
  {
    qDebug("Unable to mount data directory");
    return -2;
  }

    //Create my fuse thread
  app_argc = 4;
  app_argv = new char*[app_argc + 1];
  app_argv[0] = new char[ strlen(argv[0]) + 1 ];
  strcpy( app_argv[0], argv[0] );

  app_argv[1] = new char[ strlen("-o") + 1 ];
  strcpy( app_argv[1], "-o" );

  app_argv[2] = new char[ strlen("nonempty") + 1 ];
  strcpy( app_argv[2], "nonempty" );

  app_argv[3] = new char[(*tracker->config())["mount_dir"].size()+1];
  strcpy( app_argv[3], (*tracker->config())["mount_dir"].toAscii().data() );

  app_argv[4] = NULL;
  fuse = new FuseCppInterface( app_argc, app_argv, tracker->config() );

    //Store my mount point in the tracker
  tracker->setMounted( 
        QString("%1/.dupfs_sync").arg((*tracker->config())["mount_dir"]));

    //Get my boot class ready
  boot_system = new BootSystem();
  boot_system->addInterface( fuse );
  boot_system->addTracker( tracker );
  boot_system->addConfig( tracker->config() );

    //Load up my program
  Q_INIT_RESOURCE(systray);

    //Pass fake arguments to qt
  app_argc = 2;
  app_argv = new char*[3];
  app_argv[0] = argv[0];
  app_argv[1] = new char[(*tracker->config())["mount_dir"].size()+1];
  strcpy( app_argv[1], (*tracker->config())["mount_dir"].toAscii().data() );
  app_argv[2] = NULL;
  QApplication app( app_argc, app_argv);

    //Quit out and .... take the ball home   
  attempt = 0;
  while (!QSystemTrayIcon::isSystemTrayAvailable()) 
  {
      //If we have failed too many times then stop
    if ( ++attempt > MAX_ATTEMPTS )
    {
      QMessageBox::critical(0,QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray"
                                          " on this system."));
      return 1;  
    }

      //Sleep the program for a second
    sleep( 1 );
  }

    //Make the app not quit on a whim
  QApplication::setQuitOnLastWindowClosed(false);

  Window window( tracker );

    //Register to start the system
  boot_system->registerSystem();
  return app.exec();
}

