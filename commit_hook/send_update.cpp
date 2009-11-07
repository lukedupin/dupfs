#include <QCoreApplication>
#include <QTimer>
#include <QUdpSocket>
#include <QHostAddress>
#include "send_update.h"

void SendUpdate::startUpdate()
{
  QTimer::singleShot(200, this, SLOT(bootServer()));
}

void SendUpdate::bootServer()
{
  QString data;
  QUdpSocket* udp = new QUdpSocket(this);

    //Fill my data we will send
  data = QString::fromUtf8("SVN UPDATE REQUESTED 917");
  
  udp->writeDatagram( data.toAscii(), 
                      QHostAddress(QHostAddress::Broadcast), PORT); 

    //Wait for a 10th of a second and then quit
  usleep( 100000 );
  QCoreApplication::exit(0);
}
