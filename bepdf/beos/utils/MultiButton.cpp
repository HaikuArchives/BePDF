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

#include "MultiButton.h"

MultiButton::MultiButton(BRect rect, const char *name, uint32 resizeMask, uint32 flags)
 : BView(rect, name, resizeMask, flags),
 mCurrent(-1),
 mHandler(this)
{
	
}

MultiButton::~MultiButton() {
	BControl *item;
	for (int i = 0; (item = (BControl*)mButtons.ItemAt(i)) != NULL; i++) {
		delete item;
	}
}

void MultiButton::AddButton(BControl *control) {
	mButtons.AddItem(control);
	control->SetTarget(&mHandler, Window());	
	if (mCurrent == -1) {
		mCurrent = 0;
		AddChild(control);
	}
}


void MultiButton::Next() {
	mCurrent ++;
	if (mCurrent >= mButtons.CountItems()) mCurrent = 0;
}

void MultiButton::SetTo(int index) {
	if ((mCurrent != index) && (index >= 0) && (index < mButtons.CountItems())) {
		RemoveChild(Current()); 
		mCurrent = index;
		AddChild(Current());
	}
}

void MultiButton::Handler::MessageReceived(BMessage *msg) {
	BView *cur = mMultiButton->Current();
	mMultiButton->Next();
	BView *view = mMultiButton->Current();
	mMultiButton->RemoveChild(cur); mMultiButton->AddChild(view);
	mMultiButton->Window()->PostMessage(msg);
}
