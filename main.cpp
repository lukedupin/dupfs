#include <unistd.h>

#include <QtGui>
#include <QFile>

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
  QFile config;
  int attempt;

    //Alloc my new tracker
  tracker = new FuseTracker();

    //Load up my config
  config.setFileName( "configuration.json" );
  if ( !config.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    qCritical("%s", 
          QString("Failed to open %1").arg(config.fileName()).toAscii().data());
    return 1;
  }

    //Load the config file and store it into my system config
  try {
    tracker->setConfig( OrmLight::fromJson( QString( config.readAll())) );
  } catch ( QString str ) { 
    qCritical("Invalid Configuration File (configuration.json)\n\t*%s\n", 
              str.toAscii().data() ); 
    return 2;
  }
  config.close();

  qDebug("%s\n", (*tracker->config())[0].toAscii().data() );

    //Create my fuse thread
  app_argc = 2;
  app_argv = new char*[3];
  app_argv[0] = argv[0];
  app_argv[1] = "/tmp/test";
  app_argv[2] = NULL;
  fuse = new FuseCppInterface( app_argc, app_argv );

    //Store my mount point in the tracker
  tracker->setMounted( QString("%1/dupfs_sync").arg(QString::fromUtf8(app_argv[1])));

    //Get my boot class ready
  boot_system = new BootSystem();
  boot_system->addInterface( fuse );
  boot_system->addTracker( tracker );

    //Load up my program
  Q_INIT_RESOURCE(systray);

    //Pass fake arguments to qt
  QApplication app( argc, argv);

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

