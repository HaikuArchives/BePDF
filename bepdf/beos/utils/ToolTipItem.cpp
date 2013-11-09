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
#include "ToolTipItem.h"
#include <Message.h>

ToolTipItem::ToolTipItem(BView *view, const char *label) :
	mParent(view), mId(-1), mToolTip(NULL), mLabel(label), 
	mBounds(0, 0, 100, 30) {
}

ToolTipItem::~ToolTipItem() {}

void ToolTipItem::Draw(BView *view, BRect update) {
	font_height height;
	view->GetFontHeight(&height);
	view->DrawString(mLabel.String(), BPoint(3, height.ascent + 2));
}

BRect ToolTipItem::GetBounds(BView *view) {
	font_height height;
	view->GetFontHeight(&height);
	float w = view->StringWidth(mLabel.String());
	mBounds.Set(0, 0, w+4, height.ascent + height.descent-1 + 4);
	return mBounds;
}

void ToolTipItem::Show() {
	if (mToolTip) {
		BMessage msg(ToolTip::MSG_SHOW);
		msg.AddInt32("id", mId);
		/*
		BPoint point; uint32 buttons;
		mParent->GetMouse(&point, &buttons);
		point = mParent->ConvertToScreen(point);
		*/
		BRect bounds(mParent->Bounds());
		BPoint point(mParent->ConvertToScreen(BPoint(bounds.left, bounds.bottom + 3)));
		msg.AddPoint("point", point);
		mToolTip->PostMessage(&msg);
	}
}

void ToolTipItem::Hide() {
	if (mToolTip) {
		BMessage msg(ToolTip::MSG_HIDE);
		msg.AddInt32("id", mId);
		mToolTip->PostMessage(&msg);
	}
}

void ToolTipItem::MouseMoved(BPoint p, uint32 transit, const BMessage *msg) {
	// Note under Zeta transit = B_OUTSIDE_VIEW when the mouse ENTERS
	// the view.
	// TODO remove this workaround when yellowTab has fixed this bug!
	if (transit == B_ENTERED_VIEW || transit == B_OUTSIDE_VIEW) Show();
	else if (transit == B_EXITED_VIEW) Hide();
}

void ToolTipItem::SetLabel(const char *label) {
	if (mToolTip) {
		if (mToolTip->Lock()) {
			mLabel = label;
			mToolTip->Unlock();
		}
		BMessage msg(ToolTip::MSG_UPDATE);
		msg.AddInt32("id", mId);
		mToolTip->PostMessage(&msg);
	} else {
		mLabel = label;
	}
}
