/*
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 * 	 Copyright (C) 2016 Adrián Arroyo Calle
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

#include <ctype.h>

#include <locale/Catalog.h>
#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Region.h>
#include <StringView.h>
#include <TabView.h>

#include "BePDF.h"
#include "FileInfoWindow.h"
#include "LayoutUtils.h"
#include "TextConversion.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FileInfoWindow"


const int16 FileInfoWindow::noKeys = 9;

const char *FileInfoWindow::systemKeys[] = {
	"Author",
	"CreationDate",
	"ModDate",
	"Creator",
	"Producer",
	"Title",
	"Subject",
	"Keywords",
	"Trapped"
};

const char *FileInfoWindow::authorKey = systemKeys[0];
const char *FileInfoWindow::creationDateKey = systemKeys[1];
const char *FileInfoWindow::modDateKey = systemKeys[2];
const char *FileInfoWindow::creatorKey = systemKeys[3];
const char *FileInfoWindow::producerKey = systemKeys[4];
const char *FileInfoWindow::titleKey = systemKeys[5];
const char *FileInfoWindow::subjectKey = systemKeys[6];
const char *FileInfoWindow::keywordsKey = systemKeys[7];
const char *FileInfoWindow::trappedKey = systemKeys[8];

static const char *YesNo(bool yesNo) {
	return yesNo ? B_TRANSLATE("Yes") : B_TRANSLATE("No");
}

static const char *Allowed(bool allowed) {
	return allowed ? B_TRANSLATE("Allowed") : B_TRANSLATE("Denied");
}

#define COPYN(N) for (n = N; n && date[i]; n--) s[j++] = date[i++];

static int ToInt(const char *s, int i, int n) {
	int d = 0;
	for (; n && s[i] && isdigit(s[i]) ; n --, i ++) {
		d = 10 * d + (int)(s[i] - '0');
	}
	return d;
}

// (D:YYYYMMDDHHmmSSOHH'mm')
static const char *ToDate(const char *date, time_t *time) {
static char s[80];
	struct tm d;
	memset(&d, 0, sizeof(d));

	if ((date[0] == 'D') && (date[1] == ':')) {
		int i = 2;
		// skip spaces
		while (date[i] == ' ') i++;
		int from = i;
		while (date[i] && isdigit(date[i])) i++;
		int to = i;
		int j = 0, n;
		i = from;
		d.tm_year = ToInt(date, i, 4) - 1900;
		if (to - from > 12)
			COPYN(to - from - 10)
		else
			COPYN(to - from - 4);
		s[j++] = '/';
		d.tm_mon = ToInt(date, i, 2)-1;
		COPYN(2)
		s[j++] = '/';
		d.tm_mday = ToInt(date, i, 2);
		COPYN(2)
		s[j++] = ' ';
		if (date[i]) {
			d.tm_hour = ToInt(date, i, 2);
			COPYN(2);
			s[j++] = ':';
			d.tm_min = ToInt(date, i, 2);
			COPYN(2);
			s[j++] = ':';
			d.tm_sec = ToInt(date, i, 2);
			COPYN(2);
			if (date[i]) {
				int off = 0;
				int sign = 1;
				s[j++] = ' ';
				if (date[i] == '-') sign = -1;
				s[j++] = date[i++];
				off = ToInt(date, i, 2) * 3600;
				COPYN(2); i++; // skip '
				s[j++] = ':';
				off += ToInt(date, i, 2) * 60;
				COPYN(2);
				d.tm_gmtoff = off * sign;
			}
		}
		s[j] = 0;
		if (time) *time = mktime(&d);
		return s;
	}
	else
		return date;
}

bool FileInfoWindow::IsSystemKey(const char *key) {
	for (int i = 0; i < noKeys; i++) {
		if (strcmp(key, systemKeys[i]) == 0) return true;
	}
	return false;
}

BString *FileInfoWindow::GetProperty(Dict *dict, const char *key, time_t *time) {
	Object obj, *item;
	BString *result = NULL;
	if (time) *time = 0;

	if ((item = dict->lookup((char*)key, &obj)) != NULL) {
		ObjType type = item->getType();
		if (type == objString) {
			GString *string = item->getString();
			const char *s = string->getCString();
			const char *date = ToDate(s, time);
			if (date != s) {
				result = TextToUtf8(date, strlen(date));
			} else {
				result = TextToUtf8(s, string->getLength());
			}
		}
	}
	obj.free();
	return result;
}

void FileInfoWindow::AddPair(BGridView *dest, BView *lv, BView *rv) {
	BGridLayout *layout = dest->GridLayout();
	int32 nextRow = layout->CountRows() + 1;
	layout->AddView(lv, 1, nextRow);
	layout->AddView(rv, 2, nextRow);
}

void FileInfoWindow::CreateProperty(BGridView *view, Dict *dict, const char *key, const char *title) {
	time_t time;
	BString *string = GetProperty(dict, key, &time);
	AddPair(view, new BStringView("", title),
		new BStringView("", (string) ? string->String() : "-"));
	delete string;
}

bool FileInfoWindow::AddFont(BList *list, GfxFont *font) {
	// Font already in list?
	Font **ids = (Font**)list->Items();
	for (int i = 0; i < list->CountItems(); i++) {
		if (((ids[i]->ref.num == font->getID()->num) &&
			(ids[i]->ref.gen == font->getID()->gen))) return false;
	}
	// Add font to list
	list->AddItem(new Font(*font->getID(), font->getName()));
	return true;
}

static void GetGString(BString &s, GString *g) {
	if (g) {
		BString *utf8 = TextToUtf8(g->getCString(), g->getLength());
		s = *utf8;
		delete utf8;
	}
}


// FontItem
class FontItem : public BRow
{
public:
	FontItem(const char *name, const char *embName, const char *type);
	~FontItem() {};
};

FontItem::FontItem(const char *name, const char *embName, const char *type)
	: BRow()
{
	SetField(new BStringField(name), 0);
	SetField(new BStringField(embName), 1);
	SetField(new BStringField(type), 2);
}

BRow* FileInfoWindow::FontItem(GfxFont* font)
{
	BString name;
	GetGString(name, font->getName());

	BString embName;
	if (font->getEmbeddedFontName()) {
		const char* name = font->getEmbeddedFontName()->getCString();
		BString *utf8 = TextToUtf8(name, font->getEmbeddedFontName()->getLength());
		embName = *utf8;
		delete utf8;
	}

	BString type;
	switch (font->getType()) {
	case fontUnknownType: type = "Unknown Type"; break;
	case fontType1: 	type = "Type 1"; break;
	case fontType1C:	type = "Type 1C"; break;
	case fontType3:		type = "Type 3"; break;
	case fontTrueType:	type = "TrueType"; break;
	case fontCIDType0:	type = "CID Type 0"; break;
	case fontCIDType0C: type = "CID Type 0C"; break;
	case fontCIDType2:	type = "CID Type 2"; break;
	case fontType1COT:	type = "Type 1C (OpenType)"; break;
	case fontTrueTypeOT:type = "TrueType 0 (OpenType)"; break;
	case fontCIDType0COT:type = "CID Type 0C (OpenType)"; break;
	case fontCIDType2OT:type = "CID Type2 (OpenType)"; break;
	}
	return new ::FontItem(name.String(), embName.String(), type.String());
}


void FileInfoWindow::QueryFonts(PDFDoc *doc, int page)
{
	Catalog *catalog = doc->getCatalog();

	// remove items from font list
	Lock();
	mFontList->Clear();
	Unlock();

	BList fontList;
	int first, last;
	if (page == 0) {
		first = 1; last = doc->getNumPages();
	} else {
		first = last = page;
	}

	for (int pg = first; pg <= last; pg++) {
		if ((mState == STOP) || (mState == QUIT)) break;

		Page *page = catalog->getPage(pg);
		Dict *resDict;
		if ((resDict = page->getResourceDict()) != NULL) {
			Object obj1;
			resDict->lookupNF("Font", &obj1);
			if (obj1.isRef()) {
				Object obj2;
				obj1.fetch(doc->getXRef(), &obj2);
				if (obj2.isDict()) {
					Ref r = obj1.getRef();
					GfxFontDict* gfxFontDict = new GfxFontDict(doc->getXRef(),
						&r, obj2.getDict());
					for (int i = 0; i < gfxFontDict->getNumFonts(); i++) {
						GfxFont* font = gfxFontDict->getFont(i);
						if (font != NULL && AddFont(&fontList, font)) {
							Lock();
							mFontList->AddRow(FileInfoWindow::FontItem(font));
							Unlock();
						}
					}
					obj2.free();
					delete gfxFontDict;
				}
			}
			obj1.free();
		}
	}

	Font **ids = (Font**)fontList.Items();
	for (int i = 0; i < fontList.CountItems(); i++) {
		delete ids[i];
	}
	return;
}

void FileInfoWindow::Refresh(BEntry *file, PDFDoc *doc, int page) {
	PDFLock lock;

	mState = NORMAL;

	BTabView *tabs = new BTabView("tabs", B_WIDTH_FROM_LABEL);

	BGridView *document = new BGridView();

	BPath path;
	if (file->GetPath(&path) == B_OK) {
		AddPair(document, new BStringView("", B_TRANSLATE("Filename:")),
			new BStringView("", path.Leaf()));
		AddPair(document, new BStringView("", B_TRANSLATE("Path:")),
			new BStringView("", path.Path()));
	}

	Object obj;
	if (doc->getDocInfo(&obj) && obj.isDict()) {
		Dict *dict = obj.getDict();

		CreateProperty(document, dict, titleKey, B_TRANSLATE("Title:"));
		CreateProperty(document, dict, subjectKey, B_TRANSLATE("Subject:"));
		CreateProperty(document, dict, authorKey, B_TRANSLATE("Author:"));
		CreateProperty(document, dict, keywordsKey, B_TRANSLATE("Keywords:"));
		CreateProperty(document, dict, creatorKey, B_TRANSLATE("Creator:"));
		CreateProperty(document, dict, producerKey, B_TRANSLATE("Producer:"));
		CreateProperty(document, dict, creationDateKey, B_TRANSLATE("Created:"));
		CreateProperty(document, dict, modDateKey, B_TRANSLATE("Modified:"));

		for (int i = 0; i < dict->getLength(); i++) {
			if (!IsSystemKey(dict->getKey(i))) {
				BString title(dict->getKey(i));
				title << ":";
				CreateProperty(document, dict, dict->getKey(i), title.String());
			}
		}
	}
    obj.free();

	char ver[80];
	sprintf(ver, "%.1f", doc->getPDFVersion());
	AddPair(document, new BStringView("", B_TRANSLATE("Version:")), new BStringView("", ver));

	AddPair(document, new BStringView("", B_TRANSLATE("Linearized:")),
		new BStringView("", YesNo(doc->isLinearized())));

	BView *docView = new BView(B_TRANSLATE("Document"), 0);
	BLayoutBuilder::Group<>(docView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_HORIZONTAL)
			.Add(document)
			.AddGlue()
		.End()
		.AddGlue();

	tabs->AddTab(docView);

	// Security
	BGridView *security = new BGridView();

	AddPair(security, new BStringView("", B_TRANSLATE("Encrypted:")), new BStringView("", YesNo(doc->isEncrypted())));
	AddPair(security, new BStringView("", B_TRANSLATE("Printing:")), new BStringView("", Allowed(doc->okToPrint())));
	AddPair(security, new BStringView("", B_TRANSLATE("Editing:")), new BStringView("", Allowed(doc->okToChange())));
	AddPair(security, new BStringView("", B_TRANSLATE("Copy & paste:")), new BStringView("", Allowed(doc->okToCopy())));
	AddPair(security, new BStringView("", B_TRANSLATE("Annotations:")), new BStringView("", Allowed(doc->okToAddNotes())));

	BView *secView = new BView(B_TRANSLATE("Security"), 0);
	BLayoutBuilder::Group<>(secView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_HORIZONTAL)
			.Add(security)
			.AddGlue()
		.End()
		.AddGlue();

	tabs->AddTab(secView);

	// Fonts
	mFontList = new BColumnListView(BRect(0, 0, 100, 100), NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_NO_BORDER,true);
	mFontList->AddColumn(new BStringColumn(B_TRANSLATE("Name"), 150.0, 150.0,150.0,true),0);
	mFontList->AddColumn(new BStringColumn(B_TRANSLATE("Embedded name"), 150.0,150.0,150.0,true),1);
	mFontList->AddColumn(new BStringColumn(B_TRANSLATE("Type"), 80.0,80.0,80.0,true),2);

	mShowAllFonts = new BButton("showAllFonts", B_TRANSLATE("Show all fonts"), new BMessage(SHOW_ALL_FONTS_MSG));
	mStop = new BButton("stop", B_TRANSLATE("Abort"), new BMessage(STOP_MSG));

	BView *fonts = new BView(B_TRANSLATE("Fonts of this page"), 0);

	BLayoutBuilder::Group<>(fonts, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(mFontList)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(mShowAllFonts)
			.Add(mStop)
		.End();

	tabs->AddTab(fonts);
	tabs->SetBorder(B_NO_BORDER);

	mStop->SetEnabled(false);
	QueryFonts(doc, page);

	BLayoutBuilder::Group<>(this)
		.SetInsets(0, B_USE_WINDOW_INSETS, 0, 0)
		.Add(tabs);

	Show();
}

void FileInfoWindow::RefreshFontList(BEntry *file, PDFDoc *doc, int page) {
	if (mState == NORMAL) {
		QueryFonts(doc, page);
	}
}

void FileInfoWindow::QueryAllFonts(PDFDoc *doc) {
	QueryFonts(doc, 0);
	PostMessage(FONT_QUERY_STOPPED_NOTIFY);
}

FileInfoWindow::FileInfoWindow(GlobalSettings *settings, BEntry *file, PDFDoc *doc,
	BLooper *looper, int page)
	: BWindow(BRect(0, 0, 100, 100), B_TRANSLATE("File info"),
		B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_AUTO_UPDATE_SIZE_LIMITS),
		mLooper(looper), mSettings(settings), mState(NORMAL) {

	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));

	MoveTo(settings->GetFileInfoWindowPosition());
	float w, h;
	settings->GetFileInfoWindowSize(w, h);
	ResizeTo(w, h);
	mView = NULL;

	Refresh(file, doc, page);
}

bool FileInfoWindow::QuitRequested() {
	if ((mState == NORMAL) || (mState == ALL_FONTS)) {
		if (mLooper) {
			BMessage msg(QUIT_NOTIFY);
			mLooper->PostMessage(&msg);
		}
		return true;
	} else {
		if (mState == QUERY_ALL_FONTS) {
			mStop->SetEnabled(false); mState = QUIT;
		}
		return false;
	}
}

void FileInfoWindow::FrameMoved(BPoint p) {
	mSettings->SetFileInfoWindowPosition(p);
	BWindow::FrameMoved(p);
}

void FileInfoWindow::FrameResized(float w, float h) {
	mSettings->SetFileInfoWindowSize(w, h);
	BWindow::FrameResized(w, h);
}

void FileInfoWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case SHOW_ALL_FONTS_MSG:
		mState = QUERY_ALL_FONTS;
		mShowAllFonts->SetEnabled(false);
		mStop->SetEnabled(true);
		if (mLooper) {
			// do searching in another thread (= thread of window)
			mLooper->PostMessage(START_QUERY_ALL_FONTS_MSG);
		}
		break;
	case STOP_MSG:
		if (mState == QUERY_ALL_FONTS) {
			mStop->SetEnabled(false); mState = STOP;
		}
		break;
	case FONT_QUERY_STOPPED_NOTIFY:
		switch (mState) {
		case STOP:
			mState = NORMAL;
			mShowAllFonts->SetEnabled(true);
			break;
		case QUERY_ALL_FONTS:
			mState = ALL_FONTS;	mStop->SetEnabled(false);
			break;
		case QUIT:
			mState = NORMAL; PostMessage(B_QUIT_REQUESTED);
			break;
		 default:
		 	break; // empty
		}
		break;
	default:
		BWindow::MessageReceived(msg);
	}
}

