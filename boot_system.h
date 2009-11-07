#ifndef BOOT_SYSTEM
#define BOOT_SYSTEM

#include <QObject>

#include "window.h"
#include "fuse_tracker.h"
#include "fuse_cpp_interface.h"
#include "orm_light/orm_light.h"

  //! \brief Starts up the system
class BootSystem: public QObject
{
  Q_OBJECT

  public:

  private:
    //! \brief Holds my fuse interface
  FuseCppInterface* Fuse_Interface;
    //! \brief Holds my fuse tracker
  FuseTracker*      Fuse_Tracker;
    //! \brief Holds my configuration information
  OrmLight*         Config;

  public:
    //! \brief Assign the fuse interface
  void addInterface( FuseCppInterface* interface );

    //! \brief Assign the fuse tracker
  void addTracker( FuseTracker* tracker );

    //! \brief Assign the config information
  void addConfig( OrmLight* config );

    //! \brief Register the system
  void registerSystem();

  public slots:

    //! \brief Called from within a valid qt thread to boot the system
  void bootSystem();

  signals:
};

#endif
