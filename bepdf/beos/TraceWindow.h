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
#ifndef _TRACE_WINDOW_H
#define _TRACE_WINDOW_H

#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <Looper.h>
#include <Locker.h>

#include <MWindow.h>
#include <MButton.h>
#include <MTextView.h>
#include <MStringView.h>
#include <MCheckBox.h>

#include "Settings.h"

class TraceWindow : public MWindow {
public:
	TraceWindow(GlobalSettings *settings);
	void MessageReceived(BMessage *msg);

	virtual void FrameMoved(BPoint point);
	virtual void FrameResized(float w, float h);
	bool QuitRequested();
	void WriteData(const char* name, int fd, const char* data, int len);
	void EnableCheckboxes();
	void UpdateWindowLookAndFeel();
	
protected:
	enum {
		AUTO_OPEN_MSG   = 'Auto',
		SHOW_STDOUT_MSG = 'Sout',
		SHOW_STDERR_MSG = 'Serr',
		CLEAR_MSG       = 'Clr ',
		FLOATING_MSG    = 'Flot',
		HIDE_MSG        = 'Hide'
	};

	GlobalSettings *mSettings;
	BPoint          mWindowPos;
	MTextView      *mOutput;
	MCheckBox      *mStdoutCB, 
				   *mStderrCB;

	bool            mAutoOpen;
	bool            mShowStdout, mShowStderr;
};

class OutputTracer {
private:
	static TraceWindow* mWindow;
	static BLocker      mLock;
	static int          mTracerCount;
	
	int                 mDupFd;
	int                 mOutFd;
	int                 mInFd;
	BString             mName;
	GlobalSettings*     mSettings;
	thread_id           mPipeThread;
	
	void WriteData(const char* data, int len);
	void Run();
	static int32 start_thread(void *data);
	static TraceWindow* CreateWindow(GlobalSettings* s);
	
public:
	OutputTracer(int fd, const char* name, GlobalSettings *settings);
	~OutputTracer();
	static void ShowWindow(GlobalSettings* s);
};

#endif
