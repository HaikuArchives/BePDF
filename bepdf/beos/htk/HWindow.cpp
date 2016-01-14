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

#include <be/app/Application.h>
#include <be/support/Locker.h>
#include "HWindow.h"
#include <be/be_apps/Deskbar/Deskbar.h>
#include <be/interface/Screen.h>

extern BScreen *gScreen;
BDeskbar my_deskbar;

BList HWindow::mWindowList;
BLocker HWindow::mWindowListLocker;

const char * CMD_IDX_LABEL = "cmd";


//////////////////////////////////////////////////////////////
HWindow::HWindow(BRect frame, const char * label, window_type type,
									ulong flags, bool quitWhenClosed, ulong workSp)
	: BWindow(frame, label, type, flags, workSp),
	mQuitWhenClosed(quitWhenClosed)
{
	mUnzoomedFrame = frame;
	if (mQuitWhenClosed) {
		if (mWindowListLocker.Lock()) {
			mWindowList.AddItem(this);
			mWindowListLocker.Unlock();
		}
	}
}


//////////////////////////////////////////////////////////////
HWindow::~HWindow()
{
	if (mQuitWhenClosed) {
		if (mWindowListLocker.Lock()) {
			mWindowList.RemoveItem(this);
			mWindowListLocker.Unlock();
		}
		if (mWindowList.CountItems() == 0) {
			be_app->PostMessage(B_QUIT_REQUESTED);
		}
	}
}

void HWindow::UnzoomCheck(BRect f) {
	if (f == Frame()) {
		MoveTo(BPoint(mUnzoomedFrame.left, mUnzoomedFrame.top));
		ResizeTo(mUnzoomedFrame.Width(), mUnzoomedFrame.Height());
	} else {
		mUnzoomedFrame = f;
	}
}

void HWindow::Zoom(BPoint origin, float width, float height) {
	BRect df = my_deskbar.Frame(), f = Frame();
	const int window_border_size = 4;
	const int window_tab_height = 19;
	switch (my_deskbar.Location()) {
		case B_DESKBAR_TOP:
			df.bottom += window_tab_height + window_border_size;
			if (origin.y <= df.bottom) {
				float dy = df.bottom - origin.y + 1;
				origin.y += dy; height -= dy;
				MoveTo(origin); ResizeTo(width, height);
				UnzoomCheck(f);
				return;
			}
			break;
		case B_DESKBAR_BOTTOM:
			if (origin.y + height + window_border_size > df.top) {
				height -= origin.y + height + window_border_size - df.top + 1;
				MoveTo(origin); ResizeTo(width, height);
				UnzoomCheck(f);
				return;
			}
			break;
		default:;
	}
	inherited::Zoom(origin, width, height);
}


// #pragma mark -

//////////////////////////////////////////////////////////////
void HWindow::MessageReceived(BMessage * msg)
{
	status_t err;
	int32 cmd;

	switch(msg->what) {
	case STANDARD_CMD_MSG:
		err = msg->FindInt32(CMD_IDX_LABEL, &cmd);
		if (err == B_NO_ERROR) {
			HandleCommand(cmd, msg);
		}
		break;
	default:
		inherited::MessageReceived(msg);
		break;
	}
}



// #pragma mark -


//////////////////////////////////////////////////////////////
BMessage * HWindow::MakeCommandMessage(ulong cmd)
{
	BMessage * aMsg = new BMessage(STANDARD_CMD_MSG);
	aMsg->AddInt32(CMD_IDX_LABEL, cmd);

	return aMsg;
}


//////////////////////////////////////////////////////////////

