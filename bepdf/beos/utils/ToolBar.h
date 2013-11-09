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
#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include <be/interface/Window.h>
#include <be/interface/View.h>
#include <be/support/List.h>


class ToolBar : public BView {
public:
	enum Align {
		none = 0,
		top, 
		center, 
		bottom
	};

	enum Direction {
		horizontal,
		vertical
	};

protected:
	BList mItems;
	
	float mPos, mLength;
	Direction mDirection;
	
public:	
	ToolBar(BRect rect, const char *name, uint32 resizeMask, uint32 flags, Direction dir = horizontal);
	void Add(BView *item, Align align = center);
	void Skip(float distance = 3);
	void AddSeparator();
};

#endif

