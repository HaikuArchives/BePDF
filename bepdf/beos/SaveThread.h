/*  
	Copyright (C) 2005 Michael Pfeiffer
	
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

#ifndef _SAVE_THREAD_H
#define _SAVE_THREAD_H

#include "StatusWindow.h"
#include "Thread.h"

class XRef;

// Save thread with status window
class SaveThread : public Thread {
public:
	SaveThread(const char* statusText, XRef* xref) 
		: Thread("save", B_LOW_PRIORITY)
		, mXRef(xref)
		, mShowStatusWindow(statusText)
	{
		SetPriority(suggest_thread_priority(B_OFFLINE_PROCESSING));
	}
	
	XRef* GetXRef();

	void SetTotal(int32 total);
	void SetCurrent(int32 current);
	void SetText(const char* text);

private:
	XRef*            mXRef;
	ShowStatusWindow mShowStatusWindow;
};


#endif

