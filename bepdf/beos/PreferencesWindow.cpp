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

#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <be/storage/Path.h>
#include <be/interface/MenuItem.h>

// xpdf
#include <GlobalParams.h>

#include <layout-all.h>

#include "BepdfApplication.h"
#include "LayoutUtils.h"
#include "PreferencesWindow.h"
#include "StringLocalization.h"

static minimax maximum(0, 0, 10e10, 10e10, 100);

void PreferencesWindow::UpdateWorkspace() {
	GlobalSettings* settings = gApp->GetSettings();
	bool enabled = settings->GetOpenInWorkspace();
	int32 ws = settings->GetWorkspace();
	BMenu *m = mOpenInWorkspace->Menu();
	int32 n = m->CountItems();
	int32 i;
	i = (enabled) ? 1 + ws : 0;
	if (i >= n) i = n - 1;
	BMenuItem *item = m->ItemAt(i);
	item->SetMarked(true);
}

void PreferencesWindow::BuildWorkspaceMenu(BMenu *m) {
	m->AddItem(new BMenuItem(TRANSLATE("current"), new BMessage(WORKSPACE_CHANGED)));
	m->AddSeparatorItem();
	int n = count_workspaces();
	for (int i = 1; i <= n; i ++) {
		char buffer[10];
		sprintf(buffer, "%d", i);
		m->AddItem(new BMenuItem(buffer, new BMessage(WORKSPACE_CHANGED)));
	}
	m->SetLabelFromMarked(true);
	m->SetRadioMode(true);
}

void PreferencesWindow::SetupView() {
	char workspace[3];
	
	GlobalSettings* settings = gApp->GetSettings();
	sprintf(workspace, "%d", (int)settings->GetWorkspace());
	mPreferences = new MOutlineListView();

	BListItem *item;
	mPreferences->AddItem(new BStringItem(TRANSLATE("Document")));

	mPreferences->AddItem(item = new BStringItem(TRANSLATE("Display")));
		// reverse order:
		mPreferences->AddUnder(new BStringItem(TRANSLATE("Asian Fonts")), item);
		mPreferences->AddUnder(new BStringItem(TRANSLATE("FreeType 2")), item);
		
	if (!StringLocalization::FromEnvironmentVariables()) {
		// XXX This works because it is the last item!
		mPreferences->AddItem(new BStringItem(TRANSLATE("Language")));
	}
	mPreferences->SetSelectionMessage(new BMessage(PREFERENCE_SELECTED));

	MTextControl* author = NULL;
	
	mLayers = new LayeredGroup(
		// Document
		new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Document"),
			AlignTop(new VGroup(
				new MCheckBox(TRANSLATE("Restore Page Number"), new BMessage(RESTORE_PAGE_NO_CHANGED), NULL, settings->GetRestorePageNumber()),
				new MCheckBox(TRANSLATE("Restore Window Position and Size"), new BMessage(RESTORE_WINDOW_FRAME_CHANGED), NULL, settings->GetRestoreWindowFrame()),
				mOpenInWorkspace = new MPopup(TRANSLATE("Open in Workspace:"), NULL),
				author = new MTextControl(TRANSLATE("Author"), (char*)settings->GetAuthor(), new BMessage(AUTHOR_CHANGED)),
			0))				
		),

		// Display
		AlignTop(new VGroup(
			new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Fullscreen Mode"), 
				AlignTop(mDisplay[DISPLAY_FULLSCREEN] = new MRadioGroup(new BMessage(QUASI_FULLSCREEN_MODE_CHANGED),
					TRANSLATE("Show Document view only"),
					TRANSLATE("Show Toolbar, Statusbar and Scrollbars too"),
					0))
			),
			new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Selection Rectangle"), 
				AlignTop(mDisplay[DISPLAY_FILLED_SELECTION] = new MRadioGroup(new BMessage(FILLED_SELECTION_CHANGED),
					TRANSLATE("Filled Rectangle"),
					TRANSLATE("Stroked Rectangle"),
					0))
			),
			new MCheckBox(TRANSLATE("Invert vertical mouse scrolling"), new BMessage(INVERT_VERTICAL_SCROLLING_CHANGED), NULL, settings->GetInvertVerticalScrolling()),
			0)	
		),
		
		// FreeType 2
		new MBorder(M_LABELED_BORDER, 5, TRANSLATE("FreeType 2"),
			AlignTop(new VGroup(
				new MCheckBox(TRANSLATE("Hinting"), new BMessage(HINTING_CHANGED), NULL, settings->GetHinting()),
				0)
			)
		),
		
		// Asian Fonts
		new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Asian Fonts"),
			BuildAsianFontsView()
		),
		
		// localization
		new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Language"),
			new VGroup(
				new MScrollView ( mList = new MListView(), false, true),
				new MStringView(TRANSLATE("Will take effect at next program launch."), B_ALIGN_LEFT, minimax(10, 10, 65536, 40, 0.10)),
				0)),
		0);

	author->SetModificationMessage(new BMessage(AUTHOR_CHANGED));
		
	mView = new HGroup(
		new MBorder(M_LABELED_BORDER, 5, TRANSLATE("Preferences"),
			mPreferences
		), 
		mLayers, 
		0);

	AddChild(dynamic_cast<BView*>(mView));
	mPreferences->Select(0);

	mDisplay[DISPLAY_FULLSCREEN]->SetActive(settings->GetQuasiFullscreenMode() ? 1 : 0);
	mDisplay[DISPLAY_FILLED_SELECTION]->SetActive(settings->GetFilledSelection() ? 0 : 1);

	// localization
	BPath path(*gApp->GetAppPath()); path.Append("locale");
	BDirectory dir(path.Path());
	int32 selectIndex = 0;
	if (dir.InitCheck() == B_OK) {
		mList->SetSelectionMessage(new BMessage(LANGUAGE_SELECTED));
		BEntry entry; char name[B_FILE_NAME_LENGTH];
		const char *language = settings->GetLanguage();
		BStringItem *item;
		bool selected;
		bool found = false;
		int32 i = 0;
		int32 english = 0;
		char *s;
		while (dir.GetNextEntry(&entry) == B_OK) {
			entry.GetName(name);
			selected = strcmp(language, name) == 0;
			if ((s = strstr(name, "catalog")) && strcmp(name, "Template.catalog") != 0 && s[7] == 0) {
				char *s = strchr(name, '.'); if (s) *s = 0;
				mList->AddItem(item = new BStringItem(name));
				if (strcmp(name, "English") == 0) english = i;
				if (selected) {
					selectIndex = i;
					found = true;
				}
				i ++;
			}
		}
		mList->Select(found ? selectIndex : english);
	} 

	BuildWorkspaceMenu(mOpenInWorkspace->Menu());
	UpdateWorkspace();
}

static int compareMPopupLabels(const void* p1, const void* p2) {
	const MPopup** popup1 = (const MPopup**)p1;
	const MPopup** popup2 = (const MPopup**)p2;
	return strcmp((*popup1)->Label(), (*popup2)->Label());
}

MView* PreferencesWindow::BuildAsianFontsView() {
	if (mDisplayCIDFonts->GetSize() == 0) {
		return new VGroup();
	}
	
	BList fonts;
	// TODO check who deletes dividable
	MDividable* dividable = new MDividable();
	dividable->labelwidth = 0;

	for (int32 index = 0; index < mDisplayCIDFonts->GetSize(); index ++) {
		BString name;
		BString file;
		DisplayCIDFonts::Type type;
		mDisplayCIDFonts->Get(index, name, file, type);
		
		BString label(TRANSLATE(name.String()));
		label << ":";
		MPopup* popup = new MPopup((char*)label.String(), NULL);
		FillFontFileMenu(popup, name.String(), file.String());

		// make all labels same size
		popup->DivideSameAs(dividable);
		float width = be_plain_font->StringWidth(label.String()) + 5;
		if (dividable->labelwidth < width) {
			dividable->labelwidth = width;
		}
		
		fonts.AddItem(popup);
	}
	
	// sort MPopup alphabetically by label
	fonts.SortItems(compareMPopupLabels);
	
	// add popup menus into vertical group
	VGroup* group = new VGroup();

	for (int32 index = 0; index < fonts.CountItems(); index ++) {
		group->AddChild((MPopup*)fonts.ItemAt(index));
	}

	group->AddChild(new Space(GetFiller()));
	
	group->AddChild(new MStringView(
		TRANSLATE("Will take effect at next program launch."), 
		B_ALIGN_LEFT));
	
	return group;
}

static bool endsWith(const BString& string, const char* suffix) {
	const char* stringPtr = string.String();
	int stringLength = string.Length();
	int suffixLength = strlen(suffix);
	if (stringLength < suffixLength) {
		return gFalse;
	}
	
	return strcmp(suffix, &stringPtr[stringLength - suffixLength]) == 0;
}

DisplayCIDFonts::Type PreferencesWindow::GetType(const char* file) {
	BString string(file);
	string.ToLower();
	if (endsWith(string, ".ttf") ||
		endsWith(string, ".ttc")) {
		return DisplayCIDFonts::kTrueType;
	} else if (endsWith(string, ".pfb")) {
		return DisplayCIDFonts::kType1;
	}
	return DisplayCIDFonts::kUnknownType;
}

void PreferencesWindow::FillFontFileMenu(BMenuField* menuField, const char* name, const char* file) {
	FillFontFileMenu(menuField, B_BEOS_FONTS_DIRECTORY, TRANSLATE("System Fonts"), name, file);
	FillFontFileMenu(menuField, B_COMMON_FONTS_DIRECTORY, TRANSLATE("Common Fonts"), name, file);
	// B_USER_FONTS_DIRECTORY is same as B_COMMON_FONTS_DIRECTORY in 
	// BeOS R5
	// FillFontFileMenu(menu, B_USER_FONTS_DIRECTORY, TRANSLATE("User Fonts"), name, file);
}

void PreferencesWindow::FillFontFileMenu(BMenuField* menuField, directory_which which, const char* label, const char* name, const char* file) {
	BMenu* menu = menuField->Menu();
	mFontMenuFields.AddPointer(name, menuField);
	
	BPath path;
	if (find_directory(which, &path) != B_OK) {
		return;
	}
	
	BDirectory fontDirectory(path.Path());
	BEntry entry;
	BMenu* fontMenu = NULL;
	// iterate through font directory
	while (fontDirectory.GetNextEntry(&entry) == B_OK) {
		if (!entry.IsDirectory()) {
			continue;
		}
		BDirectory subDirectory(&entry);
		BEntry fontFile;
		BMenu* fontSubMenu = NULL;
		// iteratoe through sub directory in font directory
		while (subDirectory.GetNextEntry(&fontFile) == B_OK) {
			if (!fontFile.IsFile()) {
				continue;
			}
			
			BPath filePath;
			if (fontFile.GetPath(&filePath) != B_OK) {
				continue;
			}
			// skip unknown font types
			if (GetType(filePath.Path()) == DisplayCIDFonts::kUnknownType) {
				continue;
			}
			
			if (fontMenu == NULL) {
				// lazy add font menu
				fontMenu = new BMenu(label);
				fontMenu->SetRadioMode(false);
				menu->AddItem(fontMenu);
			}
			
			if (fontSubMenu == NULL) {
				// lazy add sub font menu
				char name[B_FILE_NAME_LENGTH+1];
				entry.GetName(name);
				fontSubMenu = new BMenu(name);
				fontSubMenu->SetRadioMode(false);
				fontMenu->AddItem(fontSubMenu);
			}
			
			// add menu item for font
			BMessage* msg = new BMessage(DISPLAY_CID_FONT_SELECTED);
			msg->AddString("name", name);
			msg->AddString("file", filePath.Path());
			BMenuItem* item = new BMenuItem(filePath.Leaf(), msg);
			fontSubMenu->AddItem(item);
			
			// set label from current font
			if (strcmp(filePath.Path(), file) == 0) {
				menuField->MenuItem()->SetLabel(filePath.Leaf());
			}
		}
	}
}

void PreferencesWindow::ClearView() {
	MakeEmpty(mList);
	RemoveChild(dynamic_cast<BView*>(mView));
	delete mView; mView = NULL;
}

PreferencesWindow::PreferencesWindow(GlobalSettings *settings, BLooper *looper) 
	: MWindow(BRect(0, 0, 100, 100)
		, TRANSLATE("Preferences")
		, B_TITLED_WINDOW_LOOK, 
		B_FLOATING_APP_WINDOW_FEEL, 
		0)
	, mLooper(looper)
	, mSettings(settings)
{	
	BMessage msg;
	settings->GetDisplayCIDFonts(msg);
	mDisplayCIDFonts = new DisplayCIDFonts(msg);

	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));

	MoveTo(settings->GetPrefsWindowPosition());
	float w, h;
	settings->GetPrefsWindowSize(w, h);
	ResizeTo(w, h);
	
	SetupView();
	
	Show();
}

PreferencesWindow::~PreferencesWindow() {
	delete mDisplayCIDFonts;
	mDisplayCIDFonts = NULL;
}

class TranslatedFileItem : public BStringItem {
	BString mFileName;
public:
	TranslatedFileItem(const char* name, const char* filename) :
	  BStringItem(name), mFileName(filename) { }
	const char* FileName() const { return mFileName.String(); }
};

bool PreferencesWindow::FindRadioGroup(MRadioGroup *group, bool &displayPrinter, int32 &index) {
	for (int i = 0; i < DISPLAY_NUM; i++) {
		if (group == mDisplay[i]) {
			displayPrinter = true; index = i;
			return true;
		}
	} 
	return false;
}

bool PreferencesWindow::DecodeMessage(BMessage *msg, int16 &kind, int16 &which, int16 &index) {
	// assert msg->what == PREFERENCES_CHANGED_NOTIFY
	return ((B_OK == msg->FindInt16("kind", &kind)) 
		&& (B_OK == msg->FindInt16("which", &which))
		&& (B_OK == msg->FindInt16("index", &index)));
}

void PreferencesWindow::Notify(uint32 what) {
	BMessage m(what);
	mLooper->PostMessage(&m);
}

void PreferencesWindow::NotifyRestartDoc() {
	Notify(RESTART_DOC_NOTIFY);
}

void PreferencesWindow::DisplayCIDFontSelected(BMessage* msg) {
	BString name;
	BString file;
	if (msg->FindString("name", &name) != B_OK || 
		msg->FindString("file", &file) != B_OK) {
		return;
	}
	DisplayCIDFonts::Type type = GetType(file.String());

	// save new font to global settings
	mDisplayCIDFonts->Set(name.String(), file.String(), type);
	BMessage archive;
	mDisplayCIDFonts->Archive(archive);
	mSettings->SetDisplayCIDFonts(archive);

	// set menu field label
	void* pointer;
	if (mFontMenuFields.FindPointer(name.String(), &pointer) != B_OK) {
		return;
	}
	
	BPath path(file.String());
	BMenuField* menuField = (BMenuField*)pointer;
	menuField->MenuItem()->SetLabel(path.Leaf());
}

void PreferencesWindow::MessageReceived(BMessage *msg) {
	void *ptr;
	int32 index, which;
	bool display;
	
	switch (msg->what) {
	case PREFERENCE_SELECTED:
		if (mPreferences->FullListCurrentSelection() >= 0) {
			mLayers->ActivateLayer(mPreferences->FullListCurrentSelection());
		}
		break;
	case LANGUAGE_SELECTED:
		if ((B_OK == msg->FindInt32("index", &index)) && (index >= 0)) {
			BStringItem *item = (BStringItem*)mList->ItemAt(index);
			if (item) {
				BString name(item->Text());
				name << ".catalog";
				gApp->GetSettings()->SetLanguage(name.String());
				gApp->SaveSettings();
			}
		}
		break;
	case RESTORE_PAGE_NO_CHANGED: mSettings->SetRestorePageNumber(IsOn(msg));
		break;
	case RESTORE_WINDOW_FRAME_CHANGED: mSettings->SetRestoreWindowFrame(IsOn(msg));
		break;
	case QUASI_FULLSCREEN_MODE_CHANGED: 
		if (B_OK == msg->FindInt32("index", &index)) {
			gApp->GetSettings()->SetQuasiFullscreenMode(index == 1 ? true : false);
		}
		break;
	case WORKSPACE_CHANGED: {
		int32 index;
		if (B_OK == msg->FindInt32("index", &index)) {
			if (index == 1) index = 0;
			bool enabled = index != 0;
			gApp->GetSettings()->SetOpenInWorkspace(enabled);
			if (enabled) gApp->GetSettings()->SetWorkspace(index - 1);
			UpdateWorkspace();
		}
		}
		break;
	case AUTHOR_CHANGED: {
		void* p;
			if (msg->FindPointer("source", &p) == B_OK) {
				BTextControl* t = (BTextControl*)p;
				gApp->GetSettings()->SetAuthor(t->Text());
			}
		}
		break;
	case INVERT_VERTICAL_SCROLLING_CHANGED: 
		mSettings->SetInvertVerticalScrolling(IsOn(msg));
		Notify(UPDATE_NOTIFY);
		break;
	case DISPLAY_CID_FONT_SELECTED:
		DisplayCIDFontSelected(msg);
		break;
	case HINTING_CHANGED:
		mSettings->SetHinting(IsOn(msg));
		globalParams->setFreeTypeHinting(
			(char*)(IsOn(msg) ? "yes" : "no"));
		NotifyRestartDoc();
		break;
	default:
		if ((B_OK == msg->FindPointer("radio", &ptr)) 
			&& (B_OK == msg->FindInt32("index", &index))
			&& FindRadioGroup((MRadioGroup*)ptr, display, which)) {
	
			BMessage msg(CHANGE_NOTIFY);
			msg.AddInt16("kind", display ? 0 : 1);
			msg.AddInt16("which", which);

			msg.AddInt16("index", index);
			mLooper->PostMessage(&msg);
		}
	}
	
	MWindow::MessageReceived(msg);
}

bool PreferencesWindow::QuitRequested() {
	if (mLooper) {
		BMessage msg(QUIT_NOTIFY);
		mLooper->PostMessage(&msg);
	}
	ClearView();
	return true;
}

void PreferencesWindow::FrameMoved(BPoint p) {
	mSettings->SetPrefsWindowPosition(p);
	MWindow::FrameMoved(p);
}

void PreferencesWindow::FrameResized(float w, float h) {
	mSettings->SetPrefsWindowSize(w, h);
	MWindow::FrameResized(w, h);
}
