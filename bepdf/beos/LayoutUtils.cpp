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

#include <Control.h>
#include <Window.h>

#include "LayoutUtils.h"

typedef struct {
	BListItem **items;
	int32 current;
} ListItems;

static bool CopyListItem(BListItem *item, void *data) {
	((ListItems*)data)->items[((ListItems*)data)->current++] = item;
	return false;
}

void MakeEmpty(BListView *list) {
	int32 count = list->CountItems();
	if (count == 0) return;
	
	BListItem **items = new BListItem *[count];
	const BListItem **listItems = list->Items();
	for (int32 i = count-1; i >= 0; i--) {
		items[i] = (BListItem*)listItems[i];
	}
	list->MakeEmpty();
	for (int32 i = count-1; i >= 0; i--) {
		delete items[i];
	}
}

void MakeEmpty(BOutlineListView *list) {
	int32 count = list->FullListCountItems();
	if (count == 0) return;

	ListItems data;
	data.items = new BListItem *[count];
	data.current = 0;
	list->FullListDoForEach(CopyListItem, &data);
	list->MakeEmpty();
	for (int32 i = count-1; i >= 0; i--) {
		delete data.items[i];
	}
}

// Implementation of EscapeMessageFilter
filter_result EscapeMessageFilter::Filter(BMessage *msg, BHandler **target) {
	int32 key;
	if (B_OK == msg->FindInt32("key", &key) && key == 1) {
		mWindow->PostMessage(mWhat);
		return B_SKIP_MESSAGE;
	}
	return B_DISPATCH_MESSAGE;
}

// Implementation of Bitset
#include <malloc.h>
Bitset::Bitset() {
	mBitsetElems = 1;
	mLength = 0;
	mBitset = (int32*)malloc(sizeof(int32) * mBitsetElems);
}

Bitset::~Bitset() {
	free(mBitset);
}

void Bitset::Resize(int32 i) {
	int32 elems = 1 + i / 32;

	// resize bitset	
	if (mBitsetElems < elems) {
		mBitsetElems = elems;
		mBitset = (int32*)realloc(mBitset, sizeof(int32) * mBitsetElems);
	}
	
	// initialize 
	if (i >= mLength) {
		int32 s = (mLength+31) / 32;
		mLength = i + 1;
		while (s < elems) {
			mBitset[s] = 0; s ++;
		};
	} 
}

void Bitset::Set(int i, bool v) { 
	if (i >= 0) {
		Resize(i); 
		if (v) {
			mBitset[i / 32] |= 1 << (i % 32);
		} else {
			mBitset[i / 32] &= ~(1 << (i % 32));
		}
	}
}

bool Bitset::IsSet(int i) { 
	if (i >= 0 && i < mLength) 
		return (mBitset[i / 32] & (1 << (i % 32))) != 0; 
	else
		return false;
}

void Bitset::Clear() { 
	mLength = 0; 
}

bool IsOn(BMessage *msg) {
	void *ptr;
	if (B_OK == msg->FindPointer("source", &ptr)) {
		return ((BControl*)ptr)->Value() == B_CONTROL_ON;
	}
	return false;
}

// Implementation of TextView
TextView::TextView(const char *name)
	: BTextView(name)
{
	InitColors();
}

TextView::TextView(BMessage* msg)
	: BTextView(msg)
{
	InitColors();
}

void TextView::InitColors()
{
	// default foreground color is black
	// set background color to white
	SetViewColor(255, 255, 255);
}

void TextView::Notify() {
	if (Window()) {
		BMessage msg(CHANGED_NOTIFY);
		msg.AddPointer("view", this);
		Window()->PostMessage(&msg);
	}
}

void TextView::InsertText(const char* inText, 
							int32 inLength, 
							int32 inOffset,
							const text_run_array* inRuns)
{
	inherited::InsertText(inText, inLength, inOffset, inRuns);
	Notify();
}

void TextView::DeleteText(int32 fromOffset, int32 toOffset)
{
	inherited::DeleteText(fromOffset, toOffset);
	Notify();
}


