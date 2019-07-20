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
// BeOS
#include <locale/Catalog.h>
#include <Button.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <ListItem.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <Window.h>
// xpdf
#include <Link.h>
#include <Object.h>
// BePDF
#include "BePDF.h"
#include "LayoutUtils.h"
#include "TextConversion.h"
#include "OutlinesWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "OutlinesWindow"

// Implementation of OutlineStyle

OutlineStyle::OutlineStyle(const BFont* font, rgb_color color)
: mFont(font)
, mColor(color)
{
}

// Implementation of OutlineStyle

OutlineStyleList::OutlineStyleList() {
	mFonts[PLAIN_STYLE] = *be_plain_font;
	mFonts[BOLD_STYLE] = *be_plain_font;
	mFonts[BOLD_STYLE].SetFace(B_BOLD_FACE);
	mFonts[ITALIC_STYLE] = *be_plain_font;
	mFonts[ITALIC_STYLE].SetFace(B_ITALIC_FACE);
	mFonts[BOLD_ITALIC_STYLE] = *be_plain_font;
	mFonts[BOLD_ITALIC_STYLE].SetFace(B_BOLD_FACE | B_ITALIC_FACE);
}

OutlineStyleList::~OutlineStyleList() {
	const int32 n = mList.CountItems();
	for (int32 i = 0; i < n; i ++) {
		OutlineStyle* style = (OutlineStyle*)mList.ItemAt(i);
		delete style;
	}
	mList.MakeEmpty();
}

const BFont* OutlineStyleList::GetFont(int style) const {
	return &mFonts[style];
}

OutlineStyle* OutlineStyleList::GetStyle(int style, rgb_color color) {
	const int32 n = mList.CountItems();
	for (int32 i = 0; i < n; i ++) {
		OutlineStyle* os = (OutlineStyle*)mList.ItemAt(i);
		if (os->GetFont() == GetFont(style) && memcmp(os->GetColor(), &color, sizeof(color)) == 0) {
			return os;
		}
	}
	OutlineStyle* os = new OutlineStyle(GetFont(style), color);
	mList.AddItem(os);
	return os;
}

OutlineStyle* OutlineStyleList::GetDefaultStyle() {
	rgb_color black={0, 0, 0, 0};
	return GetStyle(PLAIN_STYLE, black);
}

// Implementation of OutlineListItem

OutlineListItem::OutlineListItem(const char *string, uint32 level, bool expanded, OutlineStyle* style) :
	BListItem(level, expanded),
	mString(string),
	mType(linkUndefined),
	mStyle(style)
{
}

OutlineListItem::~OutlineListItem() {
	if (mType == linkDest) delete mLink.dest;
	else if (mType == linkString) delete mLink.string;
}


void OutlineListItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	rgb_color color;

	owner->PushState();
	// select background color
	if (IsSelected()) {
		color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	} else {
		color = ui_color(B_LIST_BACKGROUND_COLOR);
	}
	// fill background
	owner->SetHighColor(color);
	owner->FillRect(frame);
	// set font color
	if (IsEnabled()) {
		owner->SetHighColor(*mStyle->GetColor());
	} else {
		owner->SetHighColor(*mStyle->GetColor());
	}
	// set background color
	owner->SetLowColor(color);
	// display text
	owner->MovePenTo(frame.left+4, frame.bottom-2);
	owner->SetFont(mStyle->GetFont());
	owner->DrawString(mString.String());

	owner->PopState();
}

void OutlineListItem::SetLink(LinkDest *dest) {
	if (dest->isOk() && (mType == linkUndefined)) {
		mType = linkDest;
		mLink.dest = dest;
	} else {
		delete dest;
	}
};

void OutlineListItem::SetLink(GString *s) {
	if (mType == linkUndefined) {
		mType = linkString;
		mLink.string = s;
	}
}

void OutlineListItem::SetPageNum(int pageNum) {
	if (mType == linkUndefined) {
		mType = linkPageNum;
		mLink.pageNum = pageNum;
	}
}

// Implementation of OutlinesView
void OutlinesView::ReadOutlines(Object *o, uint32 level) {
	Object *current = new Object();
	o->copy(current);
	Object title;
	Object child;
	bool loop;
	do {
		if (current->dictLookup("Title", &title) && !title.isNull()) {
			bool open = true;
			Object count;
			if (current->dictLookup("Count", &count) && count.isInt()) {
				open = count.getInt() > 0;
			}
			count.free();

			OutlineListItem *item;
			if (title.isString()) {
				BString *s = TextToUtf8(title.getString()->getCString(), title.getString()->getLength());
				if (s && s->Length() > 0) {
					// end string at first newline character
					char *str = s->LockBuffer(s->Length());
					char *newline = strchr(str, '\n');
					if (newline) *newline = 0;
					s->UnlockBuffer();

					item = new OutlineListItem(s->String(), level, open, GetDefaultStyle());
					delete s;
				} else {
					item = new OutlineListItem(B_TRANSLATE("No title"), level, open, GetDefaultStyle());
				}
			} else {
				item = new OutlineListItem(B_TRANSLATE("No title"), level, open, GetDefaultStyle());
			}
			mList->AddItem(item);

			Object dest;
			if (current->dictLookup("Dest", &dest)) {
				if (dest.isName()) {
					item->SetLink(new GString(dest.getName()));
				} else if (dest.isArray()) {
					item->SetLink(new LinkDest(dest.getArray()));
				} else if (dest.isString()) {
					item->SetLink(dest.getString()->copy());
				}
			}
			dest.free();

			Object dict;
			if (current->dictLookup("A", &dict) && dict.isDict()) {
				Object s;
				dict.dictLookup("S", &s);
				// GoTo action
				if (s.isName("GoTo")) {
					dict.dictLookup("D", &dest);
					if (dest.isName()) {
						item->SetLink(new GString(dest.getName()));
					} else if (dest.isArray()) {
						item->SetLink(new LinkDest(dest.getArray()));
					} else if (dest.isString()) {
						item->SetLink(dest.getString()->copy());
					}
					dest.free();
				}
				s.free();
			}
			dict.free();

			// PDF 1.4
			rgb_color item_color = {0, 0, 0, 0};
			Object color;
			if (current->dictLookup("C", &color) && color.isArray() && color.arrayGetLength() == 3) {
				Object c;
				rgb_color rgb;
				if (color.arrayGet(0, &c) && c.isReal()) {
					rgb.red = (int)(255*c.getReal());
					c.free();
					if (color.arrayGet(1, &c) && c.isReal()) {
						rgb.green = (int)(255*c.getReal());
						c.free();
						if (color.arrayGet(2, &c) && c.isReal()) {
							rgb.blue = (int)(255*c.getReal());
							// set font color
							item_color = rgb;
						}
					}
				}
				c.free();
			}
			color.free();

			Object style;
			int item_style = OutlineStyleList::PLAIN_STYLE;
			if (current->dictLookup("F", &style) && style.isInt()) {
				int s = style.getInt();
				bool bold = (s & 1) != 0;
				bool italic = (s & 2) != 0;;
				// set font style
				if (bold) item_style |= OutlineStyleList::BOLD_STYLE;
				if (italic) item_style |= OutlineStyleList::ITALIC_STYLE;
			}
			style.free();

			item->SetStyle(mOutlineStyleList.GetStyle(item_style, item_color));
/*
			Object aa;
			if (current->dictLookup("AA", &aa) && !aa.isNull()) {
				fprintf(stderr, " <AA>\n");
			}
			aa.free();

			Object se;
			if (current->dictLookup("SE", &se) && !se.isNull()) {
				fprintf(stderr, " <SE>\n");
			}
			se.free();
*/
			// traverse child
			if (current->dictLookup("First", &child) && child.isDict() && !child.isNull()) {
					ReadOutlines(&child, level + 1);
			}
			child.free();

			// expanded argument of OutlineListItem constructor does not work!
			if (open) mList->Expand(item); else mList->Collapse(item);
		}
		title.free();


		Object *next = new Object();
		if (current->dictLookup("Next", next) && next->isDict() && !next->isNull()) {
			current->free();
			delete current;
			current = next;
			loop = true;
		} else {
			loop = false;
			delete next;
		}
	} while (loop);
	current->free();
	delete current;
}

OutlinesView::OutlinesView(Catalog *catalog, BMessage *bookmarks,
	GlobalSettings *settings, BLooper *looper, uint32 flags)
	:
	BScrollView("BookmarksScroll", NULL, 0, true, true),
	mLooper(looper),
	mList(NULL),
	mCatalog(NULL),
	mBookmarks(NULL),
	mNeedsUpdate(true),
	mUserDefined(NULL),
	mEmptyUserBM(NULL)
{
	SetTarget(mList = new BOutlineListView("", B_SINGLE_SELECTION_LIST));
	mEmptyUserBM = new OutlineListItem(B_TRANSLATE("<empty>"), 1, true,
		GetDefaultStyle());
	SetCatalog(catalog, bookmarks);
}

OutlinesView::~OutlinesView() {
	if (mLooper) {
		BMessage msg(QUIT_NOTIFY);
		mLooper->PostMessage(&msg);
	}
	if (mList) {
		mList->RemoveItem(mEmptyUserBM);
		delete mEmptyUserBM;
		MakeEmpty(mList);
	}
}

void OutlinesView::AttachedToWindow() {
	mList->SetSelectionMessage(new BMessage('Outl'));
	mList->SetTarget(this);
}

void OutlinesView::SetCatalog(Catalog *catalog, BMessage *bookmarks) {
	if (mCatalog != catalog) {
		mCatalog     = catalog;
		mBookmarks   = bookmarks;
		mNeedsUpdate = true;
		InitUserBookmarks(true);
	}
}

void OutlinesView::Activate() {
	if (mNeedsUpdate) {
		mNeedsUpdate = false;
		mList->RemoveItem(mEmptyUserBM); // keep mEmptyUserBM
		MakeEmpty(mList);
		Object obj;
		mList->AddItem(new OutlineListItem(B_TRANSLATE("Document"), 0, true, GetDefaultStyle()));
		gPdfLock->Lock();
		if (mCatalog->getOutline()->isDict() && mCatalog->getOutline()->dictLookup("First", &obj) && !obj.isNull()) {
			ReadOutlines(&obj, 1);
		}
		gPdfLock->Unlock();
		if (mList->CountItems() == 1) {
			mList->AddItem(new OutlineListItem(B_TRANSLATE("<empty>"), 1, true, GetDefaultStyle()));
		}
		obj.free();
		mUserDefined = new OutlineListItem(B_TRANSLATE("User defined"), 0, true, GetDefaultStyle());
		mList->AddItem(mUserDefined);
		InitUserBookmarks(false);
	}
}


// handling of user bookmarks
void OutlinesView::InitUserBookmarks(bool initOnly) {
	mBookmark.Clear();
	if (mBookmarks == NULL || mBookmarks->IsEmpty()) {
		if (!initOnly) {
			mList->AddItem(mEmptyUserBM);
		}
	} else {
		BString label;
		int32   pageNum, i = 0;
		while (B_OK == mBookmarks->FindString("l", i, &label) &&
		       B_OK == mBookmarks->FindInt32 ("p", i, &pageNum)) {
	    	mBookmark.Set(pageNum, true);
		    if (!initOnly) {
		    	OutlineListItem *item = new OutlineListItem(label.String(), 1, true, GetDefaultStyle());
		    	item->SetPageNum(pageNum);
				mList->AddItem(item);
			}
			i ++;
		}
	}
}

static BListItem* store_bookmarks(BListItem *i, void *d) {
	OutlineListItem *item = (OutlineListItem*)i;
	BMessage    *bm   = (BMessage*)d;
	if (item->isPageNum()) {
		bm->AddString("l", item->Text());
		bm->AddInt32 ("p", item->getPageNum());
	}
	return NULL;
}

bool OutlinesView::GetBookmarks(BMessage *bm) {
	if (mList == NULL) {
		return false;
	}
	mList->EachItemUnder(mUserDefined, true, store_bookmarks, bm);
	return true;
}

static BListItem *find_bookmark(BListItem *i, void *d) {
	OutlineListItem *item = (OutlineListItem*)i;
	int pageNum       = *(int*)d;
	if (item->isPageNum() && item->getPageNum() == pageNum) return i;
	return NULL;
}

OutlineListItem *OutlinesView::FindUserBookmark(int pageNum) {
	if (mList == NULL) {
		return NULL;
	}
	return (OutlineListItem*)mList->EachItemUnder(mUserDefined, true, find_bookmark, &pageNum);
}

bool OutlinesView::HasUserBookmark(int pageNum) {
	return mBookmark.IsSet(pageNum);
}

bool OutlinesView::IsUserBMSelected() {
	if (mNeedsUpdate) return false;
	int i = mList->CurrentSelection(0);
	if (i >= 0) {
		OutlineListItem *item = (OutlineListItem*)mList->ItemAt(i);
		return item->isPageNum();
	}
	return false;
}

void OutlinesView::AddUserBookmark(int pageNum, const char *label) {
	RemoveUserBookmark(pageNum);
	if (mList->CountItemsUnder(mUserDefined, true) == 1) {
		mList->RemoveItem(mEmptyUserBM);
	}
	OutlineListItem* item;
	int32 i = 0;
	int32 index = mList->FullListIndexOf(mUserDefined)+1;
	item = (OutlineListItem*)mList->ItemUnderAt(mUserDefined, true, i);
	while(item != NULL) {
		if (item->isPageNum() && item->getPageNum() > pageNum) {
			// insert new OutlineListItem before item
			break;
		}
		i ++;
		item = (OutlineListItem*)mList->ItemUnderAt(mUserDefined, true, i);
	}
	index += i;
	OutlineListItem *n = new OutlineListItem(label, 1, true, GetDefaultStyle());
	n->SetPageNum(pageNum);
	mList->AddItem(n, index);
	mBookmark.Set(pageNum, true);
}

void OutlinesView::RemoveUserBookmark(int pageNum) {
	Activate();
	OutlineListItem* item;
	int32 i = 0;
	item = (OutlineListItem*)mList->ItemUnderAt(mUserDefined, true, i);
	while(item != NULL) {
		if (item->isPageNum() && item->getPageNum() == pageNum) {
			// remove item
			mList->RemoveItem(item);
			delete item;
			mBookmark.Set(pageNum, false);
			break;
		}
		i ++;
		item = (OutlineListItem*)mList->ItemUnderAt(mUserDefined, true, i);
	}
	if (mList->CountItemsUnder(mUserDefined, true) == 0) {
		mList->AddItem(mEmptyUserBM);
	}
}

const char *OutlinesView::GetUserBMLabel(int pageNum) {
	if (mNeedsUpdate) return NULL;
	OutlineListItem *item = FindUserBookmark(pageNum);
	if (item) return item->Text();
	return NULL;
}

void OutlinesView::MessageReceived(BMessage *msg) {
	if (msg->what == 'Outl') {
		// get first selected item
		int32 selected = mList->CurrentSelection(0);
		if (selected >= 0) {
			bool msgSent = false;
			OutlineListItem *item = (OutlineListItem*)mList->ItemAt(selected);
			if (item) {
				LinkDest *link = NULL;
				bool deleteLink = false;
				if (item->isDest()) {
					link = item->getDest();
				}
				if (link != NULL) {
					// XXX: race condition: Link handled after a new pdf document has been loaded.
					// Should add a field to the message that represents the current document,
					// to check in the handler of this message if still contains a vaild pointer.
					BMessage msg(DEST_NOTIFY);
					msg.AddPointer("dest", link);
					mLooper->PostMessage(&msg);
					msgSent = true;
				} else if (item->isString()) {
					BMessage msg(STRING_NOTIFY);
					msg.AddString("string", item->getString()->getCString());
					mLooper->PostMessage(&msg);
					msgSent = true;
				} else if (link && link->isPageRef()) {
					BMessage msg(REF_NOTIFY);
					Ref r = link->getPageRef();
					int32 num = r.num, gen = r.gen;
					msg.AddInt32("num", num);
					msg.AddInt32("gen", gen);
					mLooper->PostMessage(&msg);
					msgSent = true;
				} else {
					int32 p = -1;
					if (item->isPageNum()) {
						p = item->getPageNum();
					} else if (link) {
						p = link->getPageNum();
					}
					if (p != -1) {
						BMessage msg(PAGE_NOTIFY);
						msg.AddInt32("page", p);
						mLooper->PostMessage(&msg);
						msgSent = true;
					}
				}
				if (deleteLink) delete link;
				if (!msgSent) {
					// notify window that state has changed
					BMessage msg(STATE_CHANGE_NOTIFY);
					mLooper->PostMessage(&msg);
				}
			}
		}
	} else {
		BView::MessageReceived(msg);
	}
}


// BookmarkWindow

BookmarkWindow::BookmarkWindow(int pageNum, const char* title, BRect aRect, BLooper *looper)
	: BWindow(aRect, B_TRANSLATE("Edit title for bookmark"),
		B_TITLED_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS) {
	mLooper  = looper;
	mPageNum = pageNum;

	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));

	// center window

	aRect.OffsetBy(aRect.Width() / 2, aRect.Height() / 2);
	float width = 300, height = 45;
	aRect.SetRightBottom(BPoint(aRect.left + width, aRect.top + height));
	aRect.OffsetBy(-aRect.Width() / 2, -aRect.Height() / 2);
	MoveTo(aRect.left, aRect.top);
	ResizeTo(width, height);

	mTitle = new BTextControl("mTitle", "", title, NULL);

	BButton *button = new BButton("button", B_TRANSLATE("OK"), new BMessage('OK'));

	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(mTitle)
		.Add(button);

	SetDefaultButton(button);

	mTitle->MakeFocus();
	Show();
}


bool BookmarkWindow::QuitRequested() {
	return true;
}

void BookmarkWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case 'OK': {
		// post message to application

		BMessage msg(BOOKMARK_ENTERED_NOTIFY);
		msg.AddString("label", mTitle->Text());
		msg.AddInt32("pageNum", mPageNum);
		mLooper->PostMessage(&msg, NULL);
		Quit();
		break; }
	default:
		BWindow::MessageReceived(msg);
	}
}

