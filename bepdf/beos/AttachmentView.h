/*
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 * 	 Copyright (C) 2016 Adrián Arroyo Calle.
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

#ifndef ATTACHMENT_VIEW_H
#define ATTACHMENT_VIEW_H

// BeOS
#include <be/app/Looper.h>
#include <be/interface/Font.h>
#include <be/interface/ListItem.h>
#include <be/interface/PictureButton.h>
#include <be/interface/View.h>
#include <be/support/SupportDefs.h>
#include <be/support/List.h>

#include <private/interface/ColumnListView.h>
#include <private/interface/ColumnTypes.h>

// xpdf
#include <Object.h>

// BePDF
#include "FileSpec.h"
#include "InputEnabler.h"
#include "Settings.h"
#include "ToolBar.h"
#include "ToolTip.h"

class ResourceBitmapButton;
class XRef;

// Column 0 contains file name, column 1 contains description.
class AttachmentItem : public BRow {
private:
	FileSpec mFileSpec;

	typedef BRow super;

public:
	// Constructs the AttachmentItem and the values for its columns
	AttachmentItem(FileSpec* fileSpec);

	// Returns the FileSpec.
	FileSpec* GetFileSpec() { return &mFileSpec; }

	// Returns label of column 0.
	const char* Text();
};

// AttachmentView displays attachments in a ColumnListView
class AttachmentView : public BView {
	BColumnListView* mList;
	XRef*           mXRef;
	InputEnabler    mInputEnabler;

	typedef BView super;

public:
	AttachmentView(BRect rect, GlobalSettings* settings,
		BLooper* looper, uint32 resizeMask, uint32 flags);
	~AttachmentView();
	void AttachedToWindow();
	void MessageReceived(BMessage* msg);

	// Empties the column list.
	void Empty();
	// Fills the column list with attachments.
	void Fill(XRef* xref, Object* embeddedFiles);

	// Returns the AttachmentItem at the specified index in msg.
	static AttachmentItem* GetAttachment(BMessage* msg, int32 index);

	void Update();

private:

	ResourceBitmapButton* mSaveButton;

	void Register(uint32 behavior, BControl* control, int32 cmd);
	ResourceBitmapButton* AddButton(ToolBar* toolBar,
		const char *name, const char *off, const char *on, const char *off_grey,
		const char *on_grey, int32 cmd, const char *info,
		uint32 behavior = B_ONE_STATE_BUTTON);
	ResourceBitmapButton* AddButton(ToolBar* toolBar,
		const char *name, const char *off, const char *on, int32 cmd,
		const char *info, uint32 behavior = B_ONE_STATE_BUTTON);

	// Adds selected attachments to msg
	int32 AddSelectedAttachments(BMessage* msg);
	// Saves the attachments stored in msg
	void Save(BMessage* msg);

	enum AttachmentSelection {
		kNoAttachmentSelected,
		kOneAttachmentSelected,
		kMultipleAttachmentsSelected
	};

	AttachmentSelection GetAttachmentSelection();

};

#endif
