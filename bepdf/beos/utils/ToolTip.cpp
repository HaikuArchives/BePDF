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
#include "ToolTip.h"
#include "ToolTipItem.h"
#include <Message.h>

ToolTip::ToolTip() : 
	BWindow(BRect(10, 20, 10, 20), "Tool Tip",
		B_BORDERED_WINDOW_LOOK,
		// B_FLOATING_APP_WINDOW_FEEL,
		B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_MOVABLE | B_NOT_CLOSABLE| B_AVOID_FOCUS),
	mShowItem(-1), mPulseCount(0), mTime(0) {
	AddChild(mView = new View(BRect(0, 0, (20 - 10), (20 - 10))));
	mView->SetViewColor(255, 255, 180, 255);
	mView->SetLowColor(B_TRANSPARENT_COLOR);
	mView->SetHighColor(0, 0, 0, 255);
	mView->SetDrawingMode(B_OP_OVER);
	SetPulseRate(5 * 100000);
	Show(); Hide();
}

ToolTip::~ToolTip() {
	for (int32 i = mItems.CountItems()-1; i >= 0; i--) {
		ToolTipItem *item =  (ToolTipItem*)mItems.RemoveItem(i);
		item->mToolTip = NULL;
	}
}

ToolTip::View::View(BRect rect) :
	BView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED) {
}

void ToolTip::View::Pulse() {
	::ToolTip *tip = (::ToolTip*)Window();
	if (tip) {
		#define N 2
		#define M 6
		tip->mPulseCount++;
		
		if (tip->mPulseCount == N) {
			tip->Display();
		} else if (tip->mPulseCount == M) {
			tip->Undisplay();
		}
	}
}

void ToolTip::View::Draw(BRect update) {
	::ToolTip *tip = (::ToolTip*)Window();
	ToolTipItem *item = (ToolTipItem*)tip->mItems.ItemAt(tip->mShowItem);
	if (item) {
		item->Draw(this, update);
	}
}

void ToolTip::Display() {
	if (mShowItem == -1) return;
	ToolTipItem *item = (ToolTipItem*)mItems.ItemAt(mShowItem);
	if (item && IsHidden()) {
		BRect bounds(item->GetBounds(mView));
		MoveTo(mPoint);
		ResizeTo(bounds.Width(), bounds.Height());
		SetWorkspaces(B_CURRENT_WORKSPACE);
		Show();
		mView->Invalidate();
	}
}

void ToolTip::Undisplay() {
	mShowItem = -1;
	if (!IsHidden()) { 
		mTime = real_time_clock(); 
		Hide(); 
	}
}

void ToolTip::AddItem(ToolTipItem *item) {
	item->mId = mItems.CountItems();
	item->mToolTip = this;
	mItems.AddItem(item);
}

void ToolTip::Show(BMessage *msg) {
	BPoint p; int32 id;
	if ((msg->FindInt32("id", &id) == B_OK) && 
		(msg->FindPoint("point", &p) == B_OK)) {
		mShowItem = id; mPulseCount = 0;
		mPoint = p;
		uint32 diff = real_time_clock() - mTime;
		if (diff <= 1) Display();
	}
}

void ToolTip::Hide(BMessage *msg) {
	//int32 id;
	//if (msg->FindInt32("id", &id) == B_OK) {
		Undisplay();
	//}
}

void ToolTip::Update(BMessage *msg) {
	int32 id;
	if (msg->FindInt32("id", &id) == B_OK) {
		if ((mShowItem == id) && !IsHidden()) {
			ToolTipItem *item = (ToolTipItem*)mItems.ItemAt(mShowItem);
			if (item) {
				BRect r(Bounds());
				BRect r2(item->GetBounds(mView));
				if (r != r2) {
					ResizeTo(r2.Width(), r2.Height());
				}
				mView->Invalidate();
			}
		}
	}
}

void ToolTip::MessageReceived(BMessage *msg) {
	switch(msg->what) {
	case MSG_SHOW: Show(msg);
		break;
	case MSG_HIDE: Hide(msg);
		break;
	case MSG_UPDATE: Update(msg);
		break;
	default:
		BWindow::MessageReceived(msg);
	}
}

