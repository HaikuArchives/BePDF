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

// xpdf
#include <GlobalParams.h>

#include <locale/Catalog.h>
#include <Box.h>
#include <CheckBox.h>
#include <Directory.h>
#include <Entry.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ObjectList.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <StringView.h>
#include <String.h>
#include "BepdfApplication.h"
#include "LayoutUtils.h"
#include "PreferencesWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AnnotationWindow"

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
	m->AddItem(new BMenuItem(B_TRANSLATE("Current"), new BMessage(WORKSPACE_CHANGED)));
	m->AddSeparatorItem();
	int n = count_workspaces();

    BString buffer;
	for (int i = 1; i <= n; i ++) {
        buffer.SetToFormat("%d", i);
		m->AddItem(new BMenuItem(buffer.String(), new BMessage(WORKSPACE_CHANGED)));
	}
	m->SetLabelFromMarked(true);
	m->SetRadioMode(true);
}

void PreferencesWindow::SetupView() {
	char workspace[3];

	GlobalSettings* settings = gApp->GetSettings();
	sprintf(workspace, "%d", (int)settings->GetWorkspace());
	mPreferences = new BOutlineListView("mPreferences");
	BScrollView* prefScroll = new BScrollView("SV", mPreferences,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);

	BListItem *item;
	mPreferences->AddItem(new BStringItem(B_TRANSLATE("Document")));

	mPreferences->AddItem(item = new BStringItem(B_TRANSLATE("Display")));
		// reverse order:
		mPreferences->AddUnder(new BStringItem(B_TRANSLATE("Asian fonts")), item);

	mPreferences->SetSelectionMessage(new BMessage(PREFERENCE_SELECTED));
	mPreferences->SetExplicitMinSize(BSize(200, 0));

	BCheckBox *pageNumber = new BCheckBox("pageNumber",
		B_TRANSLATE("Restore page number"), new BMessage(RESTORE_PAGE_NO_CHANGED));
	pageNumber->SetValue(settings->GetRestorePageNumber());

	BCheckBox *windowPos = new BCheckBox("windowPos",
		B_TRANSLATE("Restore window position and size"),
		new BMessage(RESTORE_WINDOW_FRAME_CHANGED));
	windowPos->SetValue(settings->GetRestoreWindowFrame());

	BPopUpMenu *openMenu = new BPopUpMenu("openMenu");
	mOpenInWorkspace = new BMenuField("mOpeninWorkspace",
		B_TRANSLATE("Open in workspace:"), openMenu);

	BTextControl *author = new BTextControl("author", B_TRANSLATE("Author"),
		settings->GetAuthor(), new BMessage(AUTHOR_CHANGED));

	BGroupLayout *docBox = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.SetInsets(B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, 0)
		.Add(pageNumber)
		.Add(windowPos)
		.AddStrut(B_USE_ITEM_INSETS)
		.Add(mOpenInWorkspace)
		.AddStrut(B_USE_SMALL_INSETS)
		.Add(author)
		.AddGlue();

	BBox *document = new BBox("document");
	document->SetLabel("Document");
	document->AddChild(docBox->View());

	BRadioButton *docOnly = new BRadioButton("docOnly",
		B_TRANSLATE("Show document view only"),
		new BMessage(QUASI_FULLSCREEN_MODE_OFF));
	docOnly->SetValue(!(settings->GetQuasiFullscreenMode()));

	BRadioButton *docMore = new BRadioButton("docMore",
		B_TRANSLATE("Show toolbar, statusbar and scrollbars, too"),
		new BMessage(QUASI_FULLSCREEN_MODE_ON));
	docMore->SetValue(settings->GetQuasiFullscreenMode());

	BGroupLayout *fsBox = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.SetInsets(B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, 0)
		.Add(docOnly)
		.Add(docMore)
		.AddStrut(B_USE_SMALL_INSETS);

	BBox *fullscreen = new BBox("fullscreen");
	fullscreen->SetLabel(B_TRANSLATE("Fullscreen mode"));
	fullscreen->AddChild(fsBox->View());

	BRadioButton *filledRect = new BRadioButton("filledRect",
		B_TRANSLATE("Filled rectangle"),
		new BMessage(FILLED_SELECTION_FILLED));
	filledRect->SetValue(settings->GetFilledSelection());

	BRadioButton *strokedRect = new BRadioButton("strokedRect",
		B_TRANSLATE("Stroked rectangle"),
		new BMessage(FILLED_SELECTION_STROKED));
	strokedRect->SetValue(!(settings->GetFilledSelection()));

	BGroupLayout *rectBox = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.SetInsets(B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, 0)
		.Add(filledRect)
		.Add(strokedRect)
		.AddStrut(B_USE_SMALL_INSETS);

	BBox *selection = new BBox("selection");
	selection->SetLabel(B_TRANSLATE("Selection rectangle"));
	selection->AddChild(rectBox->View());

	BCheckBox *scrolling = new BCheckBox("scrolling",
		B_TRANSLATE("Invert vertical mouse scrolling"),
		new BMessage(INVERT_VERTICAL_SCROLLING_CHANGED));
	scrolling->SetValue(settings->GetInvertVerticalScrolling());

	BBox *asian = new BBox("asian");
	asian->SetLabel(B_TRANSLATE("Asian fonts"));
	asian->AddChild(BuildAsianFontsView());

	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(prefScroll)
		.AddCards()
			.Add(document)
			.AddGroup(B_VERTICAL)
				.Add(fullscreen)
				.Add(selection)
				.Add(scrolling)
				.AddGlue()
			.End()
			.Add(asian)
			.GetLayout(&mLayers)
		.End();

	author->SetModificationMessage(new BMessage(AUTHOR_CHANGED));

	mLayers->SetVisibleItem((int32)1);
	mLayers->SetVisibleItem((int32)0);
	mPreferences->Select(0);

	BuildWorkspaceMenu(mOpenInWorkspace->Menu());
	UpdateWorkspace();
	ResizeTo(0, 0);
	CenterOnScreen();
}

static int comparePopupLabels(const BMenuField* p1,
	const BMenuField* p2) {
	return strcmp(p1->Label(), p2->Label());
}

BView* PreferencesWindow::BuildAsianFontsView() {
	if (mDisplayCIDFonts->GetSize() == 0) {
		return new BView("languages", 0);
	}

	BObjectList<BMenuField> menus;
	for (int32 index = 0; index < mDisplayCIDFonts->GetSize(); index ++) {
		BString name;
		BString file;
		DisplayCIDFonts::Type type;
		mDisplayCIDFonts->Get(index, name, file, type);

		BString label(B_TRANSLATE(name.String()));
		label << ":";
		BPopUpMenu *popupInner = new BPopUpMenu("");
		BMenuField *popup = new BMenuField("popup", label.String(), popupInner);
		FillFontFileMenu(popup, name.String(), file.String());

		menus.AddItem(popup);
	}

	// sort MPopup alphabetically by label
	menus.SortItems(comparePopupLabels);

	BStringView *restart = new BStringView("restart",
		B_TRANSLATE("Will take effect at next program launch."));

	// add popup menus into vertical group
	BGridLayout *grid = new BGridLayout();
	BView *ret = new BView("ret" , 0);
	BLayoutBuilder::Group<>(ret, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS)
		.Add(grid)
		.AddGlue()
		.Add(restart);

	for (int32 index = 0; index < menus.CountItems(); index ++) {
		BMenuField *item = menus.ItemAt(index);
		grid->AddItem(item->CreateLabelLayoutItem(), 0, index);
		grid->AddItem(item->CreateMenuBarLayoutItem(), 1, index);
	}

	return ret;
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
	FillFontFileMenu(menuField, B_SYSTEM_FONTS_DIRECTORY, B_TRANSLATE("System fonts"), name, file);
	FillFontFileMenu(menuField, B_USER_FONTS_DIRECTORY, B_TRANSLATE("User fonts"), name, file);
	FillFontFileMenu(menuField, B_SYSTEM_NONPACKAGED_FONTS_DIRECTORY, B_TRANSLATE("System fonts"), name, file);
	FillFontFileMenu(menuField, B_USER_NONPACKAGED_FONTS_DIRECTORY, B_TRANSLATE("User fonts"), name, file);

	// B_USER_FONTS_DIRECTORY is same as B_COMMON_FONTS_DIRECTORY in
	// BeOS R5
	// FillFontFileMenu(menu, B_USER_FONTS_DIRECTORY, B_TRANSLATE("User fonts"), name, file);
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

void PreferencesWindow::ClearView()
{
}

PreferencesWindow::PreferencesWindow(GlobalSettings *settings, BLooper *looper)
	: BWindow(BRect(0, 0, 100, 100)
		, B_TRANSLATE("Preferences")
		, B_TITLED_WINDOW_LOOK,
		B_FLOATING_APP_WINDOW_FEEL,
		B_AUTO_UPDATE_SIZE_LIMITS)
	, mLooper(looper)
	, mSettings(settings)
{
	BMessage msg;
	settings->GetDisplayCIDFonts(msg);
	mDisplayCIDFonts = new DisplayCIDFonts(msg);

	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));

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
	switch (msg->what) {
	case PREFERENCE_SELECTED:
		if (mPreferences->FullListCurrentSelection() >= 0) {
			mLayers->SetVisibleItem(mPreferences->FullListCurrentSelection());
		}
		break;
	case RESTORE_PAGE_NO_CHANGED: mSettings->SetRestorePageNumber(IsOn(msg));
		break;
	case RESTORE_WINDOW_FRAME_CHANGED: mSettings->SetRestoreWindowFrame(IsOn(msg));
		break;
	case QUASI_FULLSCREEN_MODE_ON:
		gApp->GetSettings()->SetQuasiFullscreenMode(true);
		break;
	case QUASI_FULLSCREEN_MODE_OFF:
		gApp->GetSettings()->SetQuasiFullscreenMode(false);
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
	case FILLED_SELECTION_FILLED:
	case FILLED_SELECTION_STROKED: {
		BMessage nmsg(CHANGE_NOTIFY);
		nmsg.AddInt16("kind", DISPLAY);
		nmsg.AddInt16("which", DISPLAY_FILLED_SELECTION);

		nmsg.AddInt16("index", msg->what == FILLED_SELECTION_FILLED ? 0 : 1);
		mLooper->PostMessage(&nmsg);
		}
		break;
	}

	BWindow::MessageReceived(msg);
}

bool PreferencesWindow::QuitRequested() {
	if (mLooper) {
		BMessage msg(QUIT_NOTIFY);
		mLooper->PostMessage(&msg);
	}
	ClearView();
	return true;
}
