/*  
	BeOS Front-end of PDF file reader xpdf.
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
#include "BeLoadProgressMonitor.h"
#include "BepdfApplication.h"
#include "StatusWindow.h"
#include "StringLocalization.h"
#include <layout-all.h>

// Implementation of StatusWindow

StatusWindow::StatusWindow(const char *text, BRect aRect) 
	: MWindow(aRect, TRANSLATE("BePDF Status"), 
		B_MODAL_WINDOW , 
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE) {
	// center window
	aRect.OffsetBy(aRect.Width() / 2, aRect.Height() / 2);
	float width = 300, height = 60;
	aRect.SetRightBottom(BPoint(aRect.left + width, aRect.top + height));
	aRect.OffsetBy(-aRect.Width() / 2, -aRect.Height() / 2);
	MoveTo(aRect.left, aRect.top);
	ResizeTo(width, height);
	// view for the background color

	MStringView *stringView = new MStringView(text);
	mStatus = new MProgressBar(NULL, false);
	mTotal = -1;

	mText = new MStringView("");
	
	MGroup *view = new VGroup(
		new Space(),
		stringView,
		new Space(),
		mText,
		mStatus,
		0);
	stringView->SetAlignment(B_ALIGN_CENTER);
	AddChild(dynamic_cast<BView*>(view));
	
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
			mStatus->SetValue(p / (float)mTotal);
		}
		break;
	case TEXT_NOTIFY:
		if (B_OK == msg->FindString("text", &s)) {
			mText->SetText(s.String());
		}		
		break;
	default:
		MWindow::MessageReceived(msg);
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

