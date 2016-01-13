/*  
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 *   Copyright (C) 2016 Adrián Arroyo Calle.
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


#include "AttachmentView.h"

// BeOS
#include <Entry.h>
#include <ListView.h>
#include <Path.h>
#include <Region.h>
#include <ScrollView.h>

#include <private/interface/ColumnListView.h>
#include <private/interface/ColumnTypes.h>

// bepdf
#include "Attachments.h"
#include "BitmapButton.h"
#include "BepdfApplication.h" // for save panel
#include "LayoutUtils.h"
#include "SaveThread.h"
#include "StatusWindow.h"
#include "StringLocalization.h"
#include "TextConversion.h"
#include "Thread.h"
#include "ToolBar.h"

static const int kToolbarHeight = 30;

enum {
	kOpenCmd,
	kSaveAsCmd,
	kSelectionChangedCmd,
};

enum {
	kAdditionalVerticalBorder = 2
};	

// Implementation of AttachmentItem

AttachmentItem::AttachmentItem(FileSpec* fileSpec) 
	: BRow()
	, mFileSpec(fileSpec)
{
	BString fileName;
	TextToUtf8(fileSpec->GetFileName(), &fileName);
	SetField(new BStringField(fileName.String()),0);
	
	BString description;
	TextToUtf8(fileSpec->GetDescription(), &description);
	SetField(new BStringField(description.String()),1);
}

const char* AttachmentItem::Text() {
	BStringField* field = GetField(0);
	return field->String();
}

// AttachmentListView
class AttachmentListView : public BColumnListView {
public:
		AttachmentListView(GlobalSettings* settings, 
						BRect frame,
						const char* name = NULL,
						uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
						border_style border = B_NO_BORDER,
						bool horizontal = true) 
			: BColumnListView(frame, name, resizingMode, flags, border, horizontal)
			, mSettings(settings)
		{
			SetSelectionMode(B_MULTIPLE_SELECTION_LIST);
		}
		
private:
	GlobalSettings* mSettings;
};

// Implementation of AttachmentView

void AttachmentView::Register(uint32 behavior, BControl* control, int32 cmd) {
	if (behavior == B_ONE_STATE_BUTTON) {
		mInputEnabler.Register(new IEControl(control, cmd));
	} else {
		// behavior == B_TWO_STATE_BUTTON
		// mControlValueSetter.Register(new IEControlValue(control, cmd));
	}
}

ResourceBitmapButton* 
AttachmentView::AddButton(::ToolTip* tooltip, ToolBar* toolBar, const char *name, const char *off, const char *on, const char *off_grey, const char *on_grey, int32 cmd, const char *info, uint32 behavior) {
	const int buttonSize = kToolbarHeight; 
	ResourceBitmapButton *button = new ResourceBitmapButton (BRect (0, 0, buttonSize, buttonSize), 
	                                name, off, on, off_grey, on_grey,
	                               new BMessage(cmd),
	                                behavior); 
	button->SetToolTip(tooltip, TRANSLATE(info)); 
	toolBar->Add (button);
	Register(behavior, button, cmd);
	return button;
}

///////////////////////////////////////////////////////////
ResourceBitmapButton* 
AttachmentView::AddButton(::ToolTip* tooltip, ToolBar* toolBar, const char *name, const char *off, const char *on, int32 cmd, const char *info, uint32 behavior) {
	const int buttonSize = kToolbarHeight;
	ResourceBitmapButton *button = new ResourceBitmapButton (BRect (0, 0, buttonSize, buttonSize), 
	                                name, off, on, 
	                                new BMessage(cmd),
	                                behavior); 
	button->SetToolTip(tooltip, TRANSLATE(info)); 
	toolBar->Add (button);
	Register(behavior, button, cmd);
	return button;
}

AttachmentView::AttachmentView(::ToolTip* tooltip, BRect rect, GlobalSettings *settings, BLooper *looper, uint32 resizeMask, uint32 flags) 
	: BView(rect, "attachments", resizeMask, flags | B_FRAME_EVENTS)
{
	rect.OffsetTo(0, 0);

	BRect r(rect);	
	r.bottom = 30;
	ToolBar* toolbar = new ToolBar(r, "toolbar", 
								B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT, 
								B_WILL_DRAW | B_FRAME_EVENTS );
	AddChild(toolbar);
    
    // AddButton(tooltip, toolbar, "open_btn", "OPEN_FILE_OFF", "OPEN_FILE_ON", "OPEN_FILE_OFF_GREYED",  NULL, kOpenCmd, "Open attachment(s).");
	// Note tooltip text used in method Update() also!
    mSaveButton = AddButton(tooltip, toolbar, "save_file_as_btn", "SAVE_FILE_AS_OFF", "SAVE_FILE_AS_ON", "SAVE_FILE_AS_OFF_GREYED", NULL, kSaveAsCmd, "Save attachment(s) as.");
	
	rect.top += r.bottom + 1;
	r = rect;
	r.InsetBy(2, 2);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= B_H_SCROLL_BAR_HEIGHT;
	
	mList = new AttachmentListView(settings,
		r, NULL, 
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
		B_FANCY_BORDER,
		true);
	mList->AddColumn(new BStringColumn(TRANSLATE("File Name"), settings->GetAttachmentFileNameColumnWidth(), 10, 1000, true),0);
	mList->AddColumn(new BStringColumn(TRANSLATE("Description"), settings->GetAttachmentDescriptionColumnWidth(), 10, 1000, true),1);
}

AttachmentView::~AttachmentView() {
	mList->Clear();
}

void AttachmentView::AttachedToWindow() {
	super::AttachedToWindow();

	BView* toolbar = FindView("toolbar");
	for (int32 i = 0; i < toolbar->CountChildren(); i ++) {
		BView* view = toolbar->ChildAt(i);
		BControl* control = dynamic_cast<BControl*>(view);
		if (control != NULL) {
			control->SetTarget(this);
		}
	}
	
	mList->SetSelectionMessage(new BMessage(kSelectionChangedCmd));
	mList->SetTarget(this);
}

void AttachmentView::Update() {
	AttachmentSelection selection = GetAttachmentSelection();

	// Update tool tip text
	const char* info = NULL;
	if (selection == kOneAttachmentSelected) {
		info = TRANSLATE("Save selected attachment to file.");
	} else if (selection == kMultipleAttachmentsSelected) {
		info = TRANSLATE("Save selected attachments into directory.");
	} else {
		// Note text used in constructor also!
		info = TRANSLATE("Save attachment(s) as.");
	}
	mSaveButton->GetToolTipItem()->SetLabel(info);

	// Update button state
	mInputEnabler.SetEnabled(kSaveAsCmd, selection != kNoAttachmentSelected);
	mInputEnabler.Update();
}

void AttachmentView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case kSelectionChangedCmd:
		Update();
		break;
	case kSaveAsCmd:
		{
			BMessage saveMsg(B_SAVE_REQUESTED);
			int32 count = AddSelectedAttachments(&saveMsg);
			
			if (count == 0) {
				break;
			}
			
			if (count == 1) {
				AttachmentItem* item = GetAttachment(&saveMsg, 0);
				gApp->OpenSaveFilePanel(this, NULL, &saveMsg, 
					item->GetFileSpec()->GetFileName()->getCString());
			} else {
				gApp->OpenSaveToDirectoryFilePanel(this, NULL, &saveMsg);
			}
		}
		break;
	case kOpenCmd:
		// TODO open attachment
		break;
	case B_SAVE_REQUESTED:
		Save(msg);
		break;
	default:
		super::MessageReceived(msg);
	}
}

void AttachmentView::Fill(XRef* xref, Object *embeddedFiles) {
	mXRef = xref;
	Attachments attachments;
	if (attachments.Parse(embeddedFiles)) {
		attachments.Replace(mList);
	}
	if (mList->CountRows() == 0) {
		// add empty item
		Empty();
	}
	Update();
}

void AttachmentView::Empty() {
	mList->Clear();
	BRow* item = new BRow();
	item->SetField(new BStringField(TRANSLATE("<empty>")),0);
	mList->AddRow(item);
}

int32 AttachmentView::AddSelectedAttachments(BMessage* msg) {
	int32 count = 0;
	for (int32 i = 0; i < mList->CountRows(); i ++) {
		if (!mList->RowAt(i)->IsSelected()) {
			continue;
		}
		
		BRow* item = mList->RowAt(i);
		AttachmentItem* attachment = dynamic_cast<AttachmentItem*>(item);
		if (attachment == NULL) {
			continue;
		}
		
		msg->AddPointer("attachment",  attachment);
		count ++;
	}
	msg->AddInt32("count", count);
	return count;
}

AttachmentItem* AttachmentView::GetAttachment(BMessage* msg, int32 index) {
	void* pointer;
	if (msg->FindPointer("attachment", index, &pointer) != B_OK) {
		return NULL;
	}
	
	return (AttachmentItem*)pointer;
}

class SaveAttachmentThread : public SaveThread {
public:
	SaveAttachmentThread(const char* title, XRef* xref, const BMessage* message) 
		: SaveThread(title, xref)
		, mMessage(*message)
	{
	}

	int32 Run() {
		entry_ref dir;
		int32 count;
		
		if (mMessage.FindInt32("count", &count) != B_OK) {
			return -1;
		}
		
		BPath  path;
		if (count == 1) {
			BString name;
			if (mMessage.FindRef("directory", &dir) != B_OK ||
				mMessage.FindString("name", &name) != B_OK)
			{
				// should not happen
				return -1;
			}
			path.SetTo(&dir);
			path.Append(name.String());
		} else {
			if (mMessage.FindRef("refs", &dir) != B_OK)
			{
				// should not happen
				return -1;
			}
			path.SetTo(&dir);
		}
		SetTotal(count);
				
		// TODO validate item
		if (count == 1) {
			SetCurrent(1);
			AttachmentItem* item = AttachmentView::GetAttachment(&mMessage, 0);
			if (item != NULL) {
				SetText(item->GetFileSpec()->GetFileName()->getCString());	
				item->GetFileSpec()->Save(GetXRef(), path.Path());
			}
		} else {
			AttachmentItem* item = AttachmentView::GetAttachment(&mMessage, 0);
			for (int32 i = 1; item != NULL; i ++) {
				BString name(item->GetFileSpec()->GetFileName()->getCString());
				SetText(name.String());	
				SetCurrent(i);
				for (int postfix = 1; postfix < 50; postfix ++) {
					BPath file(path);
					if (postfix == 1) {
						file.Append(name.String());
					} else {
						BString newName(name);
						newName << " " << postfix;
						file.Append(newName.String());
					}
					
					BEntry entry(file.Path());
					if (entry.Exists()) {
						continue;
					}
					
					item->GetFileSpec()->Save(GetXRef(), file.Path());
					break;
				}
				
				item = AttachmentView::GetAttachment(&mMessage, i);
			}
		}
		return 0;
	}

private:
	BMessage         mMessage;
};

void AttachmentView::Save(BMessage* msg) {
	const char* title = TRANSLATE("Saving attachment(s):");
	SaveAttachmentThread* thread = new SaveAttachmentThread(title, mXRef, msg);
	thread->Resume();
}

AttachmentView::AttachmentSelection AttachmentView::GetAttachmentSelection() 
{
	AttachmentSelection selection = kNoAttachmentSelected;
	for (int32 index = 0; index < mList->CountRows(); index ++) {
		if (!mList->RowAt(index)->IsSelected()) {
			continue;
		}
		
		BRow* item = mList->RowAt(index);
		AttachmentItem* attachment = dynamic_cast<AttachmentItem*>(item);
		if (attachment == NULL) {
			continue;
		}

		if (selection == kOneAttachmentSelected) {
			return kMultipleAttachmentsSelected;
		} else {
			selection = kOneAttachmentSelected;
		}
	}
	return selection;
}

