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

#ifndef PRINTING_PROGRESS_WINDOW_H
#define PRINTING_PROGRESS_WINDOW_H
#include <MWindow.h>
#include <MButton.h>
#include <MStringView.h>
#include <MProgressBar.h>
#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <StatusBar.h>

class PrintingProgressWindow : public MWindow {
public:
	PrintingProgressWindow(const char *text, BRect, int32 pages);
	void SetPage(int32 page);
	bool Aborted();
	bool Stopped();
	void MessageReceived(BMessage *msg);
protected:
	enum { OK, STOPPED, ABORTED } mState;
	int32 mPages, mPrintedPages;
	MStringView *mPageString;
	MButton *mAbort, *mStop;
	MProgressBar * /*BStatusBar **/mProgress;
};

class PrintingHiddenWindow : public BWindow {
public:
	PrintingHiddenWindow(BRect);
};
#endif
