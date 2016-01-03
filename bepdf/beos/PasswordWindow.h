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

#ifndef PASSWORD_WINDOW_H
#define PASSWORD_WINDOW_H

#include <Entry.h>
#include <Looper.h>
#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <Window.h>

class BTextControl;

class PasswordWindow : public BWindow {
public:
	PasswordWindow(entry_ref *ref, BRect rect, BLooper *looper);
	void MessageReceived(BMessage *msg);
	bool QuitRequested();
	
protected:
	BLooper *mLooper;
	BTextControl *mPassword;
	bool mPasswordSent;
	entry_ref mEntry;
	enum PwdKind {
		OWNER_PASSWORD,
		USER_PASSWORD
	};
	static enum PwdKind mPwdKind;
};
#endif
