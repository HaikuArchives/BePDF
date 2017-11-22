//========================================================================
//
// gfile.cc
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef _WIN32
#  include <time.h>
#  include <direct.h>
#else
#  if defined(MACOS)
#    include <sys/stat.h>
#  elif !defined(ACORN)
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#  endif
#  include <time.h>
#  include <limits.h>
#  include <string.h>
#  if !defined(VMS) && !defined(ACORN) && !defined(MACOS)
#    include <pwd.h>
#  endif
#  if defined(VMS) && (__DECCXX_VER < 50200000)
#    include <unixlib.h>
#  endif
#endif // _WIN32
#include "GString.h"
#include "gfile.h"

#include "GDir.h"

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

GDirEntry::GDirEntry(char *dirPath, char *nameA, GBool doStat) {
#ifdef VMS
  char *p;
#elif defined(_WIN32)
  int fa;
  GString *s;
#elif defined(ACORN)
#else
  struct stat st;
  GString *s;
#endif

  name = new GString(nameA);
  dir = gFalse;
  if (doStat) {
#ifdef VMS
    if (!strcmp(nameA, "-") ||
	((p = strrchr(nameA, '.')) && !strncmp(p, ".DIR;", 5)))
      dir = gTrue;
#elif defined(ACORN)
#else
    s = new GString(dirPath);
    appendToPath(s, nameA);
#ifdef _WIN32
    fa = GetFileAttributesA(s->getCString());
    dir = (fa != 0xFFFFFFFF && (fa & FILE_ATTRIBUTE_DIRECTORY));
#else
    if (stat(s->getCString(), &st) == 0)
      dir = S_ISDIR(st.st_mode);
#endif
    delete s;
#endif
  }
}

GDirEntry::~GDirEntry() {
  delete name;
}

GDir::GDir(char *name, GBool doStatA) {
  path = new GString(name);
  doStat = doStatA;
#if defined(_WIN32)
  GString *tmp;

  tmp = path->copy();
  tmp->append("/*.*");
  hnd = FindFirstFileA(tmp->getCString(), &ffd);
  delete tmp;
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(ANDROID)
#else
  dir = opendir(name);
#ifdef VMS
  needParent = strchr(name, '[') != NULL;
#endif
#endif
}

GDir::~GDir() {
  delete path;
#if defined(_WIN32)
  if (hnd) {
    FindClose(hnd);
    hnd = NULL;
  }
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(ANDROID)
#else
  if (dir)
    closedir(dir);
#endif
}

GDirEntry *GDir::getNextEntry() {
  GDirEntry *e;

#if defined(_WIN32)
  if (hnd) {
    e = new GDirEntry(path->getCString(), ffd.cFileName, doStat);
    if (hnd  && !FindNextFileA(hnd, &ffd)) {
      FindClose(hnd);
      hnd = NULL;
    }
  } else {
    e = NULL;
  }
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(ANDROID)
#elif defined(VMS)
  struct dirent *ent;
  e = NULL;
  if (dir) {
    if (needParent) {
      e = new GDirEntry(path->getCString(), "-", doStat);
      needParent = gFalse;
      return e;
    }
    ent = readdir(dir);
    if (ent) {
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
    }
  }
#else
  struct dirent *ent;
  e = NULL;
  if (dir) {
    ent = (struct dirent *)readdir(dir);
    if (ent && !strcmp(ent->d_name, ".")) {
      ent = (struct dirent *)readdir(dir);
    }
    if (ent) {
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
    }
  }
#endif

  return e;
}

void GDir::rewind() {
#ifdef _WIN32
  GString *tmp;

  if (hnd)
    FindClose(hnd);
  tmp = path->copy();
  tmp->append("/*.*");
  hnd = FindFirstFileA(tmp->getCString(), &ffd);
  delete tmp;
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(ANDROID)
#else
  if (dir)
    rewinddir(dir);
#ifdef VMS
  needParent = strchr(path->getCString(), '[') != NULL;
#endif
#endif
}
