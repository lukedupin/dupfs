#ifndef FUSE_TRACKER
#define FUSE_TRACKER

#include <QTimer>
#include <QThread>
#include <QDateTime>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QStringList>
#include <QHash>
#include <QRegExp>

#include "orm_light/orm_light.h"
#include "fuse_cpp_interface.h"

  //! \brief FuseTracker
class FuseTracker : public QThread
{
  Q_OBJECT

  public:
    //! \enum My system states
  enum SystemStatus { UNKNOWN             = 0,
                      WATCHING            = 1,
                      ADDING_ITEMS        = 2,
                      SYNC_PULL           = 4,
                      SYNC_PUSH           = 8,
                      SYNC_PULL_REQUIRED  = 16,
                      SYNC_PUSH_REQUIRED  = 32
                     };

    //! \enum The different operation modes of my system
  enum OperationMode {
    OP_SYNC_MODE,
    OP_OFFLINE_MODE,
    OPERATION_MODE_COUNT
  };

    //! \brief The timeout interval for my status updating timer
  static const int STATUS_TIMEOUT = 500;

    //! \brief The amount of time in millaseconds to wait before timeout
  static const int SVN_TIMEOUT = 1000;

    //! \brief The amount of time for my thread to sleep
  static const int THREAD_SLEEP = 100000; //1000 = one milla second

    //! \brief The max number of times to fail finding data
  static const int THREAD_FAILED = 100;
    
    //! \brief The number of seconds to wait between updates
  static const int TIMER_COUNT_MAX = 45;

    //! \brief The number of seconds to wait before calling loaded fuse method
  static const int LOADED_FUSE_COUNT = 2000;

    //! \brief Called to run a CLI program
  static int runCmd( QString prog );

  private:
    //! \brief My configuration to log in
  OrmLight*     Config;
    //! \brief The current system status
  int           Status;
    //! \brief The new status that might be different
  int           New_Status;
    //! \brief My QTimer for my status updater
  QTimer*       Status_Timer;
    //! \brief The server socket port
  QTcpServer*   Server;
    //! \brief Holds my tcp client connection
  QTcpSocket*   Client;
    //! \brief My UDP socket I listen on for pull requests
  QUdpSocket*   Udp_Socket;
    //! \brief The mounted directory
  QString       Mounted;
    //! \brief This timer issues svn updates
  QTimer*       Svn_Update;
    //! \brief My port that I end up using
  int My_Port;
    //! \brief True If an svn commit is required
  bool          Timer_Commit_Started;
    //! \brief Timer count, when this reaches the max, and udpate is issued
  int           Timer_Count;
    //! \brief True if my svn handle thread is running
  bool          Thread_Running;
    //! \brief True if the thread isn't running or its not doing anything
  bool          Thread_Idle;
    //! \brief Holds the data sent to me
  QStringList   Data_Read;
    //! \brief Keeps a list of all changes that have been made
  OrmLight      Updated_Items;
    //! \brief The number of tasks that we had on our last pass
  int           Last_Task_Count;
    //! \brief The operating mode of my system
  OperationMode Op_Mode;
    //! \brief Regex to match common things
  QRegExp       RegEx_Dup;
    //! \brief Regex to match common things
  QRegExp       RegEx_Special;

  public:
    //! \brief FuseTracker
  FuseTracker();

    //! \brief The status of the system
  int status();

    //! \brief The operation mode of the system
  OperationMode opMode();

    //! \brief Return the configuration
  OrmLight* config();

    //! \brief Store a new configuration
  OrmLight* setConfig( OrmLight* config );

    //! \brief Open up a server port to be connected to
  bool startServer( int port );

    //! \brief Get the port that was opened
  int getPort();

    //! \brief Called when a new command is issued
  void gotCommand( FuseCppInterface::NotableAction action, QString path, QString from = QString() );

    //! \brief Store the mounted directory
  void setMounted( QString mounted );

    //! \brief Called by my thread
  void run();

    //! \brief Called to load up the sync log file that might exist
  void loadSyncLog( bool change_state = false );

  private slots:

    //! \brief Called once when the fuse interface is booted
  void loadAfterFuseBooted();

  public slots:

    //! \brief Store a new operation mode
  void setOpMode( OperationMode mode );

    //! \brief Store status value
  void setStatus( SystemStatus status );

    //! \brief This will add a another status to the current one
  void addStatus( SystemStatus status );

    //! \brief This will remove a status to the current one
  void removeStatus( SystemStatus status );

    //! \brief Connection made
  void newConnection();

    //! \brief Called when I have new data from the client
  void readyRead();

    //! \brief Called when we should udpate our svn changes
  void updateSVN();

    //! \brief Called when the status update timer should be touched
  void updateStatus();

    //! \brief Called when we should read a pending UDP request
  void readPendingUdpRequest();

    //! \brief When called, a commit command is issued immediately
  void forceCommit();

  signals:
    //! \brief emitted when the operation mode changes
  void opModeChanged( int );

    //! \brief emitted when the state changes
  void statusChanged( int );

    //! \brief emiited when the number of tasks has changed
  void tasksRemaining( int count );
};

#endif
