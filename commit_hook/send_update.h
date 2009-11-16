#ifndef SEND_UPDATE
#define SEND_UPDATE

#include <QObject>
#include "../orm_light/orm_light.h"

class SendUpdate : public QObject
{
  Q_OBJECT

  public:
    //! \brief The port we are going to broadcast on
  static const int PORT = 16917;

  private:
    //! \brief The rev of this commit
  int Rev;
    //! \brief Holds the orm light config
  OrmLight* Config;

  public:
    //! \brief Init my object
  SendUpdate();

    //! \brief load config
  void loadConfig( OrmLight* config );

    //! \brief Sets the revision of this commit
  void loadRev( int rev );

    //! Called to start my timer which will send the udp packet
  void startUpdate();

  public slots:
  void bootServer();
};

#endif
