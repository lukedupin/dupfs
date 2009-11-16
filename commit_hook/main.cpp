#include <QCoreApplication>
#include <QString>
#include "send_update.h"
#include "../orm_light/orm_light.h"

int main( int argc, char** argv )
{
    //Ensure we git the correct arguments
  if ( argc != 3 )
  {
    qDebug("Usage: %s REPOS_PATH REV", argv[0] );
    return -1;
  }

  QCoreApplication* app = new QCoreApplication(argc, argv);
  SendUpdate update;
  OrmLight* orm = NULL;

    //Load myu config information
  try {
    orm = OrmLight::loadFromFile(QString("%1/conf/dupfs.conf").arg(argv[1] ));
  } catch ( QString str )
  {
    qDebug("%s", str.toAscii().data() );
    orm = NULL;
  }

    //If orm is null we can't go on
  if ( orm == NULL )
  {
    qDebug("Couldn't load config: %s", 
            QString("%1/conf/dupfs.conf").arg(argv[1]).toAscii().data() );
    return -2;
  }

    //Load up my server
  update.loadConfig( orm );
  update.loadRev( QString::fromUtf8( argv[2] ).toInt() );
  update.startUpdate();

  return app->exec();
}
