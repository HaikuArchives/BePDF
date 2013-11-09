/*  
    BeOS Front-end of PDF file reader xpdf.
	Copyright (C) 2000-2002 Michael Pfeiffer
	
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

#include "PasswordWindow.h"
#include <layout-all.h>
#include <MenuItem.h>
#include "LayoutUtils.h"
#include "StringLocalization.h"

// remember last settings in class static variable
enum PasswordWindow::PwdKind PasswordWindow::mPwdKind = USER_PASSWORD;

PasswordWindow::PasswordWindow(entry_ref *ref, BRect aRect, BLooper *looper) 
	: MWindow(aRect, TRANSLATE("Enter Password"), 
		B_TITLED_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL, 
		B_NOT_ZOOMABLE) {
	mLooper = looper;
	mPasswordSent = false;
	mEntry = *ref;
	
	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));
	
	// center window
	aRect.OffsetBy(aRect.Width() / 2, aRect.Height() / 2);
	float width = 300, height = 45;
	aRect.SetRightBottom(BPoint(aRect.left + width, aRect.top + height));
	aRect.OffsetBy(-aRect.Width() / 2, -aRect.Height() / 2);
	MoveTo(aRect.left, aRect.top);
	ResizeTo(width, height);

	MPopup *pwdKind = new MPopup("", NULL);
	BMenuItem *item;
	pwdKind->Menu()->AddItem(item = new BMenuItem(TRANSLATE("User Password"), new BMessage('user'))); 
	if (mPwdKind == USER_PASSWORD) item->SetMarked(true);
	pwdKind->Menu()->AddItem(item = new BMenuItem(TRANSLATE("Owner Password"), new BMessage('ownr'))); 
	if (mPwdKind == OWNER_PASSWORD) item->SetMarked(true);
	pwdKind->Menu()->SetLabelFromMarked(true);
	
	mPassword = new MTextView(minimax(0, 15));
	mPassword->HideTyping(true);
	mPassword->SetViewColor(255, 255, 255);


	MButton *button = new MButton(TRANSLATE("OK"), new BMessage('OK'), NULL, minimax(60, 30, 100, 40, 1));

	MGroup *view = new HGroup(
		new Space(minimax(5, 0, 5, 0, 1)),
		pwdKind,
		new VGroup(
			new Space(),
			new MScrollView(mPassword),
			new Space(),
			NULL
		),
		new Space(minimax(10, 0, 10, 0, 1)), 
		button, 
		new Space(minimax(5, 0, 5, 0, 1)),
		0);

	AddChild(dynamic_cast<BView*>(view));
	SetDefaultButton(button);

	mPassword->MakeFocus();
	Show();
}

#include "BepdfApplication.h"

bool PasswordWindow::QuitRequested() {
	if (!mPasswordSent) {
		gApp->OpenFilePanel();
	}
	return true;
}

void PasswordWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case 'OK': {
		// post message to application to open file with password
		const char *text = mPassword->Text();
		
		BMessage msg(B_REFS_RECEIVED);
		msg.AddRef("refs", &mEntry);
		msg.AddString(mPwdKind == OWNER_PASSWORD ? 
						"ownerPassword" : "userPassword", 
						text);
		mLooper->PostMessage(&msg, NULL);
		mPasswordSent = true; 
		Quit();
		break; }
	case 'user': mPwdKind = USER_PASSWORD; break;
	case 'ownr': mPwdKind = OWNER_PASSWORD; break;
	default:
		MWindow::MessageReceived(msg);
	}
}

