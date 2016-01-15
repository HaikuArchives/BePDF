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
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <TextControl.h>

#include "FindTextWindow.h"
#include "LayoutUtils.h"
#include "TextConversion.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FindTextWindow"

FindTextWindow::FindTextWindow(GlobalSettings *settings, const char *text,
	BLooper *looper)
	: BWindow(BRect(0, 0, 0, 0),
		B_TRANSLATE("Find text"),
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
		mLooper(looper),
		mSettings(settings)
{

	mSearching = false;

	MoveTo(settings->GetFindWindowPosition());
	float w, h;
	settings->GetFindWindowSize(w, h);
	ResizeTo(w, h);

	AddCommonFilter(new EscapeMessageFilter(this, FIND_ABORT_MSG));

	mText = new BTextControl("mText", B_TRANSLATE("Find: "), text, NULL);
	mText->TextView()->DisallowChar(B_ESCAPE);

	mPage = new BStringView("mPage", B_TRANSLATE("Page:"));

	mFindStop = new BButton("mFindStop", B_TRANSLATE("Find"),
		new BMessage(FIND_MSG));

	mIgnoreCase = new BCheckBox("mIgnoreCase", B_TRANSLATE("Ignore case"),
		new BMessage(FIND_IGNORE_CASE_MSG));
	mIgnoreCase->SetValue(settings->GetFindIgnoreCase());

	mBackward = new BCheckBox("mBackward", B_TRANSLATE("Search backwards"),
		new BMessage(FIND_BACKWARD_MSG));
	mBackward->SetValue(settings->GetFindBackward());

	BGroupLayout *optBox = BLayoutBuilder::Group<>(B_HORIZONTAL)
		.SetInsets(B_USE_SMALL_SPACING)
		.AddGroup(B_VERTICAL)
			.Add(mIgnoreCase)
			.Add(mBackward)
		.End()
		.AddGlue();

	BBox *options = new BBox("options");
	options->SetLabel(B_TRANSLATE("Options"));
	options->AddChild(optBox->View());

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(mText)
		.AddGroup(B_HORIZONTAL)
			.Add(options)
			.AddGroup(B_VERTICAL)
				.Add(mFindStop)
				.Add(mPage)
				.AddGlue()
			.End()
		.End();

	SetDefaultButton(mFindStop);

	mText->MakeFocus();
	Show();
}

void FindTextWindow::FrameMoved(BPoint point) {
	BWindow::FrameMoved(point);
	mSettings->SetFindWindowPosition(point);
}

void FindTextWindow::FrameResized(float w, float h) {
	BWindow::FrameResized(w, h);
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
		mFindStop->SetLabel(B_TRANSLATE("Stop"));
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
		mFindStop->SetLabel(B_TRANSLATE("Find"));
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
		BWindow::MessageReceived(msg);
	}
}

void FindTextWindow::SetPage(int32 page) {
	const char* fmt = B_TRANSLATE("Page: %d");
	char* buffer = new char[strlen(fmt)+30];
	sprintf(buffer, fmt, page);
	mPage->SetText(buffer);
	delete buffer;
}
