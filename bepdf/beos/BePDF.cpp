/*  
	BeOS Front-end du PDF file reader xpdf.
	Copyright (C) 2004-2005 Michael Pfeiffer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "BePDF.h"

LOCKER_CLASS *gPdfLock   = NULL;
BScreen *gScreen = NULL;

void InitBePDF() {
	gScreen = new BScreen();
	gPdfLock   = new LOCKER_CLASS();
}

void ExitBePDF() {
	delete gScreen;
	gScreen = NULL;
	
	delete gPdfLock;
	gPdfLock = NULL;
}
