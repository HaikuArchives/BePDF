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
#ifndef MULTI_BUTTON_H
#define MULTI_BUTTON_H

#include <be/interface/Window.h>
#include <be/interface/View.h>
#include <be/interface/Control.h>

class MultiButton : public BView {
private:
	BList    mButtons;
	int      mCurrent;

	void  Next();
	
	class Handler : public BHandler {
	private:
		MultiButton *mMultiButton;
	public:
		Handler(MultiButton* mb) { mMultiButton = mb; }
		virtual void MessageReceived(BMessage *msg);
	} mHandler;
	
	friend class Handler;
	BHandler *GetHandler();
public:
	MultiButton(BRect rect, const char *name, uint32 resizeMask, uint32 flags);
    ~MultiButton();
    
	void AttachedToWindow() { Window()->AddHandler(&mHandler); }
	void AddButton(BControl* button);
	BControl *Current()     { return (BControl*)mButtons.ItemAt(mCurrent); }
	void SetTo(int index);
	int Current() const     { return mCurrent; }
	int Length() const      { return mButtons.CountItems(); }
};

#endif

