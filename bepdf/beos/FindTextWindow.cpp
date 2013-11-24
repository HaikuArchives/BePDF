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

#include "FindTextWindow.h"
#include <stdio.h>
#include <layout-all.h>
#include "LayoutUtils.h"
#include "TextConversion.h"
#include "StringLocalization.h"

FindTextWindow::FindTextWindow(GlobalSettings *settings, const char *text, 
	BLooper *looper) 
	: MWindow(BRect(0, 0, 100, 100), 
		TRANSLATE("Find Text"), 
		B_FLOATING_WINDOW_LOOK, 
		B_MODAL_APP_WINDOW_FEEL, 
		B_NOT_ZOOMABLE),
		mLooper(looper), 
		mSettings(settings) 
{
	
	mSearching = false;
	
	MoveTo(settings->GetFindWindowPosition());
	float w, h;
	settings->GetFindWindowSize(w, h);
	ResizeTo(w, h);
	
	AddCommonFilter(new EscapeMessageFilter(this, FIND_ABORT_MSG));
	
	mText = new MTextControl(TRANSLATE("Find: "), (char*)text, NULL);
	mText->SetDivider(25);
	mText->TextView()->DisallowChar(B_ESCAPE);

	mPage = new MStringView(TRANSLATE("Page:"));

	mFindStop = new MButton(TRANSLATE("Find"), new BMessage(FIND_MSG));

	mIgnoreCase = new MCheckBox(TRANSLATE("Ignore Case"), new BMessage(FIND_IGNORE_CASE_MSG), NULL, settings->GetFindIgnoreCase());
	mBackward = new MCheckBox(TRANSLATE("Search Backwards"), new BMessage(FIND_BACKWARD_MSG), NULL, settings->GetFindBackward());

	MGroup *view = new HGroup(
		new Space(GetBorder()),
		
		new VGroup(
			new Space(GetBorder()),
			mText,
			mIgnoreCase,
			mBackward,
			mPage,
			GetFiller(),
			0),
		
		new Space(GetSeparator()),
		
		new VGroup(
			new Space(GetBorder()),
			mFindStop,
			new Space(GetBorder()),
			0),
		
		new Space(GetBorder()),
		0);

	AddChild(dynamic_cast<BView*>(view));
	SetDefaultButton(mFindStop);

	mText->MakeFocus();
	Show();
}

void FindTextWindow::FrameMoved(BPoint point) {
	MWindow::FrameMoved(point);
	mSettings->SetFindWindowPosition(point);
}

void FindTextWindow::FrameResized(float w, float h) {
	MWindow::FrameResized(w, h);
	mSettings->SetFindWindowSize(w, h);
}

bool FindTextWindow::QuitRequested() {
	if (mSearching) {
		mLooper->PostMessage((uint32)FIND_ABORT_NOTIFY_MSG, NULL);
		return false;
	}
	return true;
}

void FindTextWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case FIND_IGNORE_CASE_MSG:
		mSettings->SetFindIgnoreCase(mIgnoreCase->Value() == B_CONTROL_ON);
		break;
	case FIND_BACKWARD_MSG:
		mSettings->SetFindBackward(mBackward->Value() == B_CONTROL_ON);
		break;
	case FIND_ABORT_MSG:
		mLooper->PostMessage((uint32)FIND_ABORT_NOTIFY_MSG, NULL);
		break;
	case FIND_MSG: {
		mSearching = true;
		const char *text = mText->Text();
		if (strlen(text) == 0) return;
		mFindStop->SetLabel(TRANSLATE("Stop"));
		mFindStop->SetMessage(new BMessage(FIND_STOP_MSG));
		mText->SetEnabled(false);
		
		BMessage msg(FIND_START_NOTIFY_MSG);
		msg.AddString("text", text);
		msg.AddBool("ignoreCase", mIgnoreCase->Value() == B_CONTROL_ON);
		msg.AddBool("backward", mBackward->Value() == B_CONTROL_ON);
		mLooper->PostMessage(&msg, NULL);
		break; }
	case FIND_ABORT_NOTIFY_MSG:
		mFindStop->SetEnabled(false); 
		break;
	case FIND_STOP_MSG:
		mLooper->PostMessage((uint32)FIND_STOP_NOTIFY_MSG, NULL);
		break;
	case FIND_STOP_NOTIFY_MSG:
		mFindStop->SetLabel(TRANSLATE("Find"));
		mFindStop->SetMessage(new BMessage(FIND_MSG));
		mText->SetEnabled(true);
		mSearching = false;
		break;
	case FIND_SET_PAGE_MSG: {
		int32 page;
		msg->FindInt32("page", &page);
		SetPage(page);
		break; }
	case FIND_QUIT_REQUESTED_MSG:
		mSearching = false;
		PostMessage(B_QUIT_REQUESTED);
		break;
	default:
		MWindow::MessageReceived(msg);
	}
}

void FindTextWindow::SetPage(int32 page) {
	const char* fmt = TRANSLATE("Page: %d");
	char* buffer = new char[strlen(fmt)+30];
	sprintf(buffer, fmt, page);
	mPage->SetText(buffer);
	delete buffer;
}
