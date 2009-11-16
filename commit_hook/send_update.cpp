#include <QCoreApplication>
#include <QTimer>
#include <QUdpSocket>
#include <QHostAddress>
#include <QStringList>

#include "send_update.h"

  //Init my object
SendUpdate::SendUpdate()
{
  Rev = 0;
  Config = NULL;
}

  //Load up my config info
void SendUpdate::loadConfig( OrmLight* config )
{
  Config = config;
}

  //Loads up a given revision
void SendUpdate::loadRev( int rev )
{
  Rev = rev;
}

  //Starts my server
void SendUpdate::startUpdate()
{
  QTimer::singleShot(200, this, SLOT(bootServer()));
}

  //Runs inside my qt thread
void SendUpdate::bootServer()
{
  int i;
  int port = PORT;
  QString data;
  QUdpSocket* udp = new QUdpSocket(this);

    //Fill my data we will send
  data = QString("SVN UPDATE REQUESTED %1\n").arg( Rev );

    //Go through all my orm entries
  for ( i = 0; i < (*Config)["clients"].size(); i++ )
  {
    QStringList list = (*Config)["clients"][i].split(":");

      //Set the port we are going touse
    if ( list.size() > 1 )
      port = list[1].toInt();
    if ( list.size() <= 1 || port <= 0 )
      port = PORT;

    qDebug("%s %d", list[0].toAscii().data(), port );
      //Send the data
    udp->writeDatagram( data.toAscii(), QHostAddress(list[0]), port ); 
    usleep( 100000 ); //Sleep a 10th of a second
  }

    //Wait for a 10th of a second and then quit
  QCoreApplication::exit(0);
}
