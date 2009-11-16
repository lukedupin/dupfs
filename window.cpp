#include <QtGui>
#include <QFile>

#include "window.h"

Window::Window( FuseTracker* tracker )
{
  int i;

    //Store my params
  Fuse_Tracker = tracker;
  Last_Status = FuseTracker::UNKNOWN;

    //Init my instance variables
  Visible = false;
  Spin_Idx = -1;
  Pending_Tasks = 0;

    //Create my tray menu
  Tray_Icon_Menu = new QMenu(this);

  Tray_Icon_Menu->addAction( 
    Upload_Action = new QAction(tr("Upload Changes"), this));

  Tray_Icon_Menu->addAction( 
    Download_Action = new QAction(tr("Download Changes"), this));

    //Create the mode selectino menu
  Tray_Icon_Menu->addSeparator();
  Tray_Icon_Menu->addMenu( 
    Mode_Menu = new QMenu(tr("Select Mode"), this));
  Mode_Menu->addAction( 
    Sync_Mode_Action = new QAction(tr("Sync Mode"), this ));
  Sync_Mode_Action->setCheckable( true );
  Mode_Menu->addAction( 
    Offline_Mode_Action = new QAction(tr("Offline Mode"), this ));
  Offline_Mode_Action->setCheckable( true );

    //Give the user a quit option
  Tray_Icon_Menu->addSeparator();
  Tray_Icon_Menu->addAction(
    Quit_Action = new QAction(tr("&Quit"), this));

    //Create the system tray
  Tray_Icon = new QSystemTrayIcon(this);
  Tray_Icon->setContextMenu(Tray_Icon_Menu);

    //Build out my control menu
  Main_Layout = new QVBoxLayout;
  setLayout(Main_Layout);

    //Start my timer
  Timer = new QTimer();

    //Setup my signals/slots
  connect(Fuse_Tracker, SIGNAL(tasksRemaining(int)), 
          this, SLOT(tasksRemaining(int)));
  connect(Fuse_Tracker, SIGNAL(opModeChanged(int)), 
          this, SLOT(operationChanged(int)));
  connect(Tray_Icon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
  connect(Tray_Icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
  connect( Upload_Action, SIGNAL(triggered()), 
            Fuse_Tracker, SLOT(forceCommit()));
  connect( Download_Action, SIGNAL(triggered()), 
            Fuse_Tracker, SLOT(forceUpdate()));
  connect(Sync_Mode_Action, SIGNAL(triggered()), this, SLOT(syncModeClicked()));
  connect(Offline_Mode_Action, SIGNAL(triggered()), 
          this, SLOT(offlineModeClicked()));
  connect(Quit_Action, SIGNAL(triggered()), this, SLOT(quitRequest()));
  connect(Timer, SIGNAL(timeout()), this, SLOT(timeout()));

    //Build out my icon
  for ( i = 0; i < 8; i++ )
  {
    Spin_List["black_clock"].push_back( QIcon(QString(":/images/black_spin_clock_%1.png").arg(i)));
    Spin_List["red_clock"].push_back( QIcon(QString(":/images/red_spin_clock_%1.png").arg(i)));
    Spin_List["green_clock"].push_back( QIcon(QString(":/images/green_spin_clock_%1.png").arg(i)));
    Spin_List["blue_clock"].push_back( QIcon(QString(":/images/blue_spin_clock_%1.png").arg(i)));
  }

    //Build my called waves
  Icon_Hash.insert( "red_clock",    QIcon(":/images/red_clock.png") );
  Icon_Hash.insert( "blue_clock",   QIcon(":/images/blue_clock.png") );
  Icon_Hash.insert( "black_clock",  QIcon(":/images/black_clock.png") );
  Icon_Hash.insert( "green_clock",  QIcon(":/images/green_clock.png") );

  Tray_Icon->setIcon(Icon_Hash["red_clock"]);
  setWindowIcon(Icon_Hash["red_clock"]);

  Tray_Icon->setToolTip("DupFs");

  Tab_Widget = new QTabWidget();
  Tab_Widget->setObjectName("Tab_Widget");
  Main_Layout->addWidget(Tab_Widget);

    //Load up my icon in the system menu
  Tray_Icon->show();

    //Load up my Dialog
  setWindowTitle(tr("DupFs"));
  resize(400, 300);

    //Start my timer
  Timer->start( TIMEOUT );
}

  //Close this guy down
void Window::closeEvent(QCloseEvent *event)
{
    //If our control menu is open, don't really close
  if (Tray_Icon->isVisible()) {
      hide();
      event->ignore();
  }
}

  //Called when the user selects the system mode
void Window::syncModeClicked()
{
  Fuse_Tracker->setOpMode( FuseTracker::OP_SYNC_MODE );
}

  //Called when the user selects the offline mode
void Window::offlineModeClicked()
{
  Fuse_Tracker->setOpMode( FuseTracker::OP_OFFLINE_MODE );
}

  //Called when the operation has changed
void Window::operationChanged( int mode )
{
    //Uncheck all my options
  Sync_Mode_Action->setChecked( false );
  Offline_Mode_Action->setChecked( false );

    //Switch through all my modes
  switch ( static_cast<FuseTracker::OperationMode>(mode) )
  {
    case FuseTracker::OP_SYNC_MODE:
      Sync_Mode_Action->setChecked( true );
      break;

    case FuseTracker::OP_OFFLINE_MODE:
      Offline_Mode_Action->setChecked( true );
      break;

    default:
      break;
  }
}

  //Called to handle click actions
void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason) 
  {
      //Not really possible to get to
    case QSystemTrayIcon::DoubleClick:
      QDialog::setVisible(  Visible = !Visible );
      break;
      //Opens up the men
    case QSystemTrayIcon::Trigger:
      Tray_Icon_Menu->exec(QCursor::pos());
      break;
    case QSystemTrayIcon::MiddleClick:
      showMessage();
      break;
    default:
      break;
  }
}

  //Send the user a message!
void Window::showMessage()
{
  QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(
    QSystemTrayIcon::Information );

    //Get the current status of the fuse tracker
  int status = Fuse_Tracker->status();

    //Figure otu what message we should display
  if      ( (status & FuseTracker::SYNC_PUSH) )
    Tray_Icon->showMessage("DupFs", 
                     QString::fromUtf8("DupFs Uploading Changes"), icon, 6000);
  else if ( (status & FuseTracker::SYNC_PULL) )
    Tray_Icon->showMessage("DupFs", 
                     QString::fromUtf8("DupFs Downloading Changes"), icon,6000);
  else if ( (status & FuseTracker::ADDING_ITEMS) )
    Tray_Icon->showMessage("DupFs", 
                           QString::fromUtf8("DupFs Adding Items"), icon, 6000);
  else
    Tray_Icon->showMessage("DupFs",
                           QString::fromUtf8("DupFs Watching"), icon, 6000);

    //Set the tooltip
  Tray_Icon->setToolTip("DupFs");
}

void Window::messageClicked()
{
  QMessageBox::information(0, tr("Systray"),
                           tr("Sorry, I already gave what help I could.\n"
                              "Please goto http://lukedupin/dupfs/ for help"));
}

void Window::timeout()
{
    //Get the current status of the fuse tracker
  int status = Fuse_Tracker->status();

      //True when the system is currently tracking time
  if      ( (status & FuseTracker::SYNC_PUSH) )
  {
      //Spin the image around
    Spin_Idx++;
    if ( Spin_Idx < Spin_List["red_clock"].size() )
      Tray_Icon->setIcon(Spin_List["red_clock"][Spin_Idx]);
    else if ( Spin_Idx >= SPIN_LIST_MAX )
    {
      Spin_Idx = -1;

        //Log what time it is
      Tray_Icon->setToolTip(QString::fromUtf8("DupFs Uploading Changes"));
    }
  }

      //True when the system is currently tracking time
  else if ( (status & FuseTracker::SYNC_PULL) )
  {
      //Spin the image around
    Spin_Idx++;
    if ( Spin_Idx < Spin_List["red_clock"].size() )
      Tray_Icon->setIcon(Spin_List["red_clock"][Spin_Idx]);
    else if ( Spin_Idx >= SPIN_LIST_MAX )
    {
      Spin_Idx = -1;

        //Log what time it is
      Tray_Icon->setToolTip(QString::fromUtf8("DupFs Downloading Changes"));
    }
  }
    
      //True when the system should sync but is wiating to
  else if ( (status & FuseTracker::ADDING_ITEMS) )
  {
      //Spin the image around
    Spin_Idx++;
    if ( Spin_Idx < Spin_List["blue_clock"].size() )
      Tray_Icon->setIcon(Spin_List["blue_clock"][Spin_Idx]);
    else if ( Spin_Idx >= SPIN_LIST_MAX )
    {
      Spin_Idx = -1;

        //Log what time it is
      Tray_Icon->setToolTip(QString("DupFs Syncing Tasks Remaining: %1").arg(Pending_Tasks));
    }
  }
      //True when the system is currently tracking time
  else if ( (status & FuseTracker::SYNC_PULL_REQUIRED) ||
            (status & FuseTracker::SYNC_PUSH_REQUIRED) )
  {
      //Spin the image around
    Spin_Idx++;
    if ( Spin_Idx < Spin_List["green_clock"].size() )
      Tray_Icon->setIcon(Spin_List["green_clock"][Spin_Idx]);
    else if ( Spin_Idx >= SPIN_LIST_MAX )
    {
      Spin_Idx = -1;

        //Log what time it is
      Tray_Icon->setToolTip(QString::fromUtf8("DupFs Sync Required"));
    }
  }
  else// if ( (status & FuseTracker::WATCHING) )
  {
    if ( Last_Status == Fuse_Tracker->status() ) return;

      //Reset the thing to a sleeping state
    Spin_Idx = -1;
    Tray_Icon->setIcon(Icon_Hash["black_clock"]);
    Tray_Icon->setToolTip(QString::fromUtf8("DupFs Watching"));
  }

    //Store my last status
  Last_Status = Fuse_Tracker->status();
}

  //Quit request is being made
void Window::quitRequest()
{
  if ( QMessageBox::warning(this, "Are you sure?", 
                            "Are you sure you want to quit?",
                            QMessageBox::Ok | QMessageBox::Cancel ) == 
                              QMessageBox::Ok )
    qApp->quit();
}

  //Teh number of task I have remaining
void Window::tasksRemaining( int tasks )
{
  Pending_Tasks = tasks;
}
