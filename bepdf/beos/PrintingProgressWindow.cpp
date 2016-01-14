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

#include <stdio.h>

#include <locale/Catalog.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StatusBar.h>
#include <StringView.h>

#include "PrintingProgressWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrintingProgressWindow"

PrintingProgressWindow::PrintingProgressWindow(const char *text, BRect aRect, int32 pages)
	: BWindow(aRect, B_TRANSLATE("BePDF Printing"),
		B_TITLED_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE|B_AUTO_UPDATE_SIZE_LIMITS) {
	mPages = pages; mPrintedPages = 0;
	mState = OK;

	BString s(B_TRANSLATE("BePDF printing document: "));
	s << text;
	// center window

	BStringView *stringView = new BStringView("stringView", s.String());

	mPageString = new BStringView("mPageString", B_TRANSLATE("Page:"));

	mProgress = new BStatusBar("mProgress");
	mProgress->SetMaxValue(1);

	mStop = new BButton("mStop", B_TRANSLATE("Stop"), new BMessage('STOP'));
	mAbort = new BButton("mAbort", B_TRANSLATE("Abort"), new BMessage('ABRT'));

	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_VERTICAL)
			.Add(stringView)
			.Add(mPageString)
			.Add(mProgress)
		.End()
		.AddGroup(B_VERTICAL)
			.Add(mStop)
			.Add(mAbort)
		.End();

	ResizeToPreferred();
	CenterOnScreen();

	SetDefaultButton(mStop);
	Show();
}

void PrintingProgressWindow::SetPage(int32 page) {
	char buffer[256];
	sprintf(buffer, B_TRANSLATE("Page: %d"), page);
	Lock();
	mPageString->SetText(buffer);
	mPrintedPages ++;
	mProgress->SetTo(mPrintedPages / (float)mPages);
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


