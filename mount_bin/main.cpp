#include "../orm_light/orm_light.h"
#include <stdlib.h>

#include <QFile>

  //My mizain
int main( int argc, char** argv )
{
    //Ensure the user gave me some args
  if ( argc != 2 )
  {
    qDebug( "Usage: %s config_file", argv[0] );
    return 1;
  }

    //My c file to mount the directory
  QString data = QString::fromUtf8("#include <sys/types.h>\n#include <unistd.h>\n#include <stdio.h>\n#include <stdlib.h>\nint main() { char buf[256]; sprintf( buf, \"mount -o uid=%d,gid=%d --bind MNT DATA\", getuid(), getgid() ); setuid( geteuid() ); return system( buf ); }");

    //Load up my config file
  QString filename(argv[1]);
  OrmLight* orm = OrmLight::loadFromFile( filename );
  int res;

    //Quit if we got an invalid config
  if ( orm == NULL )
  {
    qDebug( "Got invalid config file" );
    return 1;
  }

    //Tell the user wahts up
  qDebug( "Creating mount.c based off %s", argv[1] );

    //Spit out my new file
  QFile file("mount.c");
  if ( !file.open( QIODevice::WriteOnly ) )
    return 2;

    //Write out my string
  file.write( data.replace("MNT", (*orm)["mount_dir"] ).replace( "DATA", (*orm)["data_dir"] ).toAscii() );
  file.close();

    //Tell the user what to do now
  qDebug("sudo gcc mount.c -o mount_bind && sudo chmod 4755 mount_bind");
  res = system("sudo gcc mount.c -o mount_bind && sudo chmod 4755 mount_bind");

    //Kill the file I generated
  qDebug( "Removing mount.c" );
  QFile::remove( QString::fromUtf8("mount.c") );

  return 0;
}
