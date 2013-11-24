/*  
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _BEPDF_H
#define _BEPDF_H

#include <be/support/Locker.h>
#include <be/interface/Screen.h>


#if 1
  // enable locking
  #define LOCKER_CLASS BLocker
#else
  // disable locking
  #define LOCKER_CLASS MyLocker

class MyLocker {
public:
	bool     Lock()                             { return true; }
	status_t LockWithTimeout(bigtime_t timeout) { return B_OK; }
	void     Unlock()                           { }
};

#endif

// global lock 
extern LOCKER_CLASS *gPdfLock;
// the BScreen initialized with InitBePDF()
extern BScreen *gScreen;

// Automatically locks and unlocks gPdfLock in constructor and destructor.
class PDFLock {
public:
	PDFLock()  { gPdfLock->Lock();   }
	~PDFLock() { gPdfLock->Unlock(); }
};

// Initializes global variables
void InitBePDF();
// Frees global variables
void ExitBePDF();

#endif