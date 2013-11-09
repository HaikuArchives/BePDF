/*  
	Copyright (C) 2000 Michael Pfeiffer
	
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
#ifndef TOOL_TIP_ITEM_H
#define TOOL_TIP_ITEM_H

#include "ToolTip.h"

#include <be/interface/Point.h>
#include <be/interface/Window.h>
#include <be/interface/View.h>
#include <be/support/String.h>

class ToolTipItem {
protected:
	BView *mParent;
	int32 mId;
	ToolTip *mToolTip;
	BString mLabel;
	BRect mBounds;
public:
	friend class ToolTip;
	
	ToolTipItem(BView *view, const char *label);
	virtual ~ToolTipItem();
	
	// hook functions
	virtual void Draw(BView *view, BRect update);
	virtual BRect GetBounds(BView *view);
	
	virtual void Show();
	virtual void Hide();
	void MouseMoved(BPoint p, uint32 transit, const BMessage *msg);
	void SetLabel(const char *label);
};

#endif

