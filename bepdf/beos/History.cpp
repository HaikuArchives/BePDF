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

#define DEBUG 1
#include <Debug.h>
#include "History.h"

HistoryPosition::HistoryPosition(HistoryFile* file, int page, int16 zoom, int32 left, int32 top, float rotation) 
	: mFile(file)
	, mPage(page)
	, mZoom(zoom)
	, mLeft(left)
	, mTop(top)
	, mRotation(rotation) 
{
	file->IncreaseUseCount();
}	

HistoryPosition::~HistoryPosition() {
	mFile->DecreaseUseCount();
}

HistoryFile::HistoryFile(entry_ref ref, const char* ownerPassword, const char* userPassword) 
	: fRef(ref)
	, fOwnerPassword(NULL)
	, fUserPassword(NULL)
	, fUseCount(0) 
{
	if (ownerPassword) {
		fOwnerPassword = new BString(ownerPassword);
	}
	if (userPassword) {
		fUserPassword = new BString(userPassword);
	}
}

HistoryFile::~HistoryFile() {
	delete fOwnerPassword; 
	delete fUserPassword;
}

History::History() {
	mCurrent = -1;
	mFile = NULL;
}

History::~History() {
	MakeEmpty();
}

void History::MakeEmpty() {
	HistoryEntry **items = (HistoryEntry**)mList.Items();
	for (int i = GetElements() - 1; i >= 0; i--) {
		delete items[i];
	}
	mList.MakeEmpty();
	mCurrent = -1;
	mFile = NULL;
}

void History::Add(HistoryEntry* e) {
	// delete to current (exlusive current)
	for (int32 i = GetElements()-1; i > mCurrent; i--) {
		delete (HistoryEntry*)mList.RemoveItem(i);
	}
	if (mCurrent == GetElements() - 1) {
		mList.AddItem(e);
		// allow max 100 items in list
		if (mList.CountItems() == 101) {
			delete (HistoryEntry*)mList.RemoveItem((int32)0);
		} else {
			mCurrent ++;
		}
	} else {
		delete e;
	}
}

void History::AddPosition(int page, int16 zoom, int32 left, int32 top, float rotation) {
	ASSERT(mFile != NULL);
	Add(new HistoryPosition(mFile, page, zoom, left, top, rotation));
}

void History::SetFile(entry_ref ref, const char* ownerPassword, const char* userPassword) {
	if (mFile != NULL && mFile->GetUseCount() == 0) {
		delete mFile;
	}
	mFile = new HistoryFile(ref, ownerPassword, userPassword);
}

HistoryEntry* History::GetTop() {
	if (mCurrent != -1) {
		HistoryEntry *item = (HistoryEntry*)mList.ItemAt(mCurrent);
		return item;
	} else {
		return NULL;
	}
}

bool History::Back() {
	if (mCurrent > 0) {
		mCurrent --;
		UpdateFile();
		return true;
	} else {
		return false;
	}
}

bool History::Forward() {
	if (mCurrent < GetElements() - 1) {
		mCurrent ++;
		UpdateFile();
		return true;
	} else {
		return false;
	}
}

void History::UpdateFile() {
	HistoryEntry* e = GetTop();
	HistoryPosition* pos = dynamic_cast<HistoryPosition*>(e);
	if (pos != NULL) {
		mFile = pos->GetFile();
	}
}
