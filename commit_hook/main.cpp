#include <QCoreApplication>
#include "send_update.h"

int main( int argc, char** argv )
{
  QCoreApplication* app = new QCoreApplication(argc, argv);
  SendUpdate update;
  update.startUpdate();

  return app->exec();
}
