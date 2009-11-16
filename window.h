#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QTimer>
#include <QList>
#include <QHash>
#include <QTabWidget>
#include <QByteArray>

#include "fuse_tracker.h"

class Window : public QDialog
{
  Q_OBJECT

  public:
    //! \brief The max number of ticks before the spin restarts
  static const int SPIN_LIST_MAX = 30;
    //! \brief The number of ticks to wait until calling the timeout
  static const int TIMEOUT = 100;

  private:
    //My system status
  FuseTracker    *Fuse_Tracker;

    //My menu system
  QSystemTrayIcon   *Tray_Icon;
  QMenu             *Tray_Icon_Menu;

  QAction           *Upload_Action;
  QAction           *Download_Action;

  QMenu             *Mode_Menu;
  QAction           *Sync_Mode_Action;
  QAction           *Offline_Mode_Action;

  QAction           *Quit_Action;

  QVBoxLayout       *Main_Layout;
  QTabWidget        *Tab_Widget;

  QTimer            *Timer;
  bool              Visible;

  int               Pending_Tasks;

  QHash<QString, QIcon> Icon_Hash;

    //Spin icon
  int                           Spin_Idx;
  QHash<QString, QList<QIcon> > Spin_List;

    //Store the last state
  int Last_Status;

  public:
  Window( FuseTracker* tracker );

  protected:
    //! \brief called to tell the program to close
  void closeEvent(QCloseEvent *event);

  private slots:
    //! \brief Called when a user selects the sync operation mode
  void syncModeClicked();

    //! \brief Called when a user selects the offline operation mode
  void offlineModeClicked();

    //! \brief Called when an operation changes
  void operationChanged( int );

    //! \brief Called when the user is clicking on the icon
  void iconActivated(QSystemTrayIcon::ActivationReason reason);

    //! \brief Show a little hover message
  void showMessage();

    //! \brief Send the user some information
  void messageClicked();

    //! \brief Called once a second to do whatever
  void timeout();

    //! \brief Quit requrest called
  void quitRequest();

    //! \brief Called to let me know how many pending tasks there are
  void tasksRemaining( int tasks );
};

#endif

