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


#include "AttachmentView.h"

// BeOS
#include <locale/Catalog.h>
#include <Entry.h>
#include <ListView.h>
#include <Path.h>
#include <Region.h>
#include <ScrollView.h>
#include <LayoutBuilder.h>

#include <private/interface/ColumnListView.h>
#include <private/interface/ColumnTypes.h>

// bepdf
#include "BepdfApplication.h" // for save panel
#include "ResourceLoader.h"
#include "LayoutUtils.h"
#include "SaveThread.h"
#include "StatusWindow.h"
#include "TextConversion.h"
#include "Thread.h"

// xpdf
#include <UTF8.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AttachmentView"

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

AttachmentItem::AttachmentItem(BString fileName, int fileIdx)
	: BRow(),
	  fFileName(fileName),
	  fFileIdx(fileIdx)
{
	SetField(new BStringField(fileName.String()), 0);

	//BString description;
	//TextToUtf8(fileSpec->GetDescription(), &description);
	//SetField(new BStringField(description.String()),1);
}

const char* AttachmentItem::Text() {
	BStringField* field = static_cast<BStringField*>(GetField(0));
	return field->String();
}


AttachmentView::AttachmentView(GlobalSettings *settings, BLooper *looper, uint32 flags)
	: BView("attachments", flags | B_FRAME_EVENTS)
{
	fToolBar = new BToolBar;
	fToolBar->SetName("toolbar");
	fToolBar->SetResizingMode(B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
	fToolBar->SetFlags(B_WILL_DRAW | B_FRAME_EVENTS);

	//fToolBar->AddAction(kOpenCmd, this, LoadBitmap("OPEN_FILE_ON"),
	//	B_TRANSLATE("Open attachment(s)"));

	// Note tooltip text used in method Update() also!
	fToolBar->AddAction(kSaveAsCmd, this, LoadBitmap("SAVE_FILE_AS_ON"),
		B_TRANSLATE("Save attachment(s) as"));
	fToolBar->AddGlue();

	mList = new BColumnListView(NULL,
		B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
		B_FANCY_BORDER,
		true);
	mList->SetSelectionMode(B_MULTIPLE_SELECTION_LIST);
	mList->AddColumn(new BStringColumn(B_TRANSLATE("File name"),
		settings->GetAttachmentFileNameColumnWidth(), 10, 1000, true),0);
	mList->AddColumn(new BStringColumn(B_TRANSLATE("Description"),
		settings->GetAttachmentDescriptionColumnWidth(), 10, 1000, true),1);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0)
		.Add(fToolBar)
		.Add(mList)
	.End();
}

AttachmentView::~AttachmentView()
{
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
		info = B_TRANSLATE("Save selected attachment to file");
	} else if (selection == kMultipleAttachmentsSelected) {
		info = B_TRANSLATE("Save selected attachments into directory");
	} else {
		// Note text used in constructor also!
		info = B_TRANSLATE("Save attachment(s) as");
	}
	// mSaveButton->SetToolTip(info); // TODO/FIXME

	// Update button state
	fToolBar->SetActionEnabled(kSaveAsCmd, selection != kNoAttachmentSelected);
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
				gApp->OpenSaveFilePanel(this, NULL, &saveMsg, "");
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

void AttachmentView::Fill(XRef* xref, PDFDoc* doc)
{
	mList->Clear();
	mXRef = xref;
	Catalog* catalog = doc->getCatalog();
	fDoc = doc;
	if (catalog->getNumEmbeddedFiles() == 0) {
		// add empty item
		Empty();
	} else {
		for (int i = 0; i < catalog->getNumEmbeddedFiles(); i++) {
			BString str;
			char buf[4];
			for (int j = 0; j < catalog->getEmbeddedFileNameLength(i); j++) {
				int32 len = mapUTF8(catalog->getEmbeddedFileName(i)[j],
					(char*)&buf, 4);
				str.Append((const char*)&buf, len);
			}
			mList->AddRow(new AttachmentItem(str, i));
		}
	}
	Update();
}

void AttachmentView::Empty()
{
	mList->Clear();
	BRow* item = new BRow();
	item->SetField(new BStringField(B_TRANSLATE("<empty>")),0);
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

		msg->AddPointer("attachment", attachment);
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
	SaveAttachmentThread(const char* title, XRef* xref, PDFDoc* doc, const BMessage* message)
		: SaveThread(title, xref),
		mMessage(*message),
		fDoc(doc)
	{
	}

	void ActuallySave(BString path, int fileIdx) {
		fDoc->saveEmbeddedFile(fileIdx, path.LockBuffer(path.Length()));
		path.UnlockBuffer();
	}

	int32 Run() {
		entry_ref dir;
		int32 count;

		if (mMessage.FindInt32("count", &count) != B_OK) {
			return -1;
		}

		BPath path;
		if (count == 1) {
			BString name;
			if (mMessage.FindRef("directory", &dir) != B_OK ||
				mMessage.FindString("name", &name) != B_OK)	{
				// should not happen
				return -1;
			}
			path.SetTo(&dir);
			path.Append(name.String());
		} else {
			if (mMessage.FindRef("refs", &dir) != B_OK)	{
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
				SetText(item->GetFileName());
				ActuallySave(path.Path(), item->GetFileIndex());
			}
		} else {
			AttachmentItem* item = AttachmentView::GetAttachment(&mMessage, 0);
			for (int32 i = 1; item != NULL; i ++) {
				BString name(item->GetFileName());
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

					ActuallySave(file.Path(), item->GetFileIndex());
					break;
				}

				item = AttachmentView::GetAttachment(&mMessage, i);
			}
		}
		return 0;
	}

private:
	BMessage         mMessage;
	PDFDoc*			fDoc;
};

void AttachmentView::Save(BMessage* msg) {
	const char* title = B_TRANSLATE("Saving attachment(s):");
	SaveAttachmentThread* thread = new SaveAttachmentThread(title, mXRef, fDoc, msg);
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

