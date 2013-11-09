/*  
	BeOS Front-end to PDF file reader xpdf.
 	Copyright (C) 2003 Michael Pfeiffer

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

#include <Node.h>
#include <NodeInfo.h>
#include <String.h>

#include "EntryMenuItem.h"

EntryMenuItem::EntryMenuItem(entry_ref* ref, const char* label, BMessage* message, char shortcut, uint32 modifiers)
	: BMenuItem(label, message, shortcut, modifiers)
	, fEntry(*ref)
	, fSmallIcon(NULL)
{
	BNode node(ref);
	BString title;
	if (node.InitCheck() == B_OK) {
		if (node.ReadAttrString("pdf:title", &title) == B_OK && title.Length() > 0) {
			title << " (" << label << ")";
			SetLabel(title.String());
		}
	}
}

EntryMenuItem::~EntryMenuItem()
{
	delete fSmallIcon;
}

void 
EntryMenuItem::GetContentSize(float* width, float* height)
{
	BMenuItem::GetContentSize(width, height);
	*width += kTextIndent;
	if (*height < kIconSize) {
		*height = kIconSize;
	}
}

void
EntryMenuItem::DrawContent()
{
	BView* view = Menu();
	BPoint pos(view->PenLocation());

	if (fSmallIcon == NULL) {
		fSmallIcon = LoadIcon(); // load on demand
	}

	view->MovePenBy(kTextIndent, 0);
	BMenuItem::DrawContent();
	
	if (fSmallIcon) {
		view->SetDrawingMode(B_OP_OVER);
		view->DrawBitmap(fSmallIcon, pos);
	}
}

BBitmap*
EntryMenuItem::GetIcon(BMimeType* mimeType)
{
	BBitmap* icon;
	icon = new BBitmap(BRect(0, 0, kIconSize-1, kIconSize-1), B_CMAP8);
	if (mimeType->GetIcon(icon, B_MINI_ICON) != B_OK) {
		delete icon; icon = NULL;
	}
	return icon;
}

BBitmap*
EntryMenuItem::LoadIcon()
{
	BBitmap* icon = NULL;
	BNode node(&fEntry);
	BNodeInfo info(&node);
	char type[B_MIME_TYPE_LENGTH+1];

	// Note: BNodeInfo::GetTrackerIcon does not work as expected!

	// try to get the icon stored in file attribute
	icon = new BBitmap(BRect(0, 0, kIconSize-1, kIconSize-1), B_CMAP8);
	if (info.GetIcon(icon, B_MINI_ICON) == B_OK) {
		return icon;
	} 
	delete icon; icon = NULL;

	// get the icon from type
	if (info.GetType(type) == B_OK) {
		BMimeType mimeType(type);
		BMimeType superType;
		if (mimeType.InitCheck() == B_OK) {
			icon = GetIcon(&mimeType);
			// or super type
			if (icon == NULL && mimeType.GetSupertype(&superType) == B_OK) {
				icon = GetIcon(&superType);
			}
		}
	}

	return icon;
}
