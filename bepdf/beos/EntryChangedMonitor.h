/*  
	Copyright (C) 2004 Michael Pfeiffer
	
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

#ifndef ENTRY_CHANGED_MONITOR_H
#define ENTRY_CHANGED_MONITOR_H

#include <Handler.h>
#include <Node.h>
#include <Entry.h>
#include <NodeMonitor.h>

class EntryChangedListener {
public:
	virtual void EntryChanged() = 0;
};

class EntryChangedMonitor : public BHandler {
public:
	// Note: After construction the EntryChangedMonitor has to be
	// added to a BLooper.
	EntryChangedMonitor();
	// And before destruction the EntryChangedMonitor has to be
	// removed from the BLooper.
	virtual ~EntryChangedMonitor();
	
	// Set the listener that is called when the entry changes.
	void SetEntryChangedListener(EntryChangedListener* listener);

	// Set the node referenced by the entry to be watched. 
	// Automatically stops watching of a previously set node.
	void StartWatching(entry_ref *ref);
	// Stop watching of a previously set entry.
	void StopWatching();

	void MessageReceived(BMessage* msg);

private:	
	void NotifyListener();
	
	EntryChangedListener* mListener;
	bool       mActive;
	entry_ref  mEntryRef;
	node_ref   mNodeRef;
};

#endif