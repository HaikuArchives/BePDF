/*  
	BeOS Front-end du PDF file reader xpdf.
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
#ifndef PASSWORD_WINDOW_H
#define PASSWORD_WINDOW_H
#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <Looper.h>
#include <Entry.h>

#include <MWindow.h>
#include <MTextView.h>

class PasswordWindow : public MWindow {
public:
	PasswordWindow(entry_ref *ref, BRect rect, BLooper *looper);
	void MessageReceived(BMessage *msg);
	bool QuitRequested();
	
protected:
	BLooper *mLooper;
	MTextView *mPassword;
	bool mPasswordSent;
	entry_ref mEntry;
	enum PwdKind {
		OWNER_PASSWORD,
		USER_PASSWORD
	};
	static enum PwdKind mPwdKind;
};
#endif
