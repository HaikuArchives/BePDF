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

#include "EntryChangedMonitor.h"

EntryChangedMonitor::EntryChangedMonitor() 
	: mListener(NULL)
	, mActive(false)
{
}

EntryChangedMonitor::~EntryChangedMonitor() {
	StopWatching();
}

void EntryChangedMonitor::SetEntryChangedListener(EntryChangedListener* listener) {
	mListener = listener;
}

void EntryChangedMonitor::StartWatching(entry_ref *ref) {
	// allow to watch a single file only!
	StopWatching();

	mEntryRef = *ref;
	mActive = true;
	BNode node(ref);
	if (node.InitCheck() == B_OK &&
		node.GetNodeRef(&mNodeRef) == B_OK) {
		watch_node(&mNodeRef, B_WATCH_STAT, this);
	}
}

void EntryChangedMonitor::StopWatching() {
	if (mActive) {
		watch_node(&mNodeRef, B_STOP_WATCHING, this);
		mActive = false;		
	}
}

void EntryChangedMonitor::MessageReceived(BMessage* msg) {
	if (msg->what != B_NODE_MONITOR)
		return;
		
	int32 opcode;		
	if (msg->FindInt32("opcode", &opcode) != B_OK)
		return;
			
	if (opcode != B_STAT_CHANGED)
		return;
	
	#ifdef __HAIKU__
	// Haiku sends a B_STAT_CHANGED notification 
	// when attributes are changed too
	// this leads to an infinite loop, as
	// BePDF changes file attributes after
	// loading a file.
	// TODO check if file has changed and only
	// then notify the listener
	#else	
	// This seems to be a good indicator that the
	// contents of a file has changed.
	NotifyListener();
	#endif
	
}

void EntryChangedMonitor::NotifyListener() {
	if (mListener != NULL) {
		mListener->EntryChanged();
	}
}

