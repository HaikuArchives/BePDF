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

#include "ToolBar.h"

#include <be/interface/InterfaceDefs.h>
#include <be/interface/GraphicsDefs.h>

ToolBar::ToolBar(BRect rect, const char *name, uint32 resizeMask, uint32 flags, Direction dir)
 : BView(rect, name, resizeMask, flags)
 , mDirection(dir)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	mPos = 3;
	if (mDirection == horizontal) {
		// for horizontal direction
		mLength = rect.Height()+1;
	} else {
		// for vertical direction
		mLength = rect.Width()+1;
	}
}


void ToolBar::Add(BView *view, Align align) {
	float x, y, l, h, d;
	
	mItems.AddItem(view);
	BRect r(view->Frame());
	
	x = r.left;
	y = r.top;
	
	if (mDirection == horizontal) {
		l = r.Height() + 1;
		h = y;
		d = r.Width();
	} else {
		l = r.Width() + 1;
		h = x;
		d = r.Height();
	}	
	
	switch (align) {
		case none: break;
		case top: h = 0;
			break;
		case center: h = (mLength - l) / 2;
			break;
		case bottom: h = mLength - l;
			break;
	}
	
	if (mDirection == horizontal) {
		x = mPos;
		y = h;
	} else {
		x = h;
		y = mPos;
	}
	
	view->MoveTo(x, y);
	AddChild(view);
	Skip(d+2);
}

void ToolBar::Skip(float distance) {
	mPos += distance;
}

class TBSeparator : public BView {
public:
	TBSeparator(BRect rect) : 
		BView (rect, NULL, B_FOLLOW_NONE, B_WILL_DRAW) {
		SetHighColor(0, 0, 0, 0); SetLowColor(247, 247, 247, 0);	
	}

	void Draw(BRect rect) {
		BRect r(Bounds());
		if (r.Width() <= r.Height()) {
			FillRect(BRect(r.left, r.top, r.left, r.bottom), B_SOLID_HIGH);
			FillRect(BRect(r.right, r.top, r.right, r.bottom), B_SOLID_LOW);
		} else {
			FillRect(BRect(r.left, r.top, r.right, r.top), B_SOLID_HIGH);
			FillRect(BRect(r.left, r.bottom, r.right, r.bottom), B_SOLID_LOW);
		}
	}
};

void ToolBar::AddSeparator() {
	float w, h;
	if (mDirection == horizontal) {
		w = 1;
		h = mLength - 10;
	} else {
		w = mLength - 10;
		h = 1;
	}
	Skip(1);
	Add(new TBSeparator(BRect(0, 0, w, h)));
	Skip(1);
}
