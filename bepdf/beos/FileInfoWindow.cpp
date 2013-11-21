/*  
	BeOS Front-end du PDF file reader xpdf.
	Copyright (C) 2000-2002 Michael Pfeiffer

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

#include <ctype.h>
#include <be/storage/Path.h>
#include <be/interface/Region.h>

#include <layout-all.h>
#include "FileInfoWindow.h"
#include "LayoutUtils.h"
#include "TextConversion.h"
#include "StringLocalization.h"
#include "BePDF.h"

static minimax maximum(0, 0, 10e10, 10e10, 100);

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
	return yesNo ? TRANSLATE("Yes") : TRANSLATE("No");
}

static const char *Allowed(bool allowed) {
	return allowed ? TRANSLATE("Allowed") : TRANSLATE("Denied");
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
	memset(&d, sizeof(d), 0);

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

void FileInfoWindow::AddPair(VGroup *l, VGroup *r, BView *lv, BView *rv) {
	l->AddChild(lv); r->AddChild(rv);
}

void FileInfoWindow::CreateProperty(VGroup *l, VGroup *r, Dict *dict, const char *key, const char *title) {
	static minimax mm(0, 0, 60000, 60000, 10);
	time_t time;
	BString *string = GetProperty(dict, key, &time);
	AddPair(l, r, new MStringView(title),
		new MStringView( (string) ? string->String() : "-"));
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
class FontItem : public CLVListItem
{
public:
	FontItem(uint32 level, bool superitem, bool expanded, const char *name, const char *embName, const char *type);
	~FontItem() {};
	void DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
	void Update(BView *owner, const BFont *font);
	static int MyCompare(const CLVListItem* Item1, const CLVListItem* Item2, int32 KeyColumn);

private:
	static rgb_color kHighlight, kRedColor, kDimRedColor, kBlackColor, kMedGray;
	BString fText[3];
	float fTextOffset;
};

rgb_color FontItem::kHighlight = {128, 128, 128, 0},
	FontItem::kRedColor = {255, 0, 0, 0},
	FontItem::kDimRedColor = {128, 0, 0, 0},
	FontItem::kBlackColor = {0, 0, 0, 0},
	FontItem::kMedGray = {192, 192, 192, 0};
	
FontItem::FontItem(uint32 level, bool superitem, bool expanded, const char *name, const char *embName, const char *type) 
: CLVListItem(level, superitem, expanded, 20.0) {
	fText[0] = name; 
	fText[1] = embName;
	fText[2] = type;
}

void FontItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
{
	rgb_color color;
	rgb_color White = {255, 255, 255};
	rgb_color BeListSelectGrey = {128, 128, 128};
	rgb_color Black = {0, 0, 0};
	bool selected = IsSelected();
	if(selected)
		color = BeListSelectGrey;
	else
		color = White;
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{
		owner->SetHighColor(color);
		owner->FillRect(item_column_rect);
	}
	BRegion Region;
	Region.Include(item_column_rect);
	owner->ConstrainClippingRegion(&Region);
	if (column_index == 2) {
		owner->SetHighColor(selected ? kDimRedColor : kRedColor);
	} else {
		owner->SetHighColor(Black);
	}
	if(column_index >= 0)
		owner->DrawString(fText[column_index].String(),
			BPoint(item_column_rect.left+2.0,item_column_rect.top+fTextOffset));
	owner->ConstrainClippingRegion(NULL);
}


void FontItem::Update(BView *owner, const BFont *font)
{
	CLVListItem::Update(owner,font);
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
	fTextOffset = ceil(FontAttributes.ascent) + (Height()-FontHeight)/2.0;
}


int FontItem::MyCompare(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn)
{
	BString* Text1 = &((FontItem*)a_Item1)->fText[KeyColumn];
	BString* Text2 = &((FontItem*)a_Item2)->fText[KeyColumn];
	return strcasecmp(Text1->String(), Text2->String());
}

BListItem *FileInfoWindow::FontItem(GfxFont *font) {
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
	case fontUnknownType: type = "Unknown Type";
		break;
	case fontType1: type = "Type 1";
		break;
	case fontType1C: type = "Type 1C";
		break;
	case fontType3: type = "Type 3";
		break;
	case fontTrueType: type = "TrueType";
		break;
	case fontCIDType0: type = "CID Type 0";
		break;
	case fontCIDType0C: type = "CID Type 0C";
		break;
	case fontCIDType2: type = "CID Type 2";
		break;
	case fontType1COT: type = "Type 1C OpenType";
		break;
	case fontTrueTypeOT: type = "TrueType 0 OpenType";
		break;
	case fontCIDType0COT: type = "CID Type 0C OpenType";
		break;
	case fontCIDType2OT: type = "CID Type2 OpenType";
		break;
	}
	return new ::FontItem(0, false, false, name.String(), embName.String(), type.String());	
}

void FileInfoWindow::QueryFonts(PDFDoc *doc, int page) {

	Catalog *catalog = doc->getCatalog();
	GfxFontDict *gfxFontDict;
	GfxFont *font;

	// remove items from font list
	Lock();
	MakeEmpty(mFontList);
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
			Object fontDict;
			resDict->lookup("Font", &fontDict);
			if (fontDict.isDict()) {
				// TODO check if ref to Font dict has to be passed in!
				gfxFontDict = new GfxFontDict(doc->getXRef(), NULL, fontDict.getDict());
				for (int i = 0; i < gfxFontDict->getNumFonts(); i++) {
					font = gfxFontDict->getFont(i);
					if (font != NULL && AddFont(&fontList, font)) {
						Lock();
						mFontList->AddItem(FontItem(font));
						Unlock();
					}
				}
				delete gfxFontDict;
			}
			fontDict.free();
		}	
	}
	
	Font **ids = (Font**)fontList.Items();
	for (int i = 0; i < fontList.CountItems(); i++) {
		delete ids[i];
	}
	return ;
}

void FileInfoWindow::Refresh(BEntry *file, PDFDoc *doc, int page) {
	PDFLock lock;
	
	if (mView) {
		RemoveChild(mView);
		delete mView; mView = NULL;
	}

	mState = NORMAL;

	// Document
	VGroup *group = new VGroup();
	VGroup *left, *right;
	group->AddChild(
		new HGroup(
			left = new VGroup(minimax(0,0,6000,6000,1), NULL),
			right = new VGroup(minimax(0,0,6000,6000,10), NULL),
			0),
		0
	);
	
	BPath path;
	if (file->GetPath(&path) == B_OK) {	
		char ver[80];
		sprintf(ver, "%.1f", doc->getPDFVersion());
		AddPair(left, right, new MStringView(TRANSLATE("Filename:")),
			new MStringView(path.Leaf()));
		AddPair(left, right, new MStringView(TRANSLATE("Path:")),
			new MStringView(path.Path()));
	}

	Object obj;
	if (doc->getDocInfo(&obj) && obj.isDict()) {
		Dict *dict = obj.getDict();
		
		CreateProperty(left, right, dict, titleKey, TRANSLATE("Title:"));
		CreateProperty(left, right, dict, subjectKey, TRANSLATE("Subject:"));
		CreateProperty(left, right, dict, authorKey, TRANSLATE("Author:"));
		CreateProperty(left, right, dict, keywordsKey, TRANSLATE("Keywords:"));
		CreateProperty(left, right, dict, creatorKey, TRANSLATE("Creator:"));
		CreateProperty(left, right, dict, producerKey, TRANSLATE("Producer:"));
		CreateProperty(left, right, dict, creationDateKey, TRANSLATE("Created:"));
		CreateProperty(left, right, dict, modDateKey, TRANSLATE("Modified:"));

		for (int i = 0; i < dict->getLength(); i++) {
			if (!IsSystemKey(dict->getKey(i))) {
				BString title(dict->getKey(i));
				title << ":";
				CreateProperty(left, right, dict, dict->getKey(i), title.String());
			}
		}
	}
    obj.free();
    
	char ver[80];
	sprintf(ver, "%.1f", doc->getPDFVersion());
	AddPair(left, right, new MStringView(TRANSLATE("Version:")), new MStringView(ver));

	AddPair(left, right, new MStringView(TRANSLATE("Linearized:")), 
		new MStringView(YesNo(doc->isLinearized())));

	// Security
	VGroup *security = new VGroup(
		new HGroup(
			left = new VGroup(minimax(0,0,6000,6000,1), NULL),
			right = new VGroup(minimax(0,0,6000,6000,10), NULL),
			0),
		0
	);

	AddPair(left, right, new MStringView(TRANSLATE("Encrypted:")), new MStringView(YesNo(doc->isEncrypted())));
	AddPair(left, right, new MStringView(TRANSLATE("Printing:")), new MStringView(Allowed(doc->okToPrint())));
	AddPair(left, right, new MStringView(TRANSLATE("Editing:")), new MStringView(Allowed(doc->okToChange())));
	AddPair(left, right, new MStringView(TRANSLATE("Copy & paste:")), new MStringView(Allowed(doc->okToCopy())));
	AddPair(left, right, new MStringView(TRANSLATE("Annotations:")), new MStringView(Allowed(doc->okToAddNotes())));
	


	// Fonts
	CLVContainerView* containerView;
	mFontList = new ColumnListView(BRect(0, 0, 100, 100), &containerView, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, false, B_NO_BORDER);
	mFontList->AddColumn(new CLVColumn(TRANSLATE("Name"), 150.0, CLV_SORT_KEYABLE));
	mFontList->AddColumn(new CLVColumn(TRANSLATE("Embedded Name"), 150.0, CLV_SORT_KEYABLE));
	mFontList->AddColumn(new CLVColumn(TRANSLATE("Type"), 80.0, CLV_SORT_KEYABLE));
	mFontList->SetSortFunction(FontItem::MyCompare);

	MView *fonts = new VGroup(
		mFontsBorder = new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Fonts of this page"),
			new MBViewWrapper(containerView, true, false, false)),
		new HGroup(
			mShowAllFonts = new MButton(TRANSLATE("Show All Fonts"), new BMessage(SHOW_ALL_FONTS_MSG), NULL, minimax(-1, -1, -1, -1)),
			mStop = new MButton(TRANSLATE("Abort"), new BMessage(STOP_MSG), NULL, minimax(-1, -1, -1, -1)),
			0),
		0);
	mStop->SetEnabled(false);
	QueryFonts(doc, page);
	
	MView *view = new TabGroup(
		TRANSLATE("Document"), AlignTop(group),
		TRANSLATE("Security"), AlignTop(security),
		TRANSLATE("Fonts"), fonts,
		0);
		
	AddChild(mView = dynamic_cast<BView*>(view));
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
	: MWindow(BRect(0, 0, 100, 100), TRANSLATE("File Info"), 
		B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0), 
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
	MWindow::FrameMoved(p);
}

void FileInfoWindow::FrameResized(float w, float h) {
	mSettings->SetFileInfoWindowSize(w, h);
	MWindow::FrameResized(w, h);
}

void FileInfoWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case SHOW_ALL_FONTS_MSG: 
		mState = QUERY_ALL_FONTS;
		mShowAllFonts->SetEnabled(false); mStop->SetEnabled(true);
		mFontsBorder->SetLabel(TRANSLATE("Searching all fonts…"));
		mFontsBorder->Invalidate();
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
			mFontsBorder->SetLabel(TRANSLATE("All fonts of this document (aborted)"));
			mFontsBorder->Invalidate();
			break;
		case QUERY_ALL_FONTS:
			mState = ALL_FONTS;	mStop->SetEnabled(false);
			mFontsBorder->SetLabel(TRANSLATE("All fonts of this document"));		
			mFontsBorder->Invalidate();
			break;
		case QUIT:
			mState = NORMAL; PostMessage(B_QUIT_REQUESTED);
			break;
		 default:
		 	break; // empty
		}
		break;
	default:
		MWindow::MessageReceived(msg);
	}
}

