#include "fuse_cpp_interface.h"

#include <QRegExp>
#include <QStringList>
#include <QMutex>

  //Sadly I need to use linux sockets, not qt, since we aren't running a qt loop
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <unistd.h>

static FuseCppInterface* Fuse_Interface = NULL;
static QMutex My_Mutex;

//#define DEBUG_LOG
#ifdef DEBUG_LOG
static void log( const char* msg, const char* path, int result )
{
  FILE* handle = fopen( "/tmp/log", "a");

  fprintf( handle, "(%s) %s [%d]\n", path, msg, result );

  fclose( handle );
}
#else
  #define log( x, y, z ) ;
#endif

  //Constructor and destructor
FuseCppInterface::FuseCppInterface( int argc, char** argv ) 
{
    //Store my startup args
  Argc = argc;
  Argv = argv;

    //Set my connection to be invalid
  Socket = -1;
  Socket_Port = -1;

    //Base directories
  Data_Dir = QString::fromUtf8("/tmp/dev");
  Svn_Dir = QString::fromUtf8("/usr/local/dupfs_sync/23432048932erwwer89werwr38wr");

    //Store the first creation of this object as the true fuse interface
  if ( Fuse_Interface == NULL )
    Fuse_Interface = this;  
}

  //Store the port
void FuseCppInterface::setPort( int port )
{
  Socket_Port = port;
}

  //Add a new action
bool FuseCppInterface::pushAction( NotableAction action, QString str, 
                                    QString str2 )
{
  char buffer[BUFFER_SIZE];  

    //connect to my svn tracker class
  if ( Socket < 0 )
  {
    sockaddr_in addr;
    hostent     *host_resolve;

      //Get host by name
    if ( (host_resolve = gethostbyname( "127.0.0.1" )) == NULL )
      return false;

      //set up our connection data to the server
    addr.sin_family = host_resolve->h_addrtype;
    memcpy( (char *)&addr.sin_addr.s_addr,
            host_resolve->h_addr_list[ 0 ],
            host_resolve->h_length );
    addr.sin_port = htons( ((Socket_Port > 0)? Socket_Port: CONNECT_PORT) );

      //Create the temp socket for connection
    if ( (Socket = socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
      return false;

      //Connect to my svn tracker
    if ( connect( Socket, (sockaddr *) &addr, sizeof( addr ) ) < 0 )
      return false;
  }

    //Quit out if I should ignore the action given
  if ( ignoreAction( str ) )
    return false;

    //Store a new action
  memset( buffer, 0, BUFFER_SIZE );
  if ( str2.isEmpty() )
    sprintf( buffer, "@%d,%s#\n", action, 
                            str.replace(QRegExp(","), "\\,").toAscii().data() );
  else
    sprintf( buffer, "@%d,%s,%s#\n", action, 
                            str.replace(QRegExp(","), "\\,").toAscii().data(), 
                            str2.replace(QRegExp(","), "\\,").toAscii().data());
  if ( send( Socket, buffer, strlen( buffer ), 0 ) < 0 )
  {
    close( Socket );
    Socket = -1;
  }
    
  return true;
}

  //This method is in charge of lieing about paths
char* FuseCppInterface::smartPath( const char* path, bool svn_dir )
{
  QString my_path = QString::fromUtf8(path);

    //Is this svn running?
  if      ( my_path == "/" && svn_dir )
    my_path = Svn_Dir;
  else if ( my_path.indexOf( QRegExp("^/[.]dupfs_sync") ) >= 0 )
  {
    if (svn_dir || my_path.indexOf( QRegExp("/[.]svn$") ) >= 0 || 
        my_path.indexOf( QRegExp("/[.]svn/") ) >= 0 )
      my_path = QString("%1%2").arg( Svn_Dir ).arg( my_path );
    else
      my_path = QString("%1%2").arg( Data_Dir ).arg( 
                    my_path.replace( QRegExp("^/[.]dupfs_sync"), 
                                      QString::fromUtf8("") ) );
  }
  else
    my_path = QString("%1%2").arg( Data_Dir ).arg( my_path );

    //Copy the path back to my buffer and return the buffer
  if ( my_path.size() < BUFFER_SIZE - 1 )
    strncpy( Smart_Path, my_path.toAscii().data(), BUFFER_SIZE - 1 );
  else
    Smart_Path[0] = 0;

  return Smart_Path;
}

  //This method changes paths for files which is slightly different
char* FuseCppInterface::smartFilePath( const char* path, bool svn_okay )
{
  QString my_path = QString::fromUtf8(path);

    //Svn base directory
  if ( my_path.indexOf( QRegExp("^/[.]dupfs_sync") ) >= 0 &&
      (svn_okay ||
       my_path.indexOf( QRegExp("/[.]svn$") ) >= 0 || 
       my_path.indexOf( QRegExp("/[.]svn/") ) >= 0) )
    my_path = QString("%1%2").arg( Svn_Dir ).arg( my_path );
  else
    my_path = QString("%1%2").arg( Data_Dir ).arg( 
                    my_path.replace( QRegExp("^/[.]dupfs_sync"), 
                                      QString::fromUtf8("") ) );

    //Copy the path back to my buffer and return the buffer
  if ( my_path.size() < BUFFER_SIZE - 1 )
    strncpy( Smart_Path, my_path.toAscii().data(), BUFFER_SIZE - 1 );
  else
    Smart_Path[0] = 0;

  return Smart_Path;
}

  //Returns true if we require svn
bool FuseCppInterface::requireSvn( const char* path )
{
  QString my_path = QString::fromUtf8(path);

    //Is this svn running?
  if (my_path.indexOf( QRegExp("^/[.]dupfs_sync") ) >= 0 || my_path == "/" )
  {
    if (my_path.indexOf( QRegExp("/[.]svn$") ) >= 0 || 
        my_path.indexOf( QRegExp("/[.]svn/") ) >= 0 )
      return false;
    else
      return true;
  }

  return false;
}

  //Recursively create directories
int FuseCppInterface::recursiveMkDir( QString path, mode_t mode )
{
  int i;
  QStringList list = path.split( QRegExp("/") );
  QString local = Svn_Dir;

    //Create a directory for every possible name
  for ( i = 0; i < list.size(); i++ )
  {
    local += QString("/%1").arg( list[i]);
    mkdir( local.toAscii().data(), mode );
  }

  return 0;
}

  //Returns true if I should ignore an action
bool FuseCppInterface::ignoreAction( QString my_path )
{
    //Is this svn running?
  if (my_path.indexOf( QRegExp("^/[.]dupfs_sync") ) >= 0 &&
      (my_path.indexOf( QRegExp("/[.]svn$") ) >= 0 || 
        my_path.indexOf( QRegExp("/[.]svn/") ) >= 0) )
    return true;

  return false;

}

  ///*** Fuse Interace 
static inline struct xmp_file_handle *get_fhp(struct fuse_file_info *fi)
{
  return (struct xmp_file_handle *) (uintptr_t) fi->fh;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
  { return Fuse_Interface->fuse_getattr( path, stbuf ); }
inline int FuseCppInterface::fuse_getattr(const char *path, struct stat *stbuf)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ( (res = lstat(smartFilePath(path), stbuf)) == -1 )
    res = lstat(smartFilePath(path, true), stbuf);
  log( "getattr", Smart_Path, res );
  //My_Mutex.unlock();
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_fgetattr(const char *path, struct stat *stbuf,
      struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_fgetattr( path, stbuf, fi ); }
inline int FuseCppInterface::fuse_fgetattr(const char *path, struct stat *stbuf,
      struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  (void) path;

  res = fstat( get_fhp(fi)->fh, stbuf);
  log( "fstat", Smart_Path, res );
  //My_Mutex.unlock();
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_access(const char *path, int mask)
  { return Fuse_Interface->fuse_access( path, mask ); }
inline int FuseCppInterface::fuse_access(const char *path, int mask)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ( (res = access( smartFilePath(path), mask)) == -1 )
    res = access( smartFilePath(path, true), mask);
  log( "access", Smart_Path, res );
  //My_Mutex.unlock();
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
  { return Fuse_Interface->fuse_readlink( path, buf, size ); }
inline int FuseCppInterface::fuse_readlink(const char *path, char *buf, size_t size)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ( (res = readlink( smartFilePath(path), buf, size - 1)) == -1 )
    res = readlink( smartFilePath(path, true), buf, size - 1);
  log( "readlink", Smart_Path, res );
  //My_Mutex.unlock();
  if (res == -1)
    return -errno;

  buf[res] = '\0';
  return 0;
}

static int xmp_opendir(const char *path, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_opendir( path, fi ); }
inline int FuseCppInterface::fuse_opendir(const char *path, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  struct xmp_dirp *d;

    //Quit if the alloc failed
  if ((d = new xmp_dirp()) == NULL)
  {
    //My_Mutex.unlock();
    return -ENOMEM;
  }

    //Zero out all my pointers
  d->data_dir_p = NULL;
  d->svn_dir_p = NULL;
  d->data_dir_done = false;
  d->offset = 0;
  d->entry = NULL;

    //Open up the data dir and possible the svn directory
  d->data_dir_p = opendir( smartPath(path) );
  log( "opendir_data", Smart_Path, 0 );
  if (d->data_dir_p == NULL) 
  {
    res = -errno;
    delete d;
    //My_Mutex.unlock();
    return res;
  }

    //Open up the svn dir if need be
  if ( requireSvn( path ) )
  {
    d->svn_dir_p = opendir( smartPath(path, true) );
    log( "opendir_svn", Smart_Path, 0 );
  }

   fi->fh = (unsigned long) d; //Scary!, whats wrong with void*?
  //My_Mutex.unlock();
  return 0;
}

static inline struct xmp_dirp *get_dirp(struct fuse_file_info *fi)
{
  return (struct xmp_dirp *) (uintptr_t) fi->fh;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_readdir( path, buf, filler, offset, fi ); }
inline int FuseCppInterface::fuse_readdir(const char *path,void *buf,
                                   fuse_fill_dir_t filler,
                                   off_t offset, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  struct xmp_dirp *d = get_dirp(fi);
  struct stat st;
  off_t nextoff;
  bool valid;
  bool skip_entry;

    //Figure out if we should hide svn directorys from the data section
  QString my_path;

    //Log some data
  log( "readdir", path, ((d->svn_dir_p == NULL)? 0: 1) );

    //Based on my open dir read the contents
  if (offset != d->offset) 
  {
    if ( !d->data_dir_done )
      seekdir(d->data_dir_p, offset);
    else if ( d->svn_dir_p != NULL )
      seekdir(d->svn_dir_p, offset);
    d->entry = NULL;
    d->offset = offset;
  }

    //Figure out which loop we want
  valid = true;
  while ( valid && d->data_dir_p != NULL && !d->data_dir_done ) 
  {
      //Read a new entry from the dir
    if (!d->entry) 
    {
      d->entry = readdir(d->data_dir_p);
      if ( !d->entry )  //Set that we can goto the next dir
      {
        d->data_dir_done = true;
        if ( d->svn_dir_p != NULL )
        {
          d->entry = NULL;
          d->offset = 0;
        }
        valid = false;
      }
    }

      //If we want to skip svn directories check here
    skip_entry = false;
    if ( valid && d->svn_dir_p != NULL )
    {
      my_path = QString( "%1/%2").arg(path).arg(d->entry->d_name);
      if ( my_path.indexOf( QRegExp("/[.]svn$") ) >= 0 || 
           my_path.indexOf( QRegExp("/[.]svn/") ) >= 0 )
        skip_entry = true;
    }

      //Read the stats for this item
    if ( valid )
    {
      memset(&st, 0, sizeof(st));
      st.st_ino = d->entry->d_ino;
      st.st_mode = d->entry->d_type << 12;
      nextoff = telldir(d->data_dir_p);
      if ( !skip_entry )
      {
        if (filler(buf, d->entry->d_name, &st, nextoff))
        {
          //My_Mutex.unlock();
          return 0;
        }
        else
        {
          my_path = QString("%1/%2").arg(path).arg(d->entry->d_name);
          d->path_list[my_path] = true;
        }
      }
          
      d->entry = NULL;
      d->offset = nextoff;
    }
  }

    //If required, add in the .svn directories to my data  
  while ( d->svn_dir_p != NULL && d->data_dir_done ) 
  {
    if (!d->entry) {
      d->entry = readdir(d->svn_dir_p);
      if ( !d->entry )
      {
        //My_Mutex.unlock();
        return 0;
      }
    }

      //read the status of this item
    memset(&st, 0, sizeof(st));
    st.st_ino = d->entry->d_ino;
    st.st_mode = d->entry->d_type << 12;
    nextoff = telldir(d->svn_dir_p);

      //add this entry if its new
    my_path = QString("%1/%2").arg(path).arg(d->entry->d_name);
    if ( !d->path_list.contains( my_path ) )// && 
//          (strcmp( d->entry->d_name, ".dupfs_sync" ) == 0 ||
//          strcmp( d->entry->d_name, ".svn" ) == 0) )
      if (filler(buf, d->entry->d_name, &st, nextoff))
      {
        //My_Mutex.unlock();
        return 0;
      }

    d->entry = NULL;
    d->offset = nextoff;
  }

  //My_Mutex.unlock();
  return 0;
}

static int xmp_releasedir(const char *path, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_releasedir( path, fi ); }
inline int FuseCppInterface::fuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  struct xmp_dirp *d = get_dirp(fi);

    //Log some data
  (void)path;
  log( "close dir", path, 0 );

    //Close down the directories
  closedir(d->data_dir_p);
  if ( d->svn_dir_p != NULL )
    closedir(d->svn_dir_p);
  delete d;

  //My_Mutex.unlock();
  return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
  { return Fuse_Interface->fuse_mknod( path, mode, rdev ); }
inline int FuseCppInterface::fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if (S_ISFIFO(mode))
    res = mkfifo( smartFilePath(path), mode );
  else
    res = mknod( smartFilePath(path), mode, rdev );
  log( "mknod", Smart_Path, res );

    //Make sure it works
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }
    
  pushAction( MKNOD, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
  { return Fuse_Interface->fuse_mkdir( path, mode ); }
inline int FuseCppInterface::fuse_mkdir(const char *path, mode_t mode)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res = 0;
  QString my_path = QString::fromUtf8( path );

    //Is this svn running?
  if ( my_path.indexOf( QRegExp("^/[.]dupfs_sync") ) >= 0 &&
      (my_path.indexOf( QRegExp("/[.]svn$") ) >= 0 || 
       my_path.indexOf( QRegExp("/[.]svn/") ) >= 0) )
  {
    res = recursiveMkDir( my_path, mode );
  }
  else  //Make a normal directory
    res = mkdir( smartFilePath(path), mode);
  log( "mkdir", Smart_Path, res );

    //Add the directory the user wanted
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }
     
  pushAction( MKDIR, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_unlink(const char *path)
  { return Fuse_Interface->fuse_unlink( path ); }
inline int FuseCppInterface::fuse_unlink(const char *path)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ((res = unlink( smartFilePath(path) )) == -1 )
    res = unlink( smartFilePath(path, true) );
  log( "unlink", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }
    
  pushAction( UNLINK, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_rmdir(const char *path)
  { return Fuse_Interface->fuse_rmdir( path ); }
inline int FuseCppInterface::fuse_rmdir(const char *path)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ((res = rmdir(smartFilePath(path))) == -1 )
    res = rmdir(smartFilePath(path));
  log( "rmdir", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }
    
  pushAction( RMDIR, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_symlink(const char *from, const char *to)
  { return Fuse_Interface->fuse_symlink( from, to ); }
inline int FuseCppInterface::fuse_symlink(const char *from, const char *to)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  char from_buf[BUFFER_SIZE];
  char to_buf[BUFFER_SIZE];

  strncpy( from_buf, smartFilePath(from), BUFFER_SIZE - 1 );
  strncpy( to_buf, smartFilePath(to), BUFFER_SIZE - 1 );

  res = symlink( from_buf,  to_buf );
  log( "sym", to_buf, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }
    
  pushAction( SYMLINK, QString::fromUtf8(to), QString::fromUtf8(from) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_rename(const char *from, const char *to)
  { return Fuse_Interface->fuse_rename( from, to ); }
inline int FuseCppInterface::fuse_rename(const char *from, const char *to)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  char from_buf[BUFFER_SIZE];
  char to_buf[BUFFER_SIZE];

  strncpy( from_buf, smartFilePath(from), BUFFER_SIZE - 1 );
  strncpy( to_buf, smartFilePath(to), BUFFER_SIZE - 1 );

  res = rename( from_buf,  to_buf );
  log( "rename", to_buf, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( RENAME, QString::fromUtf8(to), QString::fromUtf8(from) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_link(const char *from, const char *to)
  { return Fuse_Interface->fuse_link( from, to ); }
inline int FuseCppInterface::fuse_link(const char *from, const char *to)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  char from_buf[BUFFER_SIZE];
  char to_buf[BUFFER_SIZE];

  strncpy( from_buf, smartFilePath(from), BUFFER_SIZE - 1 );
  strncpy( to_buf, smartFilePath(to), BUFFER_SIZE - 1 );

  res = link( from_buf,  to_buf );
  log( "link", to_buf, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( HARDLINK, QString::fromUtf8(to), QString::fromUtf8(from) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
  { return Fuse_Interface->fuse_chmod( path, mode ); }
inline int FuseCppInterface::fuse_chmod(const char *path, mode_t mode)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ((res = chmod( smartFilePath(path), mode)) == -1 )
    res = chmod( smartFilePath(path, true), mode);
  log( "chmod", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( CHMOD, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
  { return Fuse_Interface->fuse_chown( path, uid, gid ); }
inline int FuseCppInterface::fuse_chown(const char *path, uid_t uid, gid_t gid)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ((res = lchown( smartFilePath(path), uid, gid)) == -1 )
    res = lchown( smartFilePath(path, true), uid, gid);
  log( "lchown", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( CHOWN, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_truncate(const char *path, off_t size)
  { return Fuse_Interface->fuse_truncate( path, size ); }
inline int FuseCppInterface::fuse_truncate(const char *path, off_t size)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ((res = truncate( smartFilePath(path), size)) == -1 )
    res = truncate( smartFilePath(path, true), size);
  log( "truncate", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( TRUNCATE, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_ftruncate(const char *path, off_t size,
                         struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_ftruncate( path, size, fi ); }
inline int FuseCppInterface::fuse_ftruncate(const char *path, off_t size,
       struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  (void) path;

  res = ftruncate( get_fhp(fi)->fh, size);
  log( "ftruncate", Smart_Path, (int) get_fhp(fi)->fh );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( FTRUNCATE, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
  { return Fuse_Interface->fuse_utimens( path, ts ); }
inline int FuseCppInterface::fuse_utimens(const char *path, const struct timespec ts[2])
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  struct timeval tv[2];

  tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec / 1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec / 1000;

  if ((res = utimes( smartFilePath(path), tv)) == -1 )
    res = utimes( smartFilePath(path, true), tv);
  log( "utimes", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( UTIMES, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_create( path, mode, fi ); }
inline int FuseCppInterface::fuse_create(const char *path, mode_t mode, 
                              struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int fd;
  xmp_file_handle* d;

  fd = open( smartFilePath(path), fi->flags, mode);
  log( "create", Smart_Path, fd );
  if (fd == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

    //Send Tell the system we just created a file
  pushAction( CREATE, QString::fromUtf8(path) );

    //Alloc my fh struct
  if ((d = new xmp_file_handle()) == NULL)
    return -1;
  
    //Store my info about this new file
  d->fh = fd;
  d->write_action = true;
  
  fi->fh = (unsigned long) d; //Scary, whats wrong with using void*? 

  //My_Mutex.unlock();
  return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_open( path, fi ); }
inline int FuseCppInterface::fuse_open(const char *path, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int fd;
  xmp_file_handle* d = NULL;

  fd = open( smartFilePath(path), fi->flags);
  log( "open", Smart_Path, fd );
  if (fd == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

    //Alloc my info about this file
  if ((d = new xmp_file_handle()) == NULL)
    return -1;
  
    //Store my info about this new file
  d->fh = fd;
  d->write_action = false;

    //Make sure that we only worry about write opens
  if ((fi->flags & O_WRONLY) || (fi->flags & O_RDWR) || (fi->flags & O_APPEND))
  {
    d->write_action = true;

//    if ( (fi->flags & O_CREAT) )
//      pushAction( CREATE, QString::fromUtf8(path) );
//    else
//      pushAction( OPEN, QString::fromUtf8(path) );
  }

  fi->fh = (unsigned long) d; //Scary, whats wrong with using void*? 
  //My_Mutex.unlock();
  return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, 
                    off_t offset, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_read( path, buf, size, offset, fi ); }
inline int FuseCppInterface::fuse_read(const char *path, char *buf, size_t size, 
                            off_t offset, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  (void) path;
  res = pread( get_fhp(fi)->fh, buf, size, offset);
  log( "read", path, res );
  if (res == -1)
    res = -errno;

  //My_Mutex.unlock();
  return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_write( path, buf, size, offset, fi ); }
inline int FuseCppInterface::fuse_write(const char *path, const char *buf, size_t size,
         off_t offset, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  (void)path;

    //Write out my data
  res = pwrite( get_fhp(fi)->fh, buf, size, offset);
  log( "write", path, res );
  if (res == -1)
    res = -errno;

//  pushAction( WRITE, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
  { return Fuse_Interface->fuse_statfs( path, stbuf ); }
inline int FuseCppInterface::fuse_statfs(const char *path, struct statvfs *stbuf)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;

  if ((res = statvfs( smartFilePath(path), stbuf)) == -1 )
    res = statvfs( smartFilePath(path, true), stbuf);
  log( "statfs", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  //My_Mutex.unlock();
  return 0;
}

static int xmp_flush(const char *path, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_flush( path, fi ); }
inline int FuseCppInterface::fuse_flush(const char *path, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  xmp_file_handle * fp = get_fhp( fi );

  /* This is called from every close on an open file, so call the
     close on the underlying filesystem.  But since flush may be
     called multiple times for an open file, this must not really
     close the file.  This is important if used on a network
     filesystem like NFS which flush the data/metadata on close() */
  res = close(dup( fp->fh));
  log( "flush", path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

    //Send a notice if we are opened for write actions
  if ( fp->write_action )
    pushAction( FLUSH, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
  { return Fuse_Interface->fuse_release( path, fi ); }
inline int FuseCppInterface::fuse_release(const char *path, struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  xmp_file_handle * fp = get_fhp( fi );

  close( fp->fh);
  log( "close", path, fp->fh );

    //If we were open for write actions then send an update command now
  if ( fp->write_action )
    pushAction( CLOSE, QString::fromUtf8(path) );

    //Kill the frame pointer
  delete fp;

  //My_Mutex.unlock();
  return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
                      struct fuse_file_info *fi)
{ return Fuse_Interface->fuse_fsync( path, isdatasync, fi ); }
inline int FuseCppInterface::fuse_fsync(const char *path, int isdatasync,
         struct fuse_file_info *fi)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  (void)path;

    //Pretty crazy code guys!
#ifndef HAVE_FDATASYNC
  (void) isdatasync;
#else
  if (isdatasync)
    res = fdatasync( get_fhp(fi)->fh);
  else
#endif
    res = fsync( get_fhp(fi)->fh);
  log( "fsync", path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  //pushAction( FSYNC, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, 
                        const char *value, size_t size, int flags)
  { return Fuse_Interface->fuse_setxattr( path, name, value, size, flags ); }
inline int FuseCppInterface::fuse_setxattr(const char *path, const char *name, 
                                const char *value, size_t size, int flags)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  if ((res = lsetxattr(smartFilePath(path), name, value, size, flags)) == -1 )
    res = lsetxattr(smartFilePath(path, true), name, value, size, flags);
  log( "lsetxattr", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( SETXATTR, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
                        size_t size)
  { return Fuse_Interface->fuse_getxattr( path, name, value, size ); }
inline int FuseCppInterface::fuse_getxattr(const char *path, const char *name, char *value,
      size_t size)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  if ((res = lgetxattr(smartFilePath(path), name, value, size)) == -1 )
    res = lgetxattr(smartFilePath(path, true), name, value, size);
  log( "lgetxattr", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  //My_Mutex.unlock();
  return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
  { return Fuse_Interface->fuse_listxattr( path, list, size ); }
inline int FuseCppInterface::fuse_listxattr(const char *path, char *list, size_t size)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  if ((res = llistxattr(smartFilePath(path), list, size)) == -1 )
    res = llistxattr(smartFilePath(path, true), list, size);
  log( "llistxattr", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  //My_Mutex.unlock();
  return res;
}

static int xmp_removexattr(const char *path, const char *name)
  { return Fuse_Interface->fuse_removexattr( path, name ); }
inline int FuseCppInterface::fuse_removexattr(const char *path, const char *name)
{
    //Lock My Mutex
  //My_Mutex.lock();

  int res;
  if ((res = lremovexattr(smartFilePath(path), name)) == -1 )
    res = lremovexattr(smartFilePath(path, true), name);
  log( "lremoveattr", Smart_Path, res );
  if (res == -1)
  {
    //My_Mutex.unlock();
    return -errno;
  }

  pushAction( REMOVEXATTR, QString::fromUtf8(path) );

  //My_Mutex.unlock();
  return 0;
}
#endif /* HAVE_SETXATTR */

static int xmp_lock(const char *path, struct fuse_file_info *fi, int cmd,
                    struct flock *lock)
  { return Fuse_Interface->fuse_lock( path, fi, cmd, lock ); }
inline int FuseCppInterface::fuse_lock(const char *path, struct fuse_file_info *fi, int cmd,
        struct flock *lock)
{
    //Lock My Mutex
  //My_Mutex.lock();

  (void) path;
  int res;

  res = ulockmgr_op( get_fhp(fi)->fh, cmd, lock, &fi->lock_owner,sizeof(fi->lock_owner));
  log( "lock", path,  get_fhp(fi)->fh );

  //My_Mutex.unlock();
  return res;
}

static struct fuse_operations xmp_oper;

//*** My thread entry point for the fuse start block

  //Start the main loop
int FuseCppInterface::startMain()
{
    //Assign my c functions to my user space struct
  xmp_oper.getattr    = xmp_getattr;
  xmp_oper.fgetattr   = xmp_fgetattr;
  xmp_oper.access     = xmp_access;
  xmp_oper.readlink   = xmp_readlink;
  xmp_oper.opendir    = xmp_opendir;
  xmp_oper.readdir    = xmp_readdir;
  xmp_oper.releasedir = xmp_releasedir;
  xmp_oper.mknod      = xmp_mknod;
  xmp_oper.mkdir      = xmp_mkdir;
  xmp_oper.symlink    = xmp_symlink;
  xmp_oper.unlink     = xmp_unlink;
  xmp_oper.rmdir      = xmp_rmdir;
  xmp_oper.rename     = xmp_rename;
  xmp_oper.link       = xmp_link;
  xmp_oper.chmod      = xmp_chmod;
  xmp_oper.chown      = xmp_chown;
  xmp_oper.truncate   = xmp_truncate;
  xmp_oper.ftruncate  = xmp_ftruncate;
  xmp_oper.utimens    = xmp_utimens;
  xmp_oper.create     = xmp_create;
  xmp_oper.open       = xmp_open;
  xmp_oper.read       = xmp_read;
  xmp_oper.write      = xmp_write;
  xmp_oper.statfs     = xmp_statfs;
  xmp_oper.flush      = xmp_flush;
  xmp_oper.release    = xmp_release;
  xmp_oper.fsync      = xmp_fsync;
#ifdef HAVE_SETXATTR
  xmp_oper.setxattr   = xmp_setxattr;
  xmp_oper.getxattr   = xmp_getxattr;
  xmp_oper.listxattr  = xmp_listxattr;
  xmp_oper.removexattr= xmp_removexattr;
#endif
  xmp_oper.lock       = xmp_lock;

  //.flag_nullpath_ok = 1,

    //My main fuse block
  umask(0);
  fuse_main(Fuse_Interface->Argc, Fuse_Interface->Argv, &xmp_oper, NULL);
  return 0;
}
