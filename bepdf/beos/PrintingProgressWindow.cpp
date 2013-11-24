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

#include "PrintingProgressWindow.h"
#include <stdio.h>
#include <layout-all.h>
#include "StringLocalization.h"

PrintingProgressWindow::PrintingProgressWindow(const char *text, BRect aRect, int32 pages) 
	: MWindow(aRect, TRANSLATE("BePDF Printing"), 
		B_TITLED_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE) {
	mPages = pages; mPrintedPages = 0;
	mState = OK;
	
	BString s(TRANSLATE("BePDF printing document: "));
	s += text;
	// center window
	aRect.OffsetBy(aRect.Width() / 2, aRect.Height() / 2);
	float width = 300, height = 60;
	aRect.SetRightBottom(BPoint(aRect.left + width, aRect.top + height));
	aRect.OffsetBy(-aRect.Width() / 2, -aRect.Height() / 2);
	MoveTo(aRect.left, aRect.top);
	ResizeTo(width, height);

	MStringView *stringView = new MStringView(s.String());

	mPageString = new MStringView(TRANSLATE("Page:"));

	mProgress = new MProgressBar(this);
	
	mStop = new MButton(TRANSLATE("Stop"), new BMessage('STOP'));
	mAbort = new MButton(TRANSLATE("Abort"), new BMessage('ABRT'));


	MGroup *view = new HGroup(
		new Space(minimax(5, 0, 5, 0, -1)),
		new VGroup(
			stringView,
			mPageString,
			new Space(minimax(0, 5, 0, 5)),
			mProgress,
			new Space(minimax(0, 5, 0, 5)),
			// new MBViewWrapper(mProgress),
			0),
		new Space(minimax(5, 0, 5, 0, -1)),
		new VGroup(
			mStop,
			mAbort,
			0),
		new Space(minimax(2, 0, 2, 0, -1)),
		0);

	AddChild(dynamic_cast<BView*>(view));
	SetDefaultButton(mStop);
	Show();
}

void PrintingProgressWindow::SetPage(int32 page) {
	char buffer[256];
	sprintf(buffer, TRANSLATE("Page: %d"), page);
	Lock();
	mPageString->SetText(buffer);
	mPrintedPages ++;
	mProgress->SetValue(mPrintedPages / (float)mPages);
	Unlock();
}

bool PrintingProgressWindow::Aborted() {
	return mState == ABORTED;
}

bool PrintingProgressWindow::Stopped() {
	return mState == STOPPED;
}

void PrintingProgressWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case 'ABRT':
		mState = ABORTED;
		mStop->SetEnabled(false); mAbort->SetEnabled(false);
		break;
	case 'STOP':
		mState = STOPPED;
		mStop->SetEnabled(false); mAbort->SetEnabled(false);
		break;
	default:
		BWindow::MessageReceived(msg);
	}
}

// PrintingHiddenWindow
PrintingHiddenWindow::PrintingHiddenWindow(BRect aRect) 
	: BWindow(aRect, "BePDF Printing Hidden Window", 
		B_FLOATING_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL, 
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE) {
	Show();
}


