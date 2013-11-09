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

#ifndef _THREAD_H
#define _THREAD_H

#include <be/kernel/scheduler.h>

class Thread {
public:
	// Constructs a thread with the specified name and priority
	// See spawn_thread()
	Thread(const char* name, int32 priority);
	
	// Thread destructor.
	virtual ~Thread();
	
	// Returns B_OK if the thread could be created in the constructor
	status_t InitCheck();

	// Returns the thread_id.
	thread_id GetThreadId();

	// Sets the thread priority.
	void SetPriority(int32 priority);

	// Resumes the thread. The thread takes ownership of this Thread
	// and deletes it before the thread exits. You must not call
	// this method more than one time.
	status_t Resume();	
	
	// This callback method is run in the created thread.
	virtual int32 Run() = 0;

private:
	static int32 DoRun(void* data);
	
	thread_id mThreadId;
};

#endif
