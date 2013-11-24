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

#ifndef LAYOUT_UTILS_H
#define LAYOUT_UTILS_H

#include <be/app/MessageFilter.h>
#include <layout.h>
#include <MGroup.h>
#include <MTextView.h>

minimax GetFiller();
minimax GetBorder();
minimax GetSeparator();

MGroup * AlignTop(MView *view);
MGroup * AlignBottom(MView *view);
MGroup * AlignLeft(MView *view);
MGroup * AlignRight(MView *view);

#include <be/interface/ListView.h>
#include <be/interface/OutlineListView.h>

void MakeEmpty(BListView *list);
void MakeEmpty(BOutlineListView *list);

class EscapeMessageFilter : public BMessageFilter {
	private:
		BWindow *mWindow;
		int32    mWhat;
		
	public:
		EscapeMessageFilter(BWindow *window, int32 what) : BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, '_KYD'), mWindow(window), mWhat(what) { }
		filter_result Filter(BMessage *msg, BHandler **target);
};

class Bitset {
	int32 * mBitset;
	int32   mBitsetElems;
	int32   mLength;

	void Resize(int32 length);

public:
	Bitset();
	~Bitset();
	void Set(int i, bool v);
	bool IsSet(int i);
	void Clear();
};

// Message received from BCheckBox, what is its state?
bool IsOn(BMessage* msg);

// Sends message (what = CHANGED_NOTIFY, "view" pointer to TextView) to
// parent window if attached.
class TextView : public MTextView {
public:
	typedef MTextView inherited;
	TextView(minimax size = 0);
	TextView(BMessage*);
	
	enum {
		CHANGED_NOTIFY = 'TVch'
	};
	
protected:
	void InitColors();
	void Notify();
	
	void			InsertText(const char				*inText, 
								   int32					inLength, 
								   int32					inOffset,
								   const text_run_array		*inRuns);
	void			DeleteText(int32 fromOffset, int32 toOffset);
	
};


#endif
