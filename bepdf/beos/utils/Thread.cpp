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

#include "Thread.h"

Thread::Thread(const char* name, int32 priority) {
	mThreadId = spawn_thread(DoRun, name, priority, this);
}

Thread::~Thread() {
}

status_t Thread::InitCheck() {
	return mThreadId >= 0 ? B_OK : B_ERROR;
}

thread_id Thread::GetThreadId() {
	return mThreadId;
}

void Thread::SetPriority(int32 priority) {
	if (InitCheck() == B_OK) {
		set_thread_priority(mThreadId, priority);
	}
}
	
status_t Thread::Resume() {
	if (InitCheck() != B_OK) {
		delete this;
	}
	
	status_t result = resume_thread(mThreadId);
	if (result != B_OK) {
		delete this;
	}
	return result;
}

int32 Thread::DoRun(void* data) {
	Thread* thread = (Thread*)data;
	int32 result = thread->Run();
	delete thread;
	return result;
}


