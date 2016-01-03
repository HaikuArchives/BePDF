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

#ifndef STATUS_WINDOW_H
#define STATUS_WINDOW_H

#include <Entry.h>
#include <Messenger.h>
#include <Rect.h>
#include <String.h>
#include <Window.h>

class BStringView;
class BStatusBar;

class StatusWindow : public BWindow {
	BStringView*  mText;
	BStatusBar* mStatus;
	int           mTotal;
	
public:
	enum {                         // BMessage attributes:
		TOTAL_NOTIFY = 'npgs',     // int32 "pages"
		CURRENT_NOTIFY = 'ldpg',   // int32 "page"
		TEXT_NOTIFY  = 'text'      // char* "text"
	};


	StatusWindow(const char* text, BRect rect);
	void MessageReceived(BMessage* msg);

	static void SetTotal(BMessenger* msgr, int32 total);
	static void SetCurrent(BMessenger* msgr, int32 current);
	static void SetText(BMessenger* msgr, const char* text);	
};

class ShowStatusWindow {
public:
	ShowStatusWindow(const char* name);
	virtual ~ShowStatusWindow();
	
	BMessenger* GetMessenger() { return &mMessenger; }
	
private:
	BMessenger mMessenger;
};

class ShowLoadProgressStatusWindow : public ShowStatusWindow {
public:
	ShowLoadProgressStatusWindow(const char* name);
	~ShowLoadProgressStatusWindow();
};

#endif
