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
#include <unistd.h>
#include <stdlib.h>
#include <layout-all.h>
#include <kernel/scheduler.h>
#include "LayoutUtils.h"
#include "TextConversion.h"
#include "StringLocalization.h"


#include "TraceWindow.h"

TraceWindow::TraceWindow(GlobalSettings *settings) 
	: MWindow(BRect(0, 0, 100, 100), TRANSLATE("Error Messages"), 
		B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0)
	, mSettings(settings)
	, mAutoOpen(settings->GetTraceAutoOpen())
	, mShowStdout(settings->GetTraceShowStdout())
	, mShowStderr(settings->GetTraceShowStderr())
{
	
	AddCommonFilter(new EscapeMessageFilter(this, HIDE_MSG));
	
	VGroup *group = new VGroup(
		new MScrollView(mOutput = new MTextView(minimax(0, 0, 65535, 65535, 100)), false, true),
		new HGroup(
			new MCheckBox(TRANSLATE("Auto Open"), new BMessage(AUTO_OPEN_MSG), NULL, mAutoOpen),
			new MCheckBox(TRANSLATE("Floating"), new BMessage(FLOATING_MSG), NULL, mSettings->GetTraceFloating()),
			mStdoutCB = new MCheckBox(TRANSLATE("stdout"), new BMessage(SHOW_STDOUT_MSG), NULL, mShowStdout),
			mStderrCB = new MCheckBox(TRANSLATE("stderr"), new BMessage(SHOW_STDERR_MSG), NULL, mShowStderr),
			new MButton(TRANSLATE("Clear"), new BMessage(CLEAR_MSG)),
			NULL),
		NULL
	);
	mOutput->MakeEditable(false);
	mOutput->SetStylable(true);
	mStdoutCB->SetHighColor(0, 0, 255);
	mStderrCB->SetHighColor(255, 0, 0);
	EnableCheckboxes();
	AddChild(dynamic_cast<BView*>(group));

	MoveTo(settings->GetTraceWindowPosition());
	float w, h;
	settings->GetTraceWindowSize(w, h);
	ResizeTo(w, h);

	UpdateWindowLookAndFeel();

	Show(); Lock(); Hide(); Unlock();
}

void TraceWindow::EnableCheckboxes() {
	mStdoutCB->SetEnabled(mShowStdout && mShowStderr || !mShowStdout);
	mStderrCB->SetEnabled(mShowStdout && mShowStderr || !mShowStderr);
}

void TraceWindow::UpdateWindowLookAndFeel() {
	if (mSettings->GetTraceFloating()) {
		SetLook(B_FLOATING_WINDOW_LOOK); SetFeel(B_FLOATING_APP_WINDOW_FEEL); 
	} else {
		SetLook(B_TITLED_WINDOW_LOOK); SetFeel(B_NORMAL_WINDOW_FEEL); 
	}
}

void TraceWindow::FrameMoved(BPoint point) {
	mWindowPos = point;
	mSettings->SetTraceWindowPosition(point);
	MWindow::FrameMoved(point);
}

void TraceWindow::FrameResized(float w, float h) {
	mSettings->SetTraceWindowSize(w, h);
	MWindow::FrameResized(w, h);
}

bool TraceWindow::QuitRequested() {
	if (!IsHidden()) Hide();
	return false;
}

void TraceWindow::WriteData(const char* name, int fd, const char* data, int len) {
	static rgb_color stderr_color = {255, 0, 0, 255};
	static rgb_color stdout_color = {0, 0, 255, 255};

	if (len == 0) return;
	
	if (!((fd == 1 && mShowStdout) || (fd == 2 && mShowStderr))) return;

	if (mAutoOpen && IsHidden()) {
		Show();
	}

//	char buffer[256];
//	sprintf(buffer, "%s %d\n", name, fd);
//	mOutput->Insert(mOutput->TextLength(), buffer, strlen(buffer));

	int32 cur = mOutput->TextLength()-1;
	mOutput->Insert(cur + 1, data, len);
	int32 end = mOutput->TextLength();
	mOutput->SetFontAndColor(cur, end, NULL, 0, (fd == 1) ? &stdout_color : &stderr_color);
	mOutput->ScrollToOffset(end);
	mOutput->Invalidate();
	
}

void TraceWindow::MessageReceived(BMessage* msg) {
	switch(msg->what) {
	case AUTO_OPEN_MSG:  
		mAutoOpen = IsOn(msg);
		mSettings->SetTraceAutoOpen(mAutoOpen);
		break;
	case SHOW_STDOUT_MSG:  
		mShowStdout = IsOn(msg); 
		mSettings->SetTraceShowStdout(mShowStdout);
		EnableCheckboxes();
		break;
	case SHOW_STDERR_MSG:  
		mShowStderr = IsOn(msg);
		mSettings->SetTraceShowStderr(mShowStderr);
		EnableCheckboxes();
		break;
	case CLEAR_MSG: 
		mOutput->SelectAll();
		mOutput->Clear();
		break;
	case FLOATING_MSG:
		mSettings->SetTraceFloating(IsOn(msg));
		UpdateWindowLookAndFeel();
		break;
	case HIDE_MSG:
		if (!IsHidden()) Hide();
		break;
	default:
		MWindow::MessageReceived(msg);
	}
}



// Implementation of OutputTracer
TraceWindow* OutputTracer::mWindow = NULL;
BLocker      OutputTracer::mLock;
int          OutputTracer::mTracerCount = 0;

OutputTracer::OutputTracer(int fd, const char* name, GlobalSettings* settings) 
	: mDupFd(-1)
	, mOutFd(fd)
	, mInFd(-1)
	, mName(name)
	, mSettings(settings)
	, mPipeThread(-1)
{
	mName = name;
	mTracerCount ++;
	int fildes[2];
#ifdef DEBUG
	mInFd = -1; return;
#endif
	if (0 != pipe(fildes)) {
		mInFd = -1;
	} else {
		int readFd = fildes[0];
		int writeFd = fildes[1];

		mDupFd = dup(mOutFd);		
		mInFd = readFd;
		dup2(writeFd, mOutFd);
		close(writeFd);
		
		mPipeThread = spawn_thread(start_thread, name, suggest_thread_priority(B_USER_INPUT_HANDLING), this);
		resume_thread(mPipeThread);
	}
}

OutputTracer::~OutputTracer() {
	status_t status;
	close(mInFd);
	dup2(mDupFd, mOutFd); close(mDupFd); 
	wait_for_thread(mPipeThread, &status);
	mLock.Lock();
	mTracerCount --;
	if (mTracerCount == 0 && mWindow) {
		mWindow->Lock();
		mWindow->Quit();
		mWindow = NULL;
	}
	mLock.Unlock();	
}

int32 OutputTracer::start_thread(void *data) {
	((OutputTracer*)data)->Run();
	return 0;
}

TraceWindow* OutputTracer::CreateWindow(GlobalSettings* s) {
	if (mWindow == NULL) {
		mLock.Lock();
		if (mWindow == NULL) {
			mWindow = new TraceWindow(s);
		}
		mLock.Unlock();
	}
	return mWindow;
}

void OutputTracer::WriteData(const char* data, int len) {
	mLock.Lock();
	mWindow = CreateWindow(mSettings);
	if (mWindow->Lock()) {
		mWindow->WriteData(mName.String(), mOutFd, data, len);
		mWindow->Unlock();
	}
	mLock.Unlock();
}

void OutputTracer::Run() {
	const int max = 256;
	char      buffer[max];
	int       len;
	
	while(0 < (len = read(mInFd, buffer, max))) {
		WriteData(buffer, len);
	}
}

void OutputTracer::ShowWindow(GlobalSettings* s) {
	BWindow* w = CreateWindow(s);
	w->Lock();
	if (w->IsHidden()) w->Show();
	w->Activate();
	w->Unlock();
}
