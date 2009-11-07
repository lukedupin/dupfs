#ifndef SEND_UPDATE
#define SEND_UPDATE

#include <QObject>

class SendUpdate : public QObject
{
  Q_OBJECT

  public:
    //! \brief The port we are going to broadcast on
  static const int PORT = 16917;

  public:
  void startUpdate();

  public slots:
  void bootServer();
};

#endif
