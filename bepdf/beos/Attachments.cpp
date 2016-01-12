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

#include <be/interface/ListItem.h>

#include "Attachments.h"
#include "AttachmentView.h" // for AttachmentItem
#include "FileSpec.h"
#include "LayoutUtils.h"
#include "TextConversion.h"

Attachments::~Attachments() {
	AttachmentItem **items = (AttachmentItem**)mAttachments.Items();
	for (int i = mAttachments.CountItems()-1; i >= 0; i--) {
		delete items[i];
	}
	mAttachments.MakeEmpty();	
}

bool Attachments::DoName(const char* name, Object* value) {
	if (!value->isDict()) {
		// do know how to handle attachment -> skip it
		return true; 
	}
	FileSpec fileSpec(value->getDict());
	if (!fileSpec.IsValid()) {
		// not or an invalid file spec -> skip it
		return true; 
	}
	mAttachments.AddItem(new AttachmentItem(&fileSpec));
	
	return true;
}

static int sortByName(const void* a, const void* b) {
	AttachmentItem** pa = (AttachmentItem**)a;
	AttachmentItem** pb = (AttachmentItem**)b;
	const char* sa = (*pa)->Text();
	const char* sb = (*pb)->Text();
	return strcmp(sa, sb);  
}

void Attachments::Replace(BColumnListView *view) {
	view->Clear();
	mAttachments.SortItems(sortByName);
	for(int i=0;i<mAttachments.CountItems();i++) {
		view->AddRow((AttachmentItem*)mAttachments.ItemAt(i));
	}
	mAttachments.MakeEmpty();
}
