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

#ifndef _H_WINDOW_H_
#define _H_WINDOW_H_

#include <be/app/AppDefs.h>
#include <be/interface/Window.h>
#include <be/support/Locker.h>

#define ADD_ITEM(menu, name, shortcut, msg)                      \
	{                                                            \
		BMenuItem* item = new BMenuItem(name, msg, shortcut);    \
		(menu)->AddItem(item);                                   \
		mInputEnabler.Register(new IEMenuItem(item));            \
	}

#define ADD_SITEM(menu) \
		{ (menu)->AddItem(new BSeparatorItem); }

#define STANDARD_CMD_MSG		'SCmd'

extern const char * CMD_IDX_LABEL;

class HWindow
	: public BWindow
{
	typedef BWindow inherited;
	
public:
		HWindow(BRect frame, const char * label, window_type type,
						ulong flags, bool quitWhenClosed = true,
						ulong workSp = B_CURRENT_WORKSPACE);
	virtual ~HWindow();

	void AskQuit()           { PostMessage(B_QUIT_REQUESTED); };
	void Zoom(BPoint origin, float width, float height);

	virtual void MessageReceived(BMessage *msg);
	virtual bool CanClose() = 0;

	static int32 GetWindowCount()
			{ return mWindowList.CountItems(); };

	static BMessage * MakeCommandMessage(uint32 cmd);
	 
protected:
	virtual void HandleCommand(int32 cmd, BMessage *msg) = 0;

private:
	bool           mQuitWhenClosed;
	static BList   mWindowList;
	static BLocker mWindowListLocker; 
	BRect          mUnzoomedFrame;
	
	void UnzoomCheck(BRect f);
};


#endif
