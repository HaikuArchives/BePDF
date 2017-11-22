//========================================================================
//
// GDir.h <copied from the old gfile.h by waddlesplash>
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GDIR_H
#define GDIR_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#if defined(_WIN32)
#  include <sys/stat.h>
#  ifdef FPTEX
#    include <win32lib.h>
#  else
#    include <windows.h>
#  endif
#elif defined(ACORN)
#elif defined(MACOS)
#  include <ctime.h>
#elif defined(ANDROID)
#else
#  include <unistd.h>
#  include <sys/types.h>
#  ifdef VMS
#    include "vms_dirent.h"
#  elif 1
#    include <dirent.h>
#    define NAMLEN(d) strlen((d)->d_name)
#  else
#    define dirent direct
#    define NAMLEN(d) (d)->d_namlen
#    if HAVE_SYS_NDIR_H
#      include <sys/ndir.h>
#    endif
#    if HAVE_SYS_DIR_H
#      include <sys/dir.h>
#    endif
#    if HAVE_NDIR_H
#      include <ndir.h>
#    endif
#  endif
#endif
#include "gtypes.h"

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

class GDirEntry {
public:

  GDirEntry(char *dirPath, char *nameA, GBool doStat);
  ~GDirEntry();
  GString *getName() { return name; }
  GBool isDir() { return dir; }

private:

  GString *name;		// dir/file name
  GBool dir;			// is it a directory?
};

class GDir {
public:

  GDir(char *name, GBool doStatA = gTrue);
  ~GDir();
  GDirEntry *getNextEntry();
  void rewind();

private:

  GString *path;		// directory path
  GBool doStat;			// call stat() for each entry?
#if defined(_WIN32)
  WIN32_FIND_DATAA ffd;
  HANDLE hnd;
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(ANDROID)
#else
  DIR *dir;			// the DIR structure from opendir()
#ifdef VMS
  GBool needParent;		// need to return an entry for [-]
#endif
#endif
};

#endif
