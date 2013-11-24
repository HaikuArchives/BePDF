/*  
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef FIND_TEXT_WINDOW_H
#define FIND_TEXT_WINDOW_H
#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <Looper.h>

#include <MWindow.h>
#include <MButton.h>
#include <MCheckBox.h>
#include <MTextControl.h>
#include <MStringView.h>

#include "Settings.h"

class FindTextWindow : public MWindow {
public:
	FindTextWindow(GlobalSettings *settings, const char *text, BLooper *looper);

	void MessageReceived(BMessage *msg);

	enum {
		FIND_START_NOTIFY_MSG     = 'TSrt',
		FIND_STOP_NOTIFY_MSG      = 'TStp',
		FIND_ABORT_NOTIFY_MSG     = 'SAbt',
		TEXT_FOUND_NOTIFY_MSG     = 'TFnd',
		TEXT_NOT_FOUND_NOTIFY_MSG = 'TNFd',
		FIND_SET_PAGE_MSG         = 'FStP',
		FIND_QUIT_REQUESTED_MSG   = 'QTrq',
		FIND_MSG                  = 'Find',
		FIND_STOP_MSG             = 'Stop',
		FIND_ABORT_MSG            = 'Abrt',
		FIND_IGNORE_CASE_MSG      = 'TIgr',
		FIND_BACKWARD_MSG         = 'TRvr',
	};

	virtual void FrameMoved(BPoint point);
	virtual void FrameResized(float w, float h);

	bool QuitRequested();

protected:
	void SetPage(int32 page);

	bool mSearching;
	BLooper* mLooper;
	GlobalSettings* mSettings;
	MButton*      mFindStop;
	MTextControl* mText;
	MCheckBox*    mIgnoreCase;
	MCheckBox*    mBackward;
	MStringView*  mPage;
};
#endif
