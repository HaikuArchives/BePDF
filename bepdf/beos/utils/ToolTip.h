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
#ifndef TOOL_TIP_H
#define TOOL_TIP_H

#include <be/interface/Point.h>
#include <be/interface/Window.h>
#include <be/interface/View.h>


class ToolTipItem;

class ToolTip : public BWindow {
protected:
	BView *mView;
	BList mItems;
	
	class View : public BView {
	public:
		View(BRect rect);
		void Draw(BRect update);
		void Pulse();
	};
	friend class ::ToolTip::View;
	
	// state
	int32  mShowItem, mPulseCount;
	uint32 mTime;
	BPoint mPoint;
	
	void Display();
	void Undisplay();
	void Show(BMessage *msg);
	void Hide(BMessage *msg);
	void Update(BMessage *msg);
public:
	enum {
		MSG_SHOW = 'Show',
		// 'id' int32
		// 'point' BPoint
		MSG_HIDE = 'Hide',
		// 'id' int32
		MSG_UPDATE = 'Upda'
		// 'id' int32
	};

	ToolTip();
	virtual ~ToolTip();
	virtual void Show() { BWindow::Show(); }
	virtual void Hide() { BWindow::Hide(); }
	void AddItem(ToolTipItem *item);
	void MessageReceived(BMessage *msg);
};

#endif

