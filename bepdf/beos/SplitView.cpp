/*  
	BeOS Front-end to PDF file reader xpdf.
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

#include "SplitView.h"
#include "BepdfApplication.h"
#include "PDFWindow.h"

#include <be/app/AppDefs.h>
#include <be/interface/Window.h>
#include <be/interface/ScrollBar.h>

SplitView::SplitView(BRect frame, const char *name, BView *leftView, BView *rightView, int leftMinWidth, int rightMinWidth) 
	: BView(frame, name, B_FOLLOW_ALL_SIDES, 0)
	, mLeftView(leftView)
	, mRightView(rightView)
	, mLeftMinWidth(leftMinWidth)
	, mRightMinWidth(rightMinWidth)
	, mAllowUserSplit(true)
	, mListener(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	mDrag = false;
	AddChild(mLeftView);
	AddChild(mRightView);
	mViewCursor = gApp->splitVCursor;
	mPosition = 0;
}

void SplitView::SetListener(PositionChangedListener* listener) {
	mListener = listener;
}

void SplitView::UncheckedSplit(float x) {
	float width = Bounds().Width();
	mPosition = x;
	float height = mLeftView->Bounds().Height();
	mLeftView->ResizeTo(x-1, height);

	BRect b(mRightView->Frame());
	height = b.Height();	
	mRightView->MoveTo(x + SEPARATION, b.top);
	mRightView->ResizeTo(width - x - SEPARATION + 2 , height);
	PositionChanged();
}

void SplitView::Split(float x) {
	float width = Bounds().Width();
	if (x < mLeftMinWidth+1) {
		x = mLeftMinWidth;
	} else if (width - x - SEPARATION+1 < mRightMinWidth) {
		x = width - mRightMinWidth - SEPARATION+1;
	}
	UncheckedSplit(x);
}

void SplitView::SetLeftMinWidth(int width) {
	mLeftMinWidth = width; 
	Split(mPosition);
}

void SplitView::SetRightMinWidth(int width) {
	mRightMinWidth = width; 
	Split(mPosition);
}

void SplitView::AllowUserSplit(bool allow) {
	mAllowUserSplit = allow;
	BCursor* cursor;
	if (allow) {
		cursor = gApp->splitVCursor;
	} else {
		cursor = (BCursor*)B_CURSOR_SYSTEM_DEFAULT;
	}
	SetViewCursor(cursor, true);
}

void SplitView::AttachedToWindow() {
	SetViewCursor(mViewCursor);
}

void SplitView::SetViewCursor(BCursor *cursor, bool sync) {
	if (Window()->Lock()) {
		mViewCursor = cursor;
		BView::SetViewCursor(cursor, sync);
		Window()->Unlock();
	} else {
		mViewCursor = cursor;
	}
}	

void SplitView::MouseDown(BPoint p) {
	if (mAllowUserSplit) {
		mDrag = true;
		SetMouseEventMask(B_POINTER_EVENTS);
	}
}

void SplitView::MouseMoved(BPoint p, uint32 transit, const BMessage *msg) {
	if (transit == B_ENTERED_VIEW && mViewCursor != NULL && Window()->Lock()) {
		BView::SetViewCursor(mViewCursor);
		Window()->Unlock();
		mViewCursor = NULL;
	}
	if (mDrag && mAllowUserSplit) {
		Split(p.x);
	}
}

void SplitView::MouseUp(BPoint p) {
	if (mDrag) {
		mDrag = false;
		if (mAllowUserSplit) {
			Split(p.x);
		}
	}
}

void SplitView::PositionChanged() {
	if (mListener) {
		mListener->PositionChanged(this, mPosition);
	}
}
