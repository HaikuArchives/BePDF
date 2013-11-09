/*  
	BeOS Front-end to PDF file reader xpdf.
 	Copyright (C) 2000-2003 Michael Pfeiffer

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
#ifndef SPLIT_VIEW_H
#define SPLIT_VIEW_H

#include <be/interface/View.h>

class SplitView;

class PositionChangedListener {
public:
	virtual void PositionChanged(SplitView* view, float position) = 0;
};

class SplitView : public BView {
	BView   *mLeftView, 
	        *mRightView;
	int     mLeftMinWidth,
			mRightMinWidth;
	bool    mAllowUserSplit;
	bool    mDrag;
	float   mPosition;	
	BCursor *mViewCursor;
	PositionChangedListener *mListener;
	
public:
	enum {
		SEPARATION = 4,
	};

	SplitView(BRect frame, const char *name, BView *leftView, BView *rightView, int leftMinWidth, int rightMinWidth);
	void SetListener(PositionChangedListener *listener);
	void UncheckedSplit(float x);
	void Split(float x);
	float GetSplitPosition() const { return mPosition; }
	
	void SetLeftMinWidth(int width);
	void SetRightMinWidth(int width);
	int GetLeftMinWidth() const { return mLeftMinWidth; }
	int GetRightMinWidth() const { return mRightMinWidth; }
	
	void AllowUserSplit(bool allow);
	bool IsUserSplitAllowed() const { return mAllowUserSplit; }

	void AttachedToWindow();
	void SetViewCursor(BCursor *cursor, bool sync = true);

	void MouseDown(BPoint p);
	void MouseMoved(BPoint p, uint32 transit, const BMessage *msg);
	void MouseUp(BPoint p);
	
	BView *Left() { return mLeftView; };
	BView *Right() { return mRightView; };
	
	virtual void PositionChanged();
};

#endif
