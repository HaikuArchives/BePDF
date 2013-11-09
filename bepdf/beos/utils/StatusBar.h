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
#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <be/interface/View.h>

class StatusBar : public BView {
protected:

public:
	StatusBar(BRect rect, const char *name, uint32 resizeMask, uint32 flags);
	void Draw(BRect r);
};

#endif

