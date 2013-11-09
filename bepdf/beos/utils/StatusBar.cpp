/*  
	Copyright (C) 2001 Michael Pfeiffer
	
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

#include "StatusBar.h"

#include <be/interface/InterfaceDefs.h>

StatusBar::StatusBar(BRect rect, const char *name, uint32 resizeMask, uint32 flags)
 : BView(rect, name, resizeMask, B_WILL_DRAW /*| B_FULL_UPDATE_ON_RESIZE*/) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

#include <stdio.h>
void StatusBar::Draw(BRect r) {
	BRect b(Bounds());

	SetHighColor(150, 150, 150, 255);
	BPoint q(b.right, 0);
	StrokeLine(BPoint(0, 0), q);
//	StrokeLine(q, BPoint(b.right, b.bottom)); 

	SetHighColor(255, 255, 255, 0);
	BPoint p(0, 1);
	StrokeLine(p, BPoint(b.right/*-1*/, 1));
	StrokeLine(p, BPoint(0, b.bottom)); 

	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(BRect(1, 2, b.right/*-1*/, b.bottom));
}

