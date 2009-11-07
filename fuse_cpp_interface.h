#ifndef FUSE_CPP_INTERFACE
#define FUSE_CPP_INTERFACE

#include <pthread.h>
#include <QList>
#include <QPair>
#include <QString>
#include <QHash>

#include "orm_light/orm_light.h"

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#define _GNU_SOURCE

#include <fuse.h>
extern "C" { 
  #include <ulockmgr.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

  //REquired for the opendir code
struct xmp_dirp {
  QHash<QString, bool> path_list;

  DIR *data_dir_p;
  DIR *svn_dir_p;

  struct dirent *entry;
  bool data_dir_done;
  off_t offset;
};

  //Used to track file actions to limit our message back and forth
struct xmp_file_handle {
  uint64_t fh;
  bool write_action;
};

	//Create a serial com interface object
class FuseCppInterface {
  public:
    //! \brief My buffer size
  static const int BUFFER_SIZE = 1024;
    //! \brief The port that we connect over
  static const int CONNECT_PORT = 15917;

    //! \brief A list of all the notable actions I deal with
  enum NotableAction {
    MKNOD         = 0,
    MKDIR         = 1,
    UNLINK        = 2,
    RMDIR         = 3,
    SYMLINK       = 4,
    RENAME        = 5,
    HARDLINK      = 6,
    CHMOD         = 7,
    CHOWN         = 8,
    TRUNCATE      = 9,
    FTRUNCATE     = 10,
    UTIMES        = 11,
    CREATE        = 12,
    OPEN          = 13,
    WRITE         = 14,
    FLUSH         = 15, 
    CLOSE         = 16,
    FSYNC         = 17,
    SETXATTR      = 18,
    REMOVEXATTR   = 19,

    NOTABLE_ACTIONS_COUNT };

	private:
    //! \brief A buffer for updating paths
  char Smart_Path[BUFFER_SIZE];
    //! \brief Holds my startup arguments
  int Argc;
  char** Argv;
    //! \brief The base data directory
  QString Data_Dir;
    //! \brief The base svn directory
  QString Svn_Dir;
    //! \brief The socket id of the port
  int Socket;
    //! \brief Holds the port I should connect on
  int Socket_Port;
    //! \brief My config information
  OrmLight* Config;

    //! \brief This will push a new action on the front of my list
  bool pushAction( NotableAction action, QString str, QString str2 = QString());
	
		public:
		//Constructor and destructor
	FuseCppInterface( int argc, char** argv, OrmLight* config );

    //Set the server port
  void setPort( int port );

    //Starts the main fuse loop
  int startMain();

    //Allows access to the current notable actions
  QList <QPair<NotableAction, QString> >& notableActions();

    //This method switches the path around realtime
  char* smartPath( const char* path, bool svn_dir = false );

    //This method switches the path around for files
  char* smartFilePath( const char* path, bool svn_okay = false );

    //This method returns true if we need to open the svn dir as well
  bool requireSvn( const char* path );

    //! \brief Recursively create directories
  int recursiveMkDir( QString path, mode_t mode );

    //! \brief Returns true if I should ignore an action
  bool ignoreAction( QString my_path );

    ///*** Fuse Interace 
  int fuse_getattr(const char *path, struct stat *stbuf);

  int fuse_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);

  int fuse_access(const char *path, int mask);

  int fuse_readlink(const char *path, char *buf, size_t size);

  int fuse_opendir(const char *path, struct fuse_file_info *fi);

  int fuse_readdir(const char *path,void *buf,fuse_fill_dir_t filler,
		                          off_t offset, struct fuse_file_info *fi);

  int fuse_releasedir(const char *path, struct fuse_file_info *fi);

  int fuse_mknod(const char *path, mode_t mode, dev_t rdev);

  int fuse_mkdir(const char *path, mode_t mode);

  int fuse_unlink(const char *path);

  int fuse_rmdir(const char *path);

  int fuse_symlink(const char *from, const char *to);

  int fuse_rename(const char *from, const char *to);

  int fuse_link(const char *from, const char *to);

  int fuse_chmod(const char *path, mode_t mode);

  int fuse_chown(const char *path, uid_t uid, gid_t gid);

  int fuse_truncate(const char *path, off_t size);

  int fuse_ftruncate(const char *path, off_t size, struct fuse_file_info *fi);

  int fuse_utimens(const char *path, const struct timespec ts[2]);

  int fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi);

  int fuse_open(const char *path, struct fuse_file_info *fi);

  int fuse_read(const char *path, char *buf, size_t size, 
                            off_t offset, struct fuse_file_info *fi);

  int fuse_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi);

  int fuse_statfs(const char *path, struct statvfs *stbuf);

  int fuse_flush(const char *path, struct fuse_file_info *fi);

  int fuse_release(const char *path, struct fuse_file_info *fi);

  int fuse_fsync(const char *path, int isdatasync, struct fuse_file_info *fi);

#ifdef HAVE_SETXATTR
  int fuse_setxattr(const char *path, const char *name, 
                                const char *value, size_t size, int flags);

  int fuse_getxattr(const char *path, const char *name, char *value,
			size_t size);

  int fuse_listxattr(const char *path, char *list, size_t size);

  int fuse_removexattr(const char *path, const char *name);
#endif /* HAVE_SETXATTR */

  int fuse_lock(const char *path, struct fuse_file_info *fi, int cmd,
		    struct flock *lock);
};

#endif
