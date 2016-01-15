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

#include <locale/Catalog.h>
#include <LayoutBuilder.h>
#include <StatusBar.h>
#include <StringView.h>

#include "BeLoadProgressMonitor.h"
#include "BepdfApplication.h"
#include "StatusWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "StatusWindow"

StatusWindow::StatusWindow(const char *text, BRect aRect)
	: BWindow(aRect, B_TRANSLATE("BePDF status"),
		B_MODAL_WINDOW ,
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE) {

	BStringView *stringView = new BStringView("stringView", text);
	mStatus = new BStatusBar("mStatus");
	mStatus->SetMaxValue(1);
	mTotal = -1;

	mText = new BStringView("mText", "");

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0)
		.AddGlue()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(stringView)
			.AddGlue()
		.End()
		.AddGlue()
		.AddGroup(B_HORIZONTAL)
			.Add(mText)
			.AddGlue()
		.End()
		.Add(mStatus);

	ResizeTo(500, 100);

	CenterOnScreen();
	Show();
}

void
StatusWindow::MessageReceived(BMessage *msg) {
	int32 p;
	BString s;
	switch(msg->what) {
	case TOTAL_NOTIFY:
		if (B_OK == msg->FindInt32("total", &p)) {
			mTotal = p;
		}
		break;
	case CURRENT_NOTIFY:
		if (B_OK == msg->FindInt32("current", &p)) {
			mStatus->SetTo(p / (float)mTotal);
		}
		break;
	case TEXT_NOTIFY:
		if (B_OK == msg->FindString("text", &s)) {
			mText->SetText(s.String());
		}
		break;
	default:
		BWindow::MessageReceived(msg);
	}
}

void StatusWindow::SetTotal(BMessenger* msgr, int32 total) {
	BMessage m(TOTAL_NOTIFY);
	m.AddInt32("total", total);
	msgr->SendMessage(&m);
}

void StatusWindow::SetCurrent(BMessenger* msgr, int32 current) {
	BMessage m(CURRENT_NOTIFY);
	m.AddInt32("current", current);
	msgr->SendMessage(&m);
}

void StatusWindow::SetText(BMessenger* msgr, const char* text) {
	BMessage m(TEXT_NOTIFY);
	m.AddString("text", text);
	msgr->SendMessage(&m);
}


// Implementation of ShowStatusWindow

ShowStatusWindow::ShowStatusWindow(const char* name) {
	BWindow* window = new StatusWindow(name, gApp->GetSettings()->GetWindowRect());
	mMessenger = BMessenger(window);
}

ShowStatusWindow::~ShowStatusWindow() {
	mMessenger.SendMessage(B_QUIT_REQUESTED);
}

// Implementation of ShowLoadProgressStatusWindow

ShowLoadProgressStatusWindow::ShowLoadProgressStatusWindow(const char* name)
	: ShowStatusWindow(name)
{
	BeLoadProgressMonitor::getInstance()->setMessenger(GetMessenger());
}

ShowLoadProgressStatusWindow::~ShowLoadProgressStatusWindow() {
	BeLoadProgressMonitor::getInstance()->setMessenger(NULL);
}

