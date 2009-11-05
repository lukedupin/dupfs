/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2005  Miklos Szeredi <miklos@szeredi.hu>

    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#define FUSE_USE_VERSION 26

#define BUFFER_SIZE 512

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char* Real_Path = "/Development";
static char  Real_Path_Tmp[BUFFER_SIZE];

static char* real_path( const char* path )
{
  sprintf( Real_Path_Tmp, "%s%s", Real_Path, path );

  return (char*)Real_Path_Tmp;
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else /*if(strcmp(path, hello_path) == 0)*/ {
        stbuf->st_mode = S_IFREG | 777;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    }
//    else
//        res = -ENOENT;

    return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
  (void) offset;
  (void) fi;

    //Create some basic variables
  char tmp[512];
  FILE* handle = NULL;

    //Create my pipe command
  sprintf( tmp, "ls -a %s", real_path( path ));

    //Do a real ls on my base directory
  if ( (handle = popen( tmp, "r")) == NULL )
    return -ENOENT;

    //Read in all the entries and store them into my filler
  for ( fgets( tmp, sizeof(tmp), handle); !feof( handle ); 
        fgets( tmp, sizeof(tmp), handle) )
  {
    filler( buf, tmp, NULL, 0);
  }
      

      //Add in all my files that we find
/*    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, hello_path + 1, NULL, 0);
*/

  pclose( handle );

  return 0;
}

static int hello_open(const char *path, fuse_file_info* fi)
{
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    if( (fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    len = strlen(hello_str);
    if (offset < (off_t)len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, hello_str + offset, size);
    } else
        size = 0;

    return size;
}

static struct fuse_operations hello_oper;

int main(int argc, char *argv[])
{
  hello_oper.getattr  = hello_getattr;
  hello_oper.readdir  = hello_readdir;
  hello_oper.open = hello_open;
  hello_oper.read = hello_read;

  return fuse_main(argc, argv, &hello_oper, NULL);
}


