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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// xpdf
#include <Object.h>
#include <Gfx.h>

// BeOS
#include <locale/Catalog.h>
#include <be/app/Roster.h>
#include <be/app/MessageQueue.h>
#include <be/interface/Alert.h>
#include <be/interface/Bitmap.h>
#include <be/interface/Button.h>
#include <be/interface/MenuBar.h>
#include <be/interface/MenuItem.h>
#include <be/interface/MenuField.h>
#include <be/interface/Screen.h>
#include <be/interface/ScrollView.h>
#include <be/interface/ScrollBar.h>
#include <be/interface/StringView.h>
#include <be/interface/TabView.h>
#include <be/interface/View.h>
#include <be/storage/Path.h>
#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <be/storage/NodeMonitor.h>
#include <be/support/Beep.h>
#include <be/support/Debug.h>

// BePDF
#include "AnnotationWindow.h"
#include "AnnotWriter.h"
#include "AttachmentView.h"
#include "BePDF.h"
#include "BepdfApplication.h"
#include "EntryMenuItem.h"
#include "FileInfoWindow.h"
#include "FindTextWindow.h"
#include "LayerView.h"
#include "LayoutUtils.h"
#include "OutlinesWindow.h"
#include "PageLabels.h"
#include "PageRenderer.h"
#include "PasswordWindow.h"
#include "PDFView.h"
#include "PDFWindow.h"
#include "PreferencesWindow.h"
#include "PrintSettingsWindow.h"
#include "ResourceLoader.h"
#include "SaveThread.h"
#include "SplitView.h"
#include "StatusBar.h"
#include "TraceWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PDFWindow"


char * PDFWindow::PAGE_MSG_LABEL = "page";

#define PATH_HELP        "docs/help.html"
#define PATH_PDF_HELP    "docs/BePDF.pdf"

#define FIRST_ZOOM_ITEM_INDEX 0

// Implementation of RecentDocumentsMenu

RecentDocumentsMenu::RecentDocumentsMenu(const char *title, uint32 what, menu_layout layout)
	: BMenu(title, layout)
	, fWhat(what)
{
}

bool
RecentDocumentsMenu::AddDynamicItem(add_state s)
{
	if (s != B_INITIAL_ADD) return false;

	BMenuItem *item;
	BMessage list, *msg;
	entry_ref ref;
	char name[B_FILE_NAME_LENGTH];

	while ((item = RemoveItem((int32)0)) != NULL) {
		delete item;
	}

	be_roster->GetRecentDocuments(&list, 20, "application/pdf", NULL);
	for (int i = 0; list.FindRef("refs", i, &ref) == B_OK; i++) {
		BEntry entry(&ref);
		if (entry.Exists() && entry.GetName(name) == B_OK) {
			msg = new BMessage(fWhat);
			msg->AddRef("refs", &ref);
			item =  new EntryMenuItem(&ref, name, msg, 0, 0);
			AddItem(item);
			if (fWhat == B_REFS_RECEIVED) {
				item->SetTarget(be_app, NULL);
			}
		}
	}

	return false;
}




///////////////////////////////////////////////////////////
/*
	Check errors that may happen
*/
PDFWindow::PDFWindow(entry_ref * ref, BRect frame, bool quitWhenClosed, const char *ownerPassword, const char *userPassword, bool *encrypted)
	: HWindow(frame, "PDF", B_DOCUMENT_WINDOW, 0, quitWhenClosed)
{
	mMainView = NULL;
	mPagesView = NULL;
	mAttachmentView = NULL;
	mPageNumberItem = NULL;
	mPrintSettings = NULL;
	mTotalPageNumberItem = NULL;
	mStatusText = NULL;
	mFindWindow = NULL;
	mPreferencesItem = NULL;
	mFileInfoItem =  NULL;
	mFindInProgress = false;

	mZoomMenu = mRotationMenu = NULL;
	mLayerView = NULL;

	mOWMessenger = NULL;
	mFIWMessenger = NULL;
	mPSWMessenger = NULL;
	mAWMessenger = NULL;

	mPrintSettingsWindowOpen = false;

	mShowLeftPanel = true;
	mFullScreen = false;

	mPendingMask = 0;

	mPressedAnnotationButton = NULL;

	AddHandler(&mEntryChangedMonitor);
	mEntryChangedMonitor.SetEntryChangedListener(this);

	InitAnnotTemplates();

	SetUpViews(ref, ownerPassword, userPassword, encrypted);

	GlobalSettings *settings = gApp->GetSettings();
	int32 ws = settings->GetWorkspace();
	if (settings->GetOpenInWorkspace() && ws >= 1 && ws <= count_workspaces()) {
		SetWorkspaces(1 << (ws - 1));
	}
	mCurrentWorkspace = Workspaces();

	if (mMainView != NULL) {
		mMainView->Redraw();
		InitAfterOpen();
	}
}



///////////////////////////////////////////////////////////
PDFWindow::~PDFWindow()
{
	RemoveHandler(&mEntryChangedMonitor);

	DeleteAnnotTemplates();
	if (mPagesView) {
		MakeEmpty(mPagesView);
	}
}

void PDFWindow::SetTotalPageNumber(int pages) {
	const char *fmt = B_TRANSLATE("of %d");
	int len = strlen(fmt) + 30;
	char *label = new char[len];
	snprintf (label, len, fmt, pages);
	mTotalPageNumberItem->SetText(label);
	delete label;
}

void PDFWindow::InitAfterOpen() {
	GlobalSettings *s = gApp->GetSettings();
	if (Lock()) {
		// set page number text
		SetTotalPageNumber(mMainView->GetNumPages());

		// set window frame
		if (s->GetRestoreWindowFrame()) {
			float left, top;
			mFileAttributes.GetLeftTop(left, top);
			mMainView->ScrollTo(left, top);
		}

		// set page number list
		if (gPdfLock->LockWithTimeout(0) == B_OK) {
			UpdatePageList();
			gPdfLock->Unlock();
		} else {
			FillPageList();
			SetPending(UPDATE_PAGE_LIST_PENDING);
		}
		// select page number
		mPagesView->Select(mFileAttributes.GetPage()-1);
		mPagesView->ScrollToSelection();
		if (s->GetRestorePageNumber()) {
			SetZoom(s->GetZoom());
			SetRotation(s->GetRotation());
		}
		Unlock();
	}
}


void PDFWindow::FillPageList() {
	BList list;
	for (int32 i = 0; i < mMainView->GetNumPages(); i++) {
		char pageNo[20];
		sprintf(pageNo, "%5.0d", (int)(i+1));
		list.AddItem(new BStringItem(pageNo));
	}

	MakeEmpty(mPagesView);
	mPagesView->AddList(&list);
	// clear attachments
	mAttachmentView->Empty();
}


void PDFWindow::UpdatePageList() {
	gPdfLock->Lock();
	PageLabels labels(mMainView->GetNumPages()-1);
	Object catDict;
	mMainView->GetPDFDoc()->getXRef()->getCatalog(&catDict);
	Object* pageLabels = new Object;
	catDict.dictLookup("PageLabels", pageLabels);
	if (labels.Parse(pageLabels)) {
		labels.Replace(mPagesView);
	}

	// update attachments as well
	mAttachmentView->Fill(mMainView->GetPDFDoc()->getXRef(),
		mMainView->GetPDFDoc());

	gPdfLock->Unlock();
}

bool PDFWindow::SetPendingIfLocked(uint32 mask) {
	if (gPdfLock->LockWithTimeout(0) == B_OK) {
		gPdfLock->Unlock();
		return false;
	} else {
		// could not lock, schedule action later
		SetPending(mask);
		return true;
	}
}

void PDFWindow::HandlePendingActions(bool ok) {
	if (IsPending(UPDATE_PAGE_LIST_PENDING))
		UpdatePageList();

	if (ok) {
		if (IsPending(UPDATE_OUTLINE_LIST_PENDING)) MessageReceived(&BMessage(SHOW_BOOKMARKS_CMD));
		if (IsPending(FILE_INFO_PENDING))           MessageReceived(&BMessage(FILE_INFO_CMD));
		if (IsPending(PRINT_SETTINGS_PENDING))      MessageReceived(&BMessage(PRINT_SETTINGS_CMD));
	}
	ClearPending();
}

///////////////////////////////////////////////////////////
void PDFWindow::StoreFileAttributes() {
	// store file settings
	if (mMainView && Lock()) {
		entry_ref cur_ref;
		if (mCurrentFile.InitCheck() == B_OK) {
			mCurrentFile.GetRef(&cur_ref);
			BMessage bm;
			if (mOutlinesView->GetBookmarks(&bm))
				mFileAttributes.SetBookmarks(&bm);
			mFileAttributes.Write(&cur_ref, gApp->GetSettings());
		}
		Unlock();
	}
}

///////////////////////////////////////////////////////////
bool PDFWindow::QuitRequested() {
	gApp->WindowClosed();
	mMainView->WaitForPage(true);
	StoreFileAttributes();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

///////////////////////////////////////////////////////////
void PDFWindow::CleanUpBeforeLoad() {
	EditAnnotation(false);
}


///////////////////////////////////////////////////////////
bool PDFWindow::IsCurrentFile(entry_ref *ref) const {
	entry_ref r;
	mCurrentFile.GetRef(&r);
	return r == *ref;
}

///////////////////////////////////////////////////////////
bool PDFWindow::LoadFile(entry_ref *ref, const char *ownerPassword, const char *userPassword, bool *encrypted) {
	if (mMainView != NULL) {
		StoreFileAttributes();
		CleanUpBeforeLoad();
		// load new file
		if (mMainView->LoadFile(ref, &mFileAttributes, ownerPassword, userPassword, false, encrypted)) {
			mEntryChangedMonitor.StartWatching(ref);
			be_roster->AddToRecentDocuments(ref, BEPDF_APP_SIG);
			mCurrentFile.SetTo(ref);
			InitAfterOpen();
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////
void PDFWindow::Reload(void) {
    BMessage m(B_REFS_RECEIVED);
    entry_ref ref;
    mCurrentFile.GetRef(&ref);
    m.AddRef("refs", &ref);
	be_app->PostMessage(&m);
}

///////////////////////////////////////////////////////////
void PDFWindow::EntryChanged() {
	Reload();
}

///////////////////////////////////////////////////////////
bool PDFWindow::CancelCommand(BMessage* msg) {
	// This is a work around:
	// This commands aren't allowed in fullscreen mode, otherwise
	// the windows opened by this commands would be behind the
	// main window and would not block the main window.
	if (mFullScreen) {
		switch(msg->what) {
			case OPEN_FILE_CMD:
			case RELOAD_FILE_CMD:
			case PAGESETUP_FILE_CMD:
			case ABOUT_APP_CMD:
			case FIND_CMD:
			case FIND_NEXT_CMD:
			case PREFERENCES_FILE_CMD:
			case FILE_INFO_CMD:
			case PRINT_SETTINGS_CMD:
				beep();
				return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////
bool PDFWindow::ActivateWindow(BMessenger *messenger) {
	if (messenger && messenger->LockTarget()) {
		BLooper *looper;
		messenger->Target(&looper);
		((BWindow*)looper)->Activate(true);
		looper->Unlock();
		return true;
	} else {
		return false;
	}
}

///////////////////////////////////////////////////////////
bool PDFWindow::CanClose()
{
	return true;
}

///////////////////////////////////////////////////////////
AnnotationWindow* PDFWindow::GetAnnotationWindow() {
	if (mAWMessenger && mAWMessenger->LockTarget()) {
		BLooper *looper;
		mAWMessenger->Target(&looper);
		return (AnnotationWindow*)looper;
	} else {
		return NULL;
	}
}

///////////////////////////////////////////////////////////
AnnotationWindow* PDFWindow::ShowAnnotationWindow() {
	AnnotationWindow* w = GetAnnotationWindow();
	if (!w) {
		delete mAWMessenger;
		w = new AnnotationWindow(gApp->GetSettings(), this);
		mAWMessenger = new BMessenger(w);
		w->Lock();
	}
	return w;
}

///////////////////////////////////////////////////////////
void PDFWindow::UpdateInputEnabler() {
	#define ie mInputEnabler
	if (mMainView) {
		PDFDoc* doc = mMainView->GetPDFDoc();
		int num_pages = mMainView->GetNumPages();
		int page = mMainView->Page();
		bool b = num_pages > 1 && page != 1;

		ie.SetEnabled(FIRST_PAGE_CMD,      b);
		ie.SetEnabled(PREVIOUS_PAGE_CMD,   b);
		ie.SetEnabled(PREVIOUS_N_PAGE_CMD, b);

		b = num_pages > 1 && page != num_pages;
		ie.SetEnabled(LAST_PAGE_CMD,       b);
		ie.SetEnabled(NEXT_PAGE_CMD,       b);
		ie.SetEnabled(NEXT_N_PAGE_CMD,     b);

		ie.SetEnabled(GOTO_PAGE_CMD,   num_pages > 1);

		ie.SetEnabled(HISTORY_FORWARD_CMD, mMainView->CanGoForward());
		ie.SetEnabled(HISTORY_BACK_CMD, mMainView->CanGoBack());

		int32 dpi = mMainView->GetZoomDPI();
		ie.SetEnabled(ZOOM_IN_CMD,  dpi != ZOOM_DPI_MAX);
		ie.SetEnabled(ZOOM_OUT_CMD, dpi != ZOOM_DPI_MIN);

		ie.SetEnabled(FIND_NEXT_CMD, mFindText.Length() > 0);

		int active = mLayerView->Active();
		// setenable state of
		mToolBar->SetActionPressed(SHOW_PAGE_LIST_CMD, mShowLeftPanel && active == PAGE_LIST_PANEL);
		mToolBar->SetActionPressed(SHOW_BOOKMARKS_CMD, mShowLeftPanel && active == BOOKMARKS_PANEL);
		mToolBar->SetActionPressed(SHOW_ANNOT_TOOLBAR_CMD, mShowLeftPanel && active == ANNOTATIONS_PANEL);
		mToolBar->SetActionPressed(SHOW_ATTACHMENTS_CMD, mShowLeftPanel && active == ATTACHMENTS_PANEL);
		mToolBar->SetActionPressed(FULL_SCREEN_CMD, mFullScreen);

		// set enable state
		ie.SetEnabled(SHOW_PAGE_LIST_CMD, (!mShowLeftPanel) || active != PAGE_LIST_PANEL);
		ie.SetEnabled(SHOW_BOOKMARKS_CMD, (!mShowLeftPanel) || active != BOOKMARKS_PANEL);
		ie.SetEnabled(SHOW_ANNOT_TOOLBAR_CMD, (!mShowLeftPanel) || active != ANNOTATIONS_PANEL);
		ie.SetEnabled(SHOW_ATTACHMENTS_CMD, (!mShowLeftPanel) || active != ATTACHMENTS_PANEL);
		ie.SetEnabled(HIDE_LEFT_PANEL_CMD, mShowLeftPanel);

		ie.SetEnabled(OPEN_FILE_CMD,      !mFullScreen);
		ie.SetEnabled(RELOAD_FILE_CMD,    !mFullScreen);
		ie.SetEnabled(PRINT_SETTINGS_CMD, !mFullScreen && !mPrintSettingsWindowOpen && doc->okToPrint());

		// PDF security settings
		bool okToCopy = doc->okToCopy();
		ie.SetEnabled(COPY_SELECTION_CMD, okToCopy);
		ie.SetEnabled(SELECT_ALL_CMD,     okToCopy);
		ie.SetEnabled(SELECT_NONE_CMD,    okToCopy);

		bool hasBookmark = mOutlinesView->HasUserBookmark(page);
		bool selected    = hasBookmark && mOutlinesView->IsUserBMSelected();
		ie.SetEnabled(ADD_BOOKMARK_CMD,   !hasBookmark);
		ie.SetEnabled(EDIT_BOOKMARK_CMD,   selected);
		ie.SetEnabled(DELETE_BOOKMARK_CMD, selected);

		// Annotation
		bool editAnnot = mMainView->EditingAnnot();

		ie.SetEnabled(DONE_EDIT_ANNOT_CMD, editAnnot);

		bool canCopy = (!editAnnot) && mMainView->HasSelection();
		ie.SetEnabled(SELECT_ALL_CMD,     !editAnnot);
		ie.SetEnabled(COPY_SELECTION_CMD, canCopy);
		ie.SetEnabled(SELECT_NONE_CMD,    canCopy);


		if (Lock()) {
			ie.Update();
			Unlock();
		}
	}
	#undef ie
}

///////////////////////////////////////////////////////////
void PDFWindow::AddItem(BMenu *subMenu, const char *label, uint32 cmd, bool marked, char shortcut, uint32 modifiers) {
	BMenuItem *item = new BMenuItem(label, new BMessage(cmd), shortcut, modifiers);
	item->SetMarked(marked);
	subMenu->AddItem(item);
	mInputEnabler.Register(new IEMenuItem(item, cmd));
}

///////////////////////////////////////////////////////////
void PDFWindow::Register(uint32 behavior, BControl* control, int32 cmd) {
	if (behavior == B_ONE_STATE_BUTTON) {
		mInputEnabler.Register(new IEControl(control, cmd));
	} // else {
		// behavior == B_TWO_STATE_BUTTON
//		mControlValueSetter.Register(new IEControlValue(control, cmd));
//	}
}

///////////////////////////////////////////////////////////
void PDFWindow::UpdateWindowsMenu() {
/*
	BMenuItem *item;
	while ((item = mWindowsMenu->RemoveItem((int32)0)) != NULL) delete item;
	BList list;
	be_roster->GetAppList(BEPDF_APP_SIG, &list);
	entry_ref ref;
	const int n = list.CountItems();

	for (int i = n-1; i >= 0; i --) {
		team_id who = (team_id)list.ItemAt(i);
		char s[256];
		sprintf(s, "BePDF %d", who);
		mWindowsMenu->AddItem(new BMenuItem(s, NULL));
	}
*/
}

///////////////////////////////////////////////////////////
BMenuBar* PDFWindow::BuildMenu() {
	BString label;
	GlobalSettings* settings = gApp->GetSettings();

	BMenuBar * menuBar = new BMenuBar(BRect(0, 0, 1024, 18), "mainBar");
		// File
		BMenu * menu = new BMenu(B_TRANSLATE("File"));
			menu->AddItem(mOpenMenu = new RecentDocumentsMenu(B_TRANSLATE("Open" B_UTF8_ELLIPSIS),  B_REFS_RECEIVED));
			menu->AddItem(mNewMenu  = new RecentDocumentsMenu(B_TRANSLATE("Open in new window" B_UTF8_ELLIPSIS), OPEN_IN_NEW_WINDOW_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Reload"), 'R', (RELOAD_FILE_CMD));
			AddItem(menu, B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), SAVE_FILE_AS_CMD, false, 'S', B_SHIFT_KEY);

			mFileInfoItem = new BMenuItem(B_TRANSLATE("File info" B_UTF8_ELLIPSIS),
				new BMessage(FILE_INFO_CMD), 'I');
			menu->AddItem(mFileInfoItem);

			ADD_SITEM (menu );
			ADD_ITEM (menu, B_TRANSLATE("Page setup" B_UTF8_ELLIPSIS), 'S',  (PAGESETUP_FILE_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Print" B_UTF8_ELLIPSIS), 'P',  (PRINT_SETTINGS_CMD));

			ADD_SITEM (menu );

			ADD_ITEM ( menu, B_TRANSLATE("Close"), 'W', ( CLOSE_FILE_CMD ) );
			ADD_ITEM ( menu, B_TRANSLATE("Quit"), 'Q' , ( QUIT_APP_CMD ));

		menuBar->AddItem(menu);

		// Edit
		menu = new BMenu ( B_TRANSLATE("Edit") );
			ADD_ITEM (menu, B_TRANSLATE("Copy selection"), 'C', ( COPY_SELECTION_CMD));
			ADD_SITEM (menu);
			ADD_ITEM (menu, B_TRANSLATE("Select all"), 'A', ( SELECT_ALL_CMD));
			AddItem(menu, B_TRANSLATE("Select none"), SELECT_NONE_CMD, false, 'A', B_SHIFT_KEY);

			ADD_SITEM (menu );

			mPreferencesItem = new BMenuItem(B_TRANSLATE("Preferences" B_UTF8_ELLIPSIS),
				new BMessage(PREFERENCES_FILE_CMD), 'P', B_SHIFT_KEY);
			menu->AddItem(mPreferencesItem);
		menuBar->AddItem ( menu );

		// Zoom
		menu = new BMenu ( B_TRANSLATE("View") );
		int16 zoom = settings->GetZoom();
			ADD_ITEM(menu, B_TRANSLATE("Bookmarks"), 'B' , (SHOW_BOOKMARKS_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Show page list"), 'L', (SHOW_PAGE_LIST_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Show annotation tool bar"), 0, (SHOW_ANNOT_TOOLBAR_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Show attachments"), 0, (SHOW_ATTACHMENTS_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Hide page list"), 'H', (HIDE_LEFT_PANEL_CMD));

			ADD_SITEM(menu);

			menu->AddItem(mFullScreenItem =
				new BMenuItem(B_TRANSLATE("Fullscreen"),
					new BMessage(FULL_SCREEN_CMD), B_RETURN));

			ADD_SITEM(menu);

			ADD_ITEM(menu, B_TRANSLATE("Fit to page width"), '/', (FIT_TO_PAGE_WIDTH_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Fit to page"), '*', (FIT_TO_PAGE_CMD));
			ADD_SITEM(menu);
			ADD_ITEM(menu, B_TRANSLATE("Zoom in"), '+', (ZOOM_IN_CMD));
			ADD_ITEM(menu, B_TRANSLATE("Zoom out"), '-', (ZOOM_OUT_CMD));

			ADD_SITEM(menu);

			mZoomMenu = new BMenu (B_TRANSLATE("Zoom"));
			mZoomMenu->SetRadioMode (true);

			AddItem(mZoomMenu, "25%", SET_ZOOM_VALUE_CMD, MIN_ZOOM == zoom);
			AddItem(mZoomMenu, "33%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 1 == zoom);
			AddItem(mZoomMenu, "50%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 2 == zoom);
			AddItem(mZoomMenu, "66%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 3 == zoom);
			AddItem(mZoomMenu, "75%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 4 == zoom);
			AddItem(mZoomMenu, "100%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 5 == zoom);
			AddItem(mZoomMenu, "125%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 6 == zoom);
			AddItem(mZoomMenu, "150%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 7 == zoom);
			AddItem(mZoomMenu, "175%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 8 == zoom);
			AddItem(mZoomMenu, "200%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 9 == zoom);
			AddItem(mZoomMenu, "300%", SET_ZOOM_VALUE_CMD, MIN_ZOOM + 10 == zoom);

			if (zoom < MIN_ZOOM) SetZoom(zoom);
			menu->AddItem (mZoomMenu);

			ADD_SITEM(menu);

			mRotationMenu = new BMenu (B_TRANSLATE("Rotation"));
			mRotationMenu->SetRadioMode(true);
			float rotation = settings->GetRotation();
				AddItem(mRotationMenu, "0°", SET_ROTATE_VALUE_CMD, rotation == 0);
				AddItem(mRotationMenu, "90°", SET_ROTATE_VALUE_CMD, rotation == 90);
				AddItem(mRotationMenu, "180°", SET_ROTATE_VALUE_CMD, rotation == 180);
				AddItem(mRotationMenu, "270°", SET_ROTATE_VALUE_CMD, rotation == 270);
			menu->AddItem(mRotationMenu);

			ADD_SITEM(menu);
			ADD_ITEM(menu, B_TRANSLATE("Show error messages"), 'M', (SHOW_TRACER_CMD));
		menuBar->AddItem ( menu );

		// Search
		menu = new BMenu ( B_TRANSLATE("Search") );
			ADD_ITEM (menu, B_TRANSLATE("Find" B_UTF8_ELLIPSIS) , 'F', ( FIND_CMD));
			menu->AddItem(new BMenuItem(B_TRANSLATE("Find next" B_UTF8_ELLIPSIS),
				new BMessage(FIND_NEXT_CMD), 'F', B_SHIFT_KEY));
		menuBar->AddItem ( menu );

		// Page
		menu = new BMenu (B_TRANSLATE("Page"));
			ADD_ITEM (menu, B_TRANSLATE("First"), 0,  (FIRST_PAGE_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Previous"), 0,  (PREVIOUS_PAGE_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Go to page"), 'G',  (GOTO_PAGE_MENU_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Next"), 0,  (NEXT_PAGE_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Last"), 0,  (LAST_PAGE_CMD));
			ADD_SITEM(menu);
			ADD_ITEM (menu, B_TRANSLATE("Back"), B_LEFT_ARROW,  (HISTORY_BACK_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Forward"), B_RIGHT_ARROW,  (HISTORY_FORWARD_CMD));
		menuBar->AddItem (menu);

		// Bookmarks
		menu = new BMenu (B_TRANSLATE("Bookmark"));

			ADD_ITEM (menu, B_TRANSLATE("Add"),    0, ( ADD_BOOKMARK_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Delete"), 0, ( DELETE_BOOKMARK_CMD));
			ADD_ITEM (menu, B_TRANSLATE("Edit"),   0, ( EDIT_BOOKMARK_CMD));

		menuBar->AddItem(menu);

//		menuBar->AddItem ( mWindowsMenu = new BMenu(B_TRANSLATE("Window")) );
		UpdateWindowsMenu();

		menu = new BMenu(B_TRANSLATE("Help"));
		ADD_ITEM (menu, B_TRANSLATE("Show help" B_UTF8_ELLIPSIS), 0, (HELP_CMD));
		ADD_ITEM (menu, B_TRANSLATE("Online help" B_UTF8_ELLIPSIS), 0, (ONLINE_HELP_CMD));

		ADD_SITEM(menu);

		ADD_ITEM (menu, B_TRANSLATE("Visit homepage" B_UTF8_ELLIPSIS), 0, (HOME_PAGE_CMD));
		ADD_ITEM (menu, B_TRANSLATE("Issue tracker" B_UTF8_ELLIPSIS), 0, (BUG_REPORT_CMD));
		ADD_SITEM (menu );
		ADD_ITEM (menu, B_TRANSLATE("About BePDF" B_UTF8_ELLIPSIS), 0, ( ABOUT_APP_CMD ) );
		menuBar->AddItem( menu );

	AddChild(menuBar);

	mOpenMenu->Superitem()->SetTrigger('O');
	mOpenMenu->Superitem()->SetMessage(new BMessage(OPEN_FILE_CMD));
	mOpenMenu->Superitem()->SetShortcut('O', 0);

	mNewMenu->Superitem()->SetTrigger('N');
	mNewMenu->Superitem()->SetMessage(new BMessage(NEW_WINDOW_CMD));
	mNewMenu->Superitem()->SetShortcut('N', 0);

	return menuBar;
}


BToolBar* PDFWindow::BuildToolBar()
{
	mToolBar = new BToolBar(BRect(0, mMenuHeight, Bounds().right,
		mMenuHeight+TOOLBAR_HEIGHT-1));
	mToolBar->SetName("toolbar");
	mToolBar->SetResizingMode(B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
	mToolBar->SetFlags(B_WILL_DRAW | B_FRAME_EVENTS);
	AddChild(mToolBar);

	mToolBar->AddAction(OPEN_FILE_CMD, this, LoadBitmap("OPEN_FILE_ON"),
		B_TRANSLATE("Open file"));
	mToolBar->AddAction(RELOAD_FILE_CMD, this, LoadBitmap("RELOAD_FILE_ON"),
		B_TRANSLATE("Reload file"));
	mToolBar->AddAction(PRINT_SETTINGS_CMD, this, LoadBitmap("PRINT_ON"),
		B_TRANSLATE("Print"));

	mToolBar->AddSeparator();

	mToolBar->AddAction(SHOW_BOOKMARKS_CMD, this, LoadBitmap("BOOKMARKS_ON"),
		B_TRANSLATE("Bookmarks"), NULL, true);
	mToolBar->AddAction(SHOW_PAGE_LIST_CMD, this, LoadBitmap("PAGE_LIST_ON"),
		B_TRANSLATE("Show page list"), NULL, true);
	mToolBar->AddAction(SHOW_ANNOT_TOOLBAR_CMD, this,
		LoadBitmap("SHOW_ANNOT_ON"), B_TRANSLATE("Show annotation toolbar"),
		NULL, true);
	mToolBar->AddAction(SHOW_ATTACHMENTS_CMD, this,
		LoadBitmap("SHOW_ATTACHMENTS_ON"), B_TRANSLATE("Show attachments"),
		NULL, true);
	// mToolBar->AddAction(HIDE_LEFT_PANEL_CMD, this,
	//	LoadBitmap("HIDE_PAGE_LIST_ON"), B_TRANSLATE("Hide page list"),
	//	NULL, true);

	mToolBar->AddSeparator();

	mToolBar->AddAction(FULL_SCREEN_CMD, this,
		LoadBitmap("FULL_SCREEN_ON"), B_TRANSLATE("Fullscreen mode"),
		NULL, true);

	mToolBar->AddSeparator();

	mToolBar->AddAction(FIRST_PAGE_CMD, this, LoadBitmap("FIRST_ON"),
		B_TRANSLATE("Go to start of document"));
	mToolBar->AddAction(PREVIOUS_N_PAGE_CMD, this,
		LoadBitmap("PREVIOUS_N_ON"),
		B_TRANSLATE("Go back 10 pages"));
	mToolBar->AddAction(PREVIOUS_PAGE_CMD, this, LoadBitmap("PREVIOUS_ON"),
		B_TRANSLATE("Go to previous page"));
	mToolBar->AddAction(NEXT_PAGE_CMD, this, LoadBitmap("NEXT_ON"),
		B_TRANSLATE("Go to next page"));
	mToolBar->AddAction(NEXT_N_PAGE_CMD, this, LoadBitmap("NEXT_N_ON"),
		B_TRANSLATE("Go forward 10 pages"));
	mToolBar->AddAction(LAST_PAGE_CMD, this, LoadBitmap("LAST_ON"),
		B_TRANSLATE("Go to end of document"));

	mToolBar->AddSeparator();

	mToolBar->AddAction(HISTORY_BACK_CMD, this, LoadBitmap("BACK_ON"),
		B_TRANSLATE("Back in page history list"));
	mToolBar->AddAction(HISTORY_FORWARD_CMD, this, LoadBitmap("FORWARD_ON"),
		B_TRANSLATE("Forward in page history list"));

	mToolBar->AddSeparator();

	// Add "go to page number" TextControl
	mPageNumberItem	= new BTextControl(BRect (0, 6, 50, 30), "goto_page",
		"", "", new BMessage(GOTO_PAGE_CMD));
	mPageNumberItem->SetDivider (0.0);
	mPageNumberItem->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_RIGHT);
	mPageNumberItem->SetTarget (this);
	mPageNumberItem->TextView()->DisallowChar(B_ESCAPE);
	mInputEnabler.Register(new IETextView(mPageNumberItem->TextView(),
		GOTO_PAGE_CMD));

	BTextView *t = mPageNumberItem->TextView();
	BFont font(be_plain_font);
	t->GetFontAndColor(0, &font);
	font.SetSize(10);
	t->SetFontAndColor(0, 1000, &font, B_FONT_SIZE);
	mToolBar->AddView(mPageNumberItem);

	// display total number of pages
	mTotalPageNumberItem = new BStringView(BRect (0, 0, 50, 22),
		"total_num_of_pages", "");
	mTotalPageNumberItem->SetAlignment(B_ALIGN_CENTER);
	mTotalPageNumberItem->SetFont(&font);
	mToolBar->AddView(mTotalPageNumberItem);

	mToolBar->AddSeparator();

	mToolBar->AddAction(FIT_TO_PAGE_WIDTH_CMD, this,
		LoadBitmap("FIT_TO_PAGE_WIDTH_ON"),
		B_TRANSLATE("Fit to page width"));
	mToolBar->AddAction(FIT_TO_PAGE_CMD, this, LoadBitmap("FIT_TO_PAGE_ON"),
		B_TRANSLATE("Fit to page"));

	mToolBar->AddSeparator();

	mToolBar->AddAction(ROTATE_CLOCKWISE_CMD, this,
		LoadBitmap("ROTATE_CLOCKWISE_ON"), B_TRANSLATE("Rotate clockwise"));
	mToolBar->AddAction(ROTATE_ANTI_CLOCKWISE_CMD, this,
		LoadBitmap("ROTATE_ANTI_CLOCKWISE_ON"),
		B_TRANSLATE("Rotate anti-clockwise"));
	mToolBar->AddAction(ZOOM_IN_CMD, this, LoadBitmap("ZOOM_IN_ON"),
		B_TRANSLATE("Zoom in"));
	mToolBar->AddAction(ZOOM_OUT_CMD, this, LoadBitmap("ZOOM_OUT_ON"),
		B_TRANSLATE("Zoom out"));

	mToolBar->AddSeparator();

	mToolBar->AddAction(FIND_CMD, this, LoadBitmap("FIND_ON"),
		B_TRANSLATE("Find"));
	mToolBar->AddAction(FIND_NEXT_CMD, this, LoadBitmap("FIND_NEXT_ON"),
		B_TRANSLATE("Find next"));
	mToolBar->AddGlue();
	return mToolBar;
}


///////////////////////////////////////////////////////////
LayerView* PDFWindow::BuildLeftPanel(BRect rect) {
	BRect r(rect);
	LayerView* layerView;
	layerView = new LayerView(rect, "layers", B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, B_FRAME_EVENTS);

	// PageList
	mOutlinesView = new OutlinesView(rect, mMainView->GetPDFDoc()->getCatalog(),
	                                 mFileAttributes.GetBookmarks(), gApp->GetSettings(),
	                                 this, B_FOLLOW_ALL, B_FRAME_EVENTS);


	// r.Set(2, 2, rect.Width() - 2 - B_V_SCROLL_BAR_WIDTH, rect.Height() - 2 - B_H_SCROLL_BAR_HEIGHT);
	mAttachmentView = new AttachmentView(rect, gApp->GetSettings(), this, B_FOLLOW_ALL, 0);

	r.Set(2, 2, rect.Width() - 2 - B_V_SCROLL_BAR_WIDTH, rect.Height() - 2 - B_H_SCROLL_BAR_HEIGHT);

	// LayerView contains the page numbers
	mPagesView = new BListView(r, "pagesList", B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
	mPagesView->SetSelectionMessage(new BMessage(PAGE_SELECTED_CMD));

	BView *pageView = new BScrollView ("pageScrollView", mPagesView,
		B_FOLLOW_ALL_SIDES,
		B_FRAME_EVENTS,
		true, true, B_FANCY_BORDER);

	r = rect;
	r.right = r.left + TOOLBAR_WIDTH;
	mAnnotationBar = BuildAnnotToolBar(r, "annotationToolBar", NULL);
	mAnnotationBar->ResizeBy(rect.right - r.right, 0);

	layerView->AddLayer(pageView);
	layerView->AddLayer(mOutlinesView);
	layerView->AddLayer(mAnnotationBar);
	layerView->AddLayer(mAttachmentView);

	return layerView;
}

///////////////////////////////////////////////////////////
void PDFWindow::SetUpViews(entry_ref * ref, const char *ownerPassword, const char *userPassword, bool *encrypted)
{
	BScrollView * mainScrollView;
	BScrollBar * scroller;
	BRect rect;
	BFont font(be_plain_font);
	font.SetSize(10);

	SetSizeLimits(500, 10000, 120, 10000);
	BMenuBar* menuBar = BuildMenu();
	mMenuHeight = menuBar->Frame().Height() + 1;
	BuildToolBar();

	/* Main View is right view of SplitView */
	BRect r(0, mToolBar->Frame().bottom+1, Bounds().right, Bounds().bottom);
	float height = r.Height();
	float width = r.Width();

	BRect viewRect(64, 2, width - B_V_SCROLL_BAR_WIDTH, height - B_H_SCROLL_BAR_HEIGHT);
	mMainView = new PDFView(ref, &mFileAttributes, viewRect, "mainView",
				B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS,
				ownerPassword, userPassword, encrypted);

	mCurrentFile.SetTo(ref);
	if (!mMainView->IsOk()) {
		delete mMainView;
		mMainView = NULL;
		return;			//ERROR !
	}
	mEntryChangedMonitor.StartWatching(ref);

	// ScrollView of mMainView
	mainScrollView = new BScrollView ("scrollView", mMainView, B_FOLLOW_ALL, 0, true, true, B_FANCY_BORDER );
	mainScrollView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));


	#define STATUS_TEXT_WIDTH 200
	#define PAGE_NUM_WIDTH 50
	#define TOTAL_PAGE_WIDTH 50
	#define SCROLL_BAR_WIDTH 300

	rect.Set(0, 0, 50, height);

	// left view of SplitView is a LayerView
	mLayerView = BuildLeftPanel(rect);

	// SplitView
	mSplitView = new SplitView(r, "splitView", mLayerView, mainScrollView, TOOLBAR_WIDTH, 380);
	AddChild(mSplitView);

	// Set Scrollbar parameters for pageView
	scroller = mPagesView->ScrollBar(B_HORIZONTAL);
	scroller->ResizeBy(B_V_SCROLL_BAR_WIDTH, 0);
	scroller->SetRange(0, 300);
	scroller->SetSteps(30, 60);

	scroller = mainScrollView->ScrollBar (B_HORIZONTAL);

	rect = mainScrollView->Bounds();

	// Statusbar
	BRect sbr(rect.left+2, rect.bottom - B_H_SCROLL_BAR_HEIGHT - 1, rect.right - SCROLL_BAR_WIDTH - B_V_SCROLL_BAR_WIDTH - 1, rect.bottom);
	StatusBar *sb = new StatusBar(sbr, "status_bar", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, 0);
	mainScrollView->AddChild(sb);

	rect = sb->Bounds();
	int w = rect.IntegerWidth(), h = rect.IntegerHeight();

	// Status Text
	mStatusText = new BStringView (BRect (1, 2, w - 1, h), "status_text", B_TRANSLATE("Status"));
	mStatusText->SetResizingMode(B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT);
	mStatusText->SetFont(&font);
	sb->AddChild(mStatusText);

	rect = mainScrollView->Bounds();
	scroller->MoveTo(rect.right - SCROLL_BAR_WIDTH - B_V_SCROLL_BAR_WIDTH, rect.bottom - B_H_SCROLL_BAR_HEIGHT - 1);
	scroller->ResizeTo(SCROLL_BAR_WIDTH, B_H_SCROLL_BAR_HEIGHT);
	scroller->SetResizingMode(B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);

	SetTotalPageNumber(mMainView->GetNumPages());

    GlobalSettings *s = gApp->GetSettings();
    // restore position of page list
	mSplitView->Split(s->GetSplitPosition());
	// set listener now, so position of current left panel
	// is not changed by previous method invocation
	mSplitView->SetListener(this);

	// show or hide panel that is stored in settings
	if (!s->GetShowLeftPanel()) {
		// hide panel
		ToggleLeftPanel();
	} else {
		switch (s->GetLeftPanel()) {
			case PAGE_LIST_PANEL:
				// nothing to do, panel already displayed
				break;
			case BOOKMARKS_PANEL: ShowBookmarks();
				break;
			case ANNOTATIONS_PANEL: ShowAnnotationToolbar();
				break;
			case ATTACHMENTS_PANEL: ShowAttachments();
				break;
		}
	}

	// set focus to PDFView, so it receives mouse and keyboard events
	mMainView->MakeFocus ();
}

// Called when position of split view has changed.
void PDFWindow::PositionChanged(SplitView *view, float position) {
	// Remember new position
	if (gApp->GetSettings()->GetLeftPanel() == PDFWindow::PAGE_LIST_PANEL) {
		gApp->GetSettings()->SetSplitPosition(position);
	} else if (gApp->GetSettings()->GetLeftPanel() == PDFWindow::BOOKMARKS_PANEL) {
		gApp->GetSettings()->SetOutlinesPosition(position);
	} else if (gApp->GetSettings()->GetLeftPanel() == PDFWindow::ATTACHMENTS_PANEL) {
		gApp->GetSettings()->SetAttachmentsPosition(position);
	}
}


///////////////////////////////////////////////////////////
void PDFWindow::SetZoom(int16 zoom) {
	BMenuItem *item;
	gApp->GetSettings()->SetZoom(zoom);
	if (zoom >= MIN_ZOOM) {
		item = mZoomMenu->ItemAt(zoom - MIN_ZOOM + FIRST_ZOOM_ITEM_INDEX);
		if (item != NULL)
			item->SetMarked(true);
	} else {
		item = mZoomMenu->FindItem(CUSTOM_ZOOM_FACTOR_MSG);
		if (item != NULL) {
			mZoomMenu->RemoveItem(item);
			delete item;
		}
		char label[256];
		sprintf(label, B_TRANSLATE("Custom zoom factor (%d%%)"), -zoom * 100 / 72);
		BMessage *msg = new BMessage(CUSTOM_ZOOM_FACTOR_MSG);
		msg->AddInt16("zoom", zoom);
		item = new BMenuItem(label, msg, 0);
		mZoomMenu->AddItem(item);
		item->SetMarked(true);
	}
}
///////////////////////////////////////////////////////////
void PDFWindow::SetRotation(float rotation) {
int16 i;
	if (rotation <= 45) i = 0;
	else if (rotation <= 90.0+45) i = 1;
	else if (rotation <= 180.0+45) i = 2;
	else if (rotation <= 270.0+45) i = 3;
	else i = 0;
	BMenuItem *item = mRotationMenu->ItemAt(i);
	item->SetMarked(true);
}
///////////////////////////////////////////////////////////
void PDFWindow::SetStatus(const char *text) {
	mStatusText->SetText(text);
}
///////////////////////////////////////////////////////////
void PDFWindow::NewDoc(PDFDoc *doc) {
	Catalog *catalog = doc->getCatalog();

	mOutlinesView->SetCatalog(catalog, mFileAttributes.GetBookmarks());
	ActivateOutlines();

	if (mFIWMessenger && mFIWMessenger->LockTarget()) {
		BLooper *looper;
		FileInfoWindow *w = (FileInfoWindow*)mFIWMessenger->Target(&looper);
		w->Refresh(&mCurrentFile, doc, mFileAttributes.GetPage());
		looper->Unlock();
	}
	if (mPSWMessenger && mPSWMessenger->LockTarget()) {
		BLooper *looper;
		PrintSettingsWindow *w = (PrintSettingsWindow*)mPSWMessenger->Target(&looper);
		w->Refresh(doc);
		looper->Unlock();
	}
	if (mAWMessenger && mAWMessenger->LockTarget()) {
		BLooper *looper;
		AnnotationWindow *w = (AnnotationWindow*)mAWMessenger->Target(&looper);
		w->Quit();
	}
}
///////////////////////////////////////////////////////////
void PDFWindow::NewPage(int page) {
	UpdateInputEnabler();
	if (mFIWMessenger && mFIWMessenger->LockTarget()) {
		BLooper *looper;
		FileInfoWindow *w = (FileInfoWindow*)mFIWMessenger->Target(&looper);
		w->RefreshFontList(&mCurrentFile, mMainView->GetPDFDoc(), page);
		looper->Unlock();
	}
}
///////////////////////////////////////////////////////////
void PDFWindow::FrameMoved(BPoint p) {
	if (!mFullScreen) {
		gApp->GetSettings()->SetWindowPosition(p);
	}
}
///////////////////////////////////////////////////////////
void PDFWindow::FrameResized (float width, float height)
{
	if (!mFullScreen) {
		gApp->GetSettings()->SetWindowSize(width, height);
	}
}

///////////////////////////////////////////////////////////
void
PDFWindow::SetZoomSize (float w, float h)
{
	float width = w + B_V_SCROLL_BAR_WIDTH,
	height = h + mMenuHeight + TOOLBAR_HEIGHT + B_H_SCROLL_BAR_HEIGHT;
	return;

	SetSizeLimits (100, width, 100, height);
	BRect bounds(Bounds());
	if (bounds.Width() < width) width = bounds.Width();
	if (bounds.Height() < height) height = bounds.Height();
	ResizeTo(width, height);
}

///////////////////////////////////////////////////////////
// update page list and page number item
void
PDFWindow::SetPage(int16 page) {
	char pageStr [64];
	snprintf (pageStr, sizeof (pageStr), "%d", page);
	mPageNumberItem->SetText (pageStr);
	mPagesView->Select(page-1);
	mPagesView->ScrollToSelection();
}


void
PDFWindow::MessageReceived(BMessage* message)
{
	int page;
	const char *text;

	if (CancelCommand(message))
		return;

	switch (message->what) {
	case OPEN_FILE_CMD:
		mMainView->WaitForPage();
		EditAnnotation(false);
		gApp->OpenFilePanel();
		break;
	case NEW_WINDOW_CMD:
		be_roster->Launch(BEPDF_APP_SIG, 0, (char**)NULL);
		break;
	case OPEN_IN_NEW_WINDOW_CMD: {
		BMessage m(B_REFS_RECEIVED);
		entry_ref r;
		if (message->FindRef("refs", 0, &r) == B_OK) {
			BEntry entry(&r);
			BPath path;
			entry.GetPath(&path);
			OpenPDF(path.Path());
		}
		}
		break;
	case RELOAD_FILE_CMD:
		Reload();
		break;
	case SAVE_FILE_AS_CMD:
		gApp->OpenSaveFilePanel(this, GetPdfFilter());
		break;
	case CLOSE_FILE_CMD:
		mMainView->WaitForPage(true);
		PostMessage (B_QUIT_REQUESTED );
		break;
	case QUIT_APP_CMD:
	    gApp->Notify(BepdfApplication::NOTIFY_QUIT_MSG);
		break;
	case PAGESETUP_FILE_CMD:
		mMainView->PageSetup ();
		break;
	case ABOUT_APP_CMD:
		be_app->PostMessage (B_ABOUT_REQUESTED);
		break;
	case COPY_SELECTION_CMD: mMainView->CopySelection();
		break;
	case SELECT_ALL_CMD: mMainView->SelectAll();
		break;
	case SELECT_NONE_CMD: mMainView->SelectNone();
		break;
	case FIRST_PAGE_CMD:
		mMainView->MoveToPage (1);
		break;
	case PREVIOUS_N_PAGE_CMD:
		mMainView->MoveToPage(mMainView->Page() - 10);
		break;
	case NEXT_N_PAGE_CMD:
		mMainView->MoveToPage(mMainView->Page() + 10);
		break;
	case PREVIOUS_PAGE_CMD:
		if (B_SHIFT_KEY & modifiers()) {
			mMainView->ScrollVertical (false, 0.95);
		} else {
			page = mMainView->Page();
			mMainView->MoveToPage (page - 1);
		}
		break;
	case NEXT_PAGE_CMD:
		if (B_SHIFT_KEY & modifiers()) {
			mMainView->ScrollVertical (true, 0.95);
		} else {
			page = mMainView->Page();
			mMainView->MoveToPage (page + 1);
		}
		break;
	case LAST_PAGE_CMD:
		mMainView->MoveToPage (mMainView->GetNumPages());
		break;
	case GOTO_PAGE_CMD:
		{
			status_t err;
			BTextControl * control;
			BControl * ptr;

			err  = message->FindPointer ("source", (void **)&ptr);
			control = dynamic_cast <BTextControl *> (ptr);
			if (control != NULL) {
				const char *txt = control->Text ();
				page = atoi (txt);
				mMainView->MoveToPage (page);
				mMainView->MakeFocus();
			}
			else {
				/* ERROR */
			}
		}
		break;
	case PAGE_SELECTED_CMD:
		page = mPagesView->CurrentSelection(0) + 1;
		mMainView->MoveToPage(page);
		break;
	case GOTO_PAGE_MENU_CMD:
		mPageNumberItem->MakeFocus();
		break;
	case SET_ZOOM_VALUE_CMD:
		{
			status_t err;
			BMenuItem * item;
			BMenu * menu;
			BArchivable * ptr;
			int32 idx;

			err  = message->FindPointer ("source", (void **)&ptr);
			item = dynamic_cast <BMenuItem *> (ptr);
			if (item != NULL) {
				menu = item->Menu ();
				if (menu == NULL) {
					// ERROR
				}
				else {
					idx = menu->IndexOf (item) - FIRST_ZOOM_ITEM_INDEX;
					if (idx > MAX_ZOOM) {
						idx = MAX_ZOOM;
					}
					SetZoom (idx);
					mMainView->SetZoom(idx);
				}
			}
		}
		break;
	case ZOOM_IN_CMD:
	case ZOOM_OUT_CMD:
		mMainView->Zoom(message->what == ZOOM_IN_CMD);
		break;
	case FIT_TO_PAGE_WIDTH_CMD:
		mMainView->FitToPageWidth();
		break;
	case FIT_TO_PAGE_CMD:
		mMainView->FitToPage();
		break;
	case SET_ROTATE_VALUE_CMD:
		{
			status_t err;
			BMenuItem * item;
			BMenu * menu;
			BArchivable * ptr;
			int32 idx;

			err  = message->FindPointer ("source", (void **)&ptr);
			item = dynamic_cast <BMenuItem *> (ptr);
			if (item != NULL) {
				menu = item->Menu ();
				if (menu == NULL) {
					// ERROR
				}
				else {
					idx = menu->IndexOf (item);
					mMainView->SetRotation (idx * 90);
				}
			}
		}
		break;
	case ROTATE_CLOCKWISE_CMD: mMainView->RotateClockwise();
		break;
	case ROTATE_ANTI_CLOCKWISE_CMD: mMainView->RotateAntiClockwise();
		break;
	case HISTORY_BACK_CMD:
		mMainView->Back ();
		break;
	case HISTORY_FORWARD_CMD:
		mMainView->Forward ();
		break;

	case FIND_CMD:
		mMainView->WaitForPage();
		if (Lock()) {
			mFindWindow = new FindTextWindow(gApp->GetSettings(), mFindText.String(), this);
			Unlock();
		}
		break;
	case FIND_NEXT_CMD:
		mMainView->WaitForPage();
		if (Lock()) {
			mFindWindow = new FindTextWindow(gApp->GetSettings(), mFindText.String(), this);
			Unlock();
			mFindWindow->PostMessage('FIND');
		}
		break;
/*	case KEYBOARD_SHORTCUTS_CMD: {
			BAlert *info = new BAlert("Info",
				"Keyboard Shortcuts:\n\n"
				"Space - scroll forward on a page\n"
				"Backspace - scroll backwards on page\n"
				"Cursor Arrow Keys - scroll incrementally in the direction of the cursor key\n"
				"Page Up - skip to the previous page\n"
				"Page Down - skip to the next page\n"
				"Home - return to the beginning of the document\n"
				"End - advance to the end of the document\n"
				"ALT+B - return to the previously viewed page within the document"
				, "OK");
			info->Go();
		}
		break;*/
	case HELP_CMD:
		OpenHelp();
		break;
	case ONLINE_HELP_CMD:
		LaunchHTMLBrowser("http://haikuarchives.github.io/BePDF/English/table_of_contents.html");
		break;
	case HOME_PAGE_CMD:
		LaunchHTMLBrowser("http://haikuarchives.github.io/BePDF/");
		break;
	case BUG_REPORT_CMD:
		LaunchHTMLBrowser("http://github.com/HaikuArchives/BePDF/issues/");
		break;
	case PREFERENCES_FILE_CMD:
		mPreferencesItem->SetEnabled(false);
		new PreferencesWindow(gApp->GetSettings(), this);
		break;
	case FILE_INFO_CMD:
		if (SetPendingIfLocked(FILE_INFO_PENDING)) return;
		if (!ActivateWindow(mFIWMessenger)) {
			FileInfoWindow *w;
			mMainView->WaitForPage();
			w = new FileInfoWindow(gApp->GetSettings(), &mCurrentFile, mMainView->GetPDFDoc(), this, mFileAttributes.GetPage());
			mFIWMessenger = new BMessenger(w);
		}
		break;
	case PRINT_SETTINGS_CMD: {
			if (SetPendingIfLocked(PRINT_SETTINGS_PENDING)) return;
			PrintSettingsWindow *w;
			mPrintSettingsWindowOpen = true;
			UpdateInputEnabler();
			w = new PrintSettingsWindow(mMainView->GetPDFDoc(), gApp->GetSettings(), this);
			mPSWMessenger = new BMessenger(w);
		}
		break;
	case SHOW_BOOKMARKS_CMD: ShowBookmarks();
		break;
	case SHOW_PAGE_LIST_CMD: ShowPageList();
		break;
	case SHOW_ANNOT_TOOLBAR_CMD: ShowAnnotationToolbar();
		break;
	case SHOW_ATTACHMENTS_CMD: ShowAttachments();
		break;
	case HIDE_LEFT_PANEL_CMD: HideLeftPanel();
		break;
	case FULL_SCREEN_CMD: OnFullScreen();
		break;
	case ADD_BOOKMARK_CMD: AddBookmark();
		break;
	case DELETE_BOOKMARK_CMD: DeleteBookmark();
		break;
	case EDIT_BOOKMARK_CMD: EditBookmark();
		break;
	case SHOW_TRACER_CMD: OutputTracer::ShowWindow(gApp->GetSettings());
		break;
	// Annotation
	case DONE_EDIT_ANNOT_CMD: EditAnnotation(false);
		break;
	// Attachments
	case ATTACHMENT_SELECTION_CHANGED_MSG:
		message->PrintToStream();
		break;

	case CUSTOM_ZOOM_FACTOR_MSG: {
		int16 zoom;
		if (message->FindInt16("zoom", &zoom) == B_OK) {
			SetZoom(zoom);
			mMainView->SetZoom(zoom);
		}
		}
		break;

	// Find Text Window
	case FindTextWindow::FIND_START_NOTIFY_MSG: {
			bool ignoreCase;
			bool backward;
			mFindInProgress = true;
			mFindState = (uint32) FindTextWindow::FIND_STOP_NOTIFY_MSG;
			message->FindString("text", &text);
			message->FindBool("ignoreCase", &ignoreCase);
			message->FindBool("backward", &backward);
			mFindText.SetTo(text);
			mMainView->Find(text, ignoreCase, backward, mFindWindow);
			break;
		}
	case FindTextWindow::FIND_STOP_NOTIFY_MSG:
	case FindTextWindow::FIND_ABORT_NOTIFY_MSG:
		if (mFindInProgress) {
			mFindState = message->what;
			mMainView->StopFind();
		} else {
			mFindWindow->PostMessage(message->what);
			if (message->what == (uint32)FindTextWindow::FIND_ABORT_NOTIFY_MSG) {
				mFindWindow->PostMessage(FindTextWindow::FIND_QUIT_REQUESTED_MSG);
			}
		}
		UpdateInputEnabler();
		break;
	case FindTextWindow::TEXT_FOUND_NOTIFY_MSG:
	case FindTextWindow::TEXT_NOT_FOUND_NOTIFY_MSG:
		mFindInProgress = false;
		mFindWindow->PostMessage(mFindState);
		if (mFindState == (uint32)FindTextWindow::FIND_ABORT_NOTIFY_MSG) {
			mFindWindow->PostMessage(FindTextWindow::FIND_QUIT_REQUESTED_MSG);
		}
		break;

	// Page Renderer
	case PageRenderer::UPDATE_MSG:
	case PageRenderer::FINISH_MSG: {
			thread_id id;
			BBitmap *bitmap;
			PageRenderer::GetParameter(message, &id, &bitmap);
			mMainView->PostRedraw(id, bitmap);
			HandlePendingActions(message->what == PageRenderer::FINISH_MSG);
		}
		break;
	case PageRenderer::ABORT_MSG: {
			thread_id id; BBitmap *bitmap;
			PageRenderer::GetParameter(message, &id, &bitmap);
			mMainView->RedrawAborted(id, bitmap);
			HandlePendingActions(false);
		}
		break;

	// Preferences Window
	case PreferencesWindow::RESTART_DOC_NOTIFY:
		mMainView->WaitForPage(true);
		mMainView->RestartDoc();
		break;
	case PreferencesWindow::CHANGE_NOTIFY: {
		int16 kind, which, index;
			if (PreferencesWindow::DecodeMessage(message, kind, which, index)) {
				switch (kind) {
				case PreferencesWindow::DISPLAY:
					switch (which) {
					case PreferencesWindow::DISPLAY_FILLED_SELECTION:
						mMainView->SetFilledSelection(index == 0);
						break;
					}
				}
			}
		}
		break;
	case PreferencesWindow::QUIT_NOTIFY: mPreferencesItem->SetEnabled(true);
		break;
	case PreferencesWindow::UPDATE_NOTIFY:
		mMainView->UpdateSettings(gApp->GetSettings());
		break;

	// File Info Window
	case FileInfoWindow::QUIT_NOTIFY:
		mFileInfoItem->SetEnabled(true);
		delete mFIWMessenger;
		mFIWMessenger = NULL;
		break;
	case FileInfoWindow::START_QUERY_ALL_FONTS_MSG:
		mMainView->WaitForPage(); // need exculsive access to PDFDoc
		if (mFIWMessenger && mFIWMessenger->LockTarget()) {
			BLooper *looper;
			FileInfoWindow *w = (FileInfoWindow*)mFIWMessenger->Target(&looper);
			looper->Unlock();
			w->QueryAllFonts(mMainView->GetPDFDoc());
		}
		break;
	// Print Settings Window
	case PrintSettingsWindow::QUIT_NOTIFY:
		// mPrintSettingsItem->SetEnabled(true);
		mPrintSettingsWindowOpen = false;
		UpdateInputEnabler();
		delete mPSWMessenger;
		mPSWMessenger = NULL;
		break;
	case PrintSettingsWindow::PRINT_NOTIFY:
			mMainView->WaitForPage();
			mMainView->Print ();
		break;

	// Outlines View (TODO simplify, BMessenger not needed any more)
	case OutlinesView::PAGE_NOTIFY: {
			int32 page;
			if (message->FindInt32("page", &page) == B_OK) {
				mMainView->MoveToPage(page);
				UpdateInputEnabler();
			}
		}
		break;
	case OutlinesView::REF_NOTIFY: {
			int32 num, gen;
			if (message->FindInt32("num", &num) == B_OK && message->FindInt32("gen", &gen) == B_OK) {
				mMainView->MoveToPage(num, gen, true);
			}
		}
		break;
	case OutlinesView::STRING_NOTIFY: {
			BString s;
			if (message->FindString("string", &s) == B_OK) {
				mMainView->MoveToPage(s.String());
			}
		}
		break;
	case OutlinesView::DEST_NOTIFY: {
			void* link;
			if (message->FindPointer("dest", &link) == B_OK) {
				LinkDest* dest = static_cast<LinkDest*>(link);
				mMainView->GotoDest(dest);
			}
		}
		break;
	case OutlinesView::QUIT_NOTIFY:
		delete mOWMessenger; mOWMessenger = NULL;
		break;
	case OutlinesView::STATE_CHANGE_NOTIFY:
		UpdateInputEnabler();
		break;
	case BookmarkWindow::BOOKMARK_ENTERED_NOTIFY:
		{
		BString label;
		int32  pageNum;
			if (message->FindString("label", &label) == B_OK &&
			    message->FindInt32("pageNum", &pageNum) == B_OK) {
				mOutlinesView->AddUserBookmark(pageNum, label.String());
				UpdateInputEnabler();
			}
		}
	case AnnotationWindow::QUIT_NOTIFY:
			delete mAWMessenger; mAWMessenger = NULL;
			break;
	case AnnotationWindow::CHANGE_NOTIFY:
		{
			void *p;
			if (message->FindPointer("annotation", &p) == B_OK) {
				mMainView->UpdateAnnotation((Annotation*)p, message);
			}
		}
		break;

	case B_SAVE_REQUESTED:
		SaveFile(message);
		break;

	default:
		if (FIRST_ANNOT_CMD <= message->what && message->what <= LAST_ANNOT_CMD) {
			InsertAnnotation(message->what);
		} else
			HWindow::MessageReceived(message);
	}
}


///////////////////////////////////////////////////////////
void
PDFWindow::OpenPDF(const char* file) {
	char *argv[2] = { (char*)file, NULL };
	be_roster->Launch(BEPDF_APP_SIG, 1, argv);
}

///////////////////////////////////////////////////////////
bool
PDFWindow::OpenPDFHelp(const char* name) {
	BPath path(*gApp->GetAppPath());
	path.Append("docs");
	path.Append(name);
	BEntry entry(path.Path());
	if (entry.InitCheck() == B_OK && entry.Exists()) {
		printf("exists");
		OpenPDF(path.Path());
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////
void
PDFWindow::OpenHelp()
{
	// help in pdf
	BString name = gApp->GetSettings()->GetLanguage();
	name.RemoveLast(".catalog");
	name.Append(".pdf");
	name.ReplaceAll("Default", "English");
	OpenPDFHelp(name.String());
}

///////////////////////////////////////////////////////////
void
PDFWindow::LaunchHTMLBrowser(const char *path) {
	char *argv[2] = {(char*)path, NULL};
	be_roster->Launch("text/html", 1, argv);
}

///////////////////////////////////////////////////////////
void
PDFWindow::LaunchInHome(const char *rel_path) {
	BPath path(*gApp->GetAppPath());
	path.Append(rel_path);
	Launch(path.Path());
}

bool
PDFWindow::FindFile(BPath* path) {
	if (path->InitCheck() != B_OK) return false;
	BString leaf(path->Leaf());
	if (leaf.Length() == 0) {
		path->SetTo("/");
		return true;
	}
	if (path->GetParent(path) != B_OK) return false;
	if (FindFile(path)) {
		BPath p(path->Path());
		path->Append(leaf.String());
		BEntry entry(path->Path());
		if (entry.Exists()) return true;

		*path = p;
		entry.SetTo(p.Path());
		BDirectory dir(&entry);
		char name[B_FILE_NAME_LENGTH];
		while (dir.GetNextEntry(&entry) == B_OK) {
			entry.GetName(name);
			if (leaf.ICompare(name) == 0) {
				path->Append(name);
				return true;
			}
		}
	}
	return false;
}

bool
PDFWindow::GetEntryRef(const char* file, entry_ref* ref) {
	BEntry entry(file);
	BPath path(file);
	if (!entry.Exists() && FindFile(&path)) {
		entry.SetTo(path.Path());
	}
	if (entry.Exists()) {
		entry.GetRef(ref); return true;
	}
	return false;
}

///////////////////////////////////////////////////////////
void
PDFWindow::Launch(const char *file) {
	entry_ref r;
	if (GetEntryRef(file, &r)) {
		be_roster->Launch(&r);
	}
}

void
PDFWindow::OpenInWindow(const char *file) {
	entry_ref r;
	if (GetEntryRef(file, &r)) {
		BMessage msg(B_REFS_RECEIVED);
		msg.AddRef("refs", &r);
		be_app->PostMessage(&msg);
	}
}

#if 0
#pragma mark *********** Left Panel ***************
#endif

///////////////////////////////////////////////////////////
void
PDFWindow::ActivateOutlines() {
	// mMainView->WaitForPage();
	if (mLayerView->Active() == BOOKMARKS_PANEL && mShowLeftPanel) {
		mMainView->WaitForPage();
		mOutlinesView->Activate();
	}
}

///////////////////////////////////////////////////////////
void
PDFWindow::ShowLeftPanel(int panel) {
	if (mShowLeftPanel && mLayerView->Active() == panel) {
		// hide panel if tool bar item is clicked a second time
		ToggleLeftPanel();
		return;
	}

	if (!mShowLeftPanel) {
		ToggleLeftPanel();
	}
	if (mLayerView->Active() != panel) {
		float pos = 0;
		switch (panel) {
			case BOOKMARKS_PANEL: pos = gApp->GetSettings()->GetOutlinesPosition();
				break;
			case PAGE_LIST_PANEL: pos = gApp->GetSettings()->GetSplitPosition();
				break;
			case ANNOTATIONS_PANEL: pos = TOOLBAR_WIDTH;
				break;
			case ATTACHMENTS_PANEL: pos = gApp->GetSettings()->GetAttachmentsPosition();
				break;
		}
		gApp->GetSettings()->SetLeftPanel(panel);
		mSplitView->AllowUserSplit(panel != ANNOTATIONS_PANEL);
		mSplitView->Split(pos);
		mLayerView->SetActive(panel);
		UpdateInputEnabler();
		if (panel == BOOKMARKS_PANEL) {
			ActivateOutlines();
		}
	}
	UpdateInputEnabler();
}

///////////////////////////////////////////////////////////
void
PDFWindow::ShowBookmarks() {
	ShowLeftPanel(BOOKMARKS_PANEL);
}

///////////////////////////////////////////////////////////
void
PDFWindow::ShowPageList() {
	ShowLeftPanel(PAGE_LIST_PANEL);
}

///////////////////////////////////////////////////////////
void
PDFWindow::ShowAnnotationToolbar() {
	ShowLeftPanel(ANNOTATIONS_PANEL);
}

///////////////////////////////////////////////////////////
void
PDFWindow::ShowAttachments() {
	ShowLeftPanel(ATTACHMENTS_PANEL);
}

///////////////////////////////////////////////////////////
void
PDFWindow::HideLeftPanel() {
	if (mShowLeftPanel) {
		ToggleLeftPanel();
	}
}

///////////////////////////////////////////////////////////
void
PDFWindow::ToggleLeftPanel() {
	mShowLeftPanel = !mShowLeftPanel;
	UpdateInputEnabler();
	float w = mSplitView->Left()->Bounds().Width()+SplitView::SEPARATION+1;
	gApp->GetSettings()->SetShowLeftPanel(mShowLeftPanel);
	if (mShowLeftPanel) {
		ActivateOutlines();
		mSplitView->SetFlags(B_NAVIGABLE | mSplitView->Flags());
		mSplitView->ResizeBy(-w, 0);
		mSplitView->MoveBy(w, 0);
	} else {
		// Hide left panel moves the left panel "outside" of the left window border.
		// The SplitView has to be resized so it fits into the window.
		mSplitView->SetFlags((~B_NAVIGABLE) & mSplitView->Flags());
		mSplitView->ResizeBy(w, 0);
		mSplitView->MoveBy(-w, 0);
	}
	mMainView->Resize();
}

void
PDFWindow::OnFullScreen() {
	bool quasiFullScreenMode = gApp->GetSettings()->GetQuasiFullscreenMode();
	mFullScreen = !mFullScreen;
	BRect frame;
	bool pgList = false;
	if (mFullScreen) {
		pgList = mShowLeftPanel;
		if (pgList) ToggleLeftPanel(); // left panel
		mWindowFrame = Frame();
		frame = gScreen->Frame();
		if (quasiFullScreenMode) {
			frame.OffsetBy(0, -mMenuHeight);
			frame.bottom += mMenuHeight;
		} else {
			BRect bounds = mMainView->Parent()->ConvertToScreen(mMainView->Frame());
			frame.bottom += mWindowFrame.IntegerHeight() - bounds.IntegerHeight();
			frame.right += mWindowFrame.IntegerWidth() - bounds.IntegerWidth();
			frame.OffsetBy(-bounds.left+mWindowFrame.left, -bounds.top+mWindowFrame.top);
		}
		mFullScreenItem->SetLabel(B_TRANSLATE("Window"));
		SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
		SetFlags(Flags() | B_NOT_RESIZABLE | B_NOT_MOVABLE);
		Activate(true);
	} else {
		SetFeel(B_NORMAL_WINDOW_FEEL);
		SetFlags(Flags() & ~(B_NOT_RESIZABLE | B_NOT_MOVABLE));
		frame = mWindowFrame;
		mFullScreenItem->SetLabel(B_TRANSLATE("Fullscreen"));
	}
	MoveTo(frame.left, frame.top);
	ResizeTo(frame.Width(), frame.Height());

	if (pgList) ToggleLeftPanel(); // show left panel

	if (!quasiFullScreenMode) {
		if (mFullScreen) {
			// increase size of page list
			mLayerView->ResizeBy(0, 1);
		} else {
			// decrease size of page list
			mLayerView->ResizeBy(0, -1);
		}
	}

	UpdateInputEnabler();
}

//~ this is very restrictive: it assumes that the window is only set in one workspace
void PDFWindow::WorkspaceActivated(int32 workspace, bool active) {
#ifdef MORE_DEBUG
	fprintf(stderr, "%s %d %s %d %d\n",
		mFullScreen ? "fullscreen" : "window",
		workspace,
		active ? "active" : "not active",
		mCurrentWorkspace, Workspaces());
#endif
	if (mFullScreen) {
		if (mCurrentWorkspace == 1 << workspace) {
			SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
		} else {
			SetFeel(B_NORMAL_WINDOW_FEEL);
			SetWorkspaces(mCurrentWorkspace);
		}
	} else if (active) {
		mCurrentWorkspace = 1 << workspace;
	}
}

// User defined bookmarks
#if 0
#pragma mark *********** Bookmark ***************
#endif

void PDFWindow::AddBookmark() {
	char buffer[256];
	sprintf(buffer, B_TRANSLATE("Page %d"), mMainView->Page());
	new BookmarkWindow(mMainView->Page(), buffer, BRect(30, 30, 300, 200), this);
}

void PDFWindow::DeleteBookmark() {
	mOutlinesView->RemoveUserBookmark(mMainView->Page());
	UpdateInputEnabler();
}

void PDFWindow::EditBookmark() {
	const char *label = mOutlinesView->GetUserBMLabel(mMainView->Page());
	if (label) {
		new BookmarkWindow(mMainView->Page(), label, BRect(30, 30, 300, 200), this);
	} else {
		// should not reach here
	}
}

// Annotations
#if 0
#pragma mark *********** Annotation ***************
#endif

static const int32 kAnnotDescEOL = -1;
static const int32 kAnnotDescSeparator = -2;

static AnnotDesc annotDescs[] = {
	{ PDFWindow::ADD_COMMENT_TEXT_ANNOT_CMD, B_TRANSLATE("Add comment text annotation"), "ANNOT_COMMENT"},
	{ PDFWindow::ADD_HELP_TEXT_ANNOT_CMD, B_TRANSLATE("Add help text annotation"), "ANNOT_HELP"},
	{ PDFWindow::ADD_INSERT_TEXT_ANNOT_CMD, B_TRANSLATE("Add insert text annotation"), "ANNOT_INSERT"},
	{ PDFWindow::ADD_KEY_TEXT_ANNOT_CMD, B_TRANSLATE("Add key text annotation"), "ANNOT_KEY"},
	{ PDFWindow::ADD_NEW_PARAGRAPH_TEXT_ANNOT_CMD, B_TRANSLATE("Add new paragraph text annotation"), "ANNOT_NEW_PARAGRAPH"},
	{ PDFWindow::ADD_NOTE_TEXT_ANNOT_CMD, B_TRANSLATE("Add note text annotation"), "ANNOT_NOTE"},
	{ PDFWindow::ADD_PARAGRAPH_TEXT_ANNOT_CMD, B_TRANSLATE("Add paragraph text annotation"), "ANNOT_PARAGRAPH"},
	{ PDFWindow::ADD_LINK_ANNOT_CMD, B_TRANSLATE("Add link annotation"), "ANNOT_LINK"},
	{ kAnnotDescSeparator, NULL, NULL},
	{ PDFWindow::ADD_FREETEXT_ANNOT_CMD, B_TRANSLATE("Add free text annotation"), "ANNOT_FREETEXT"},
	{ PDFWindow::ADD_LINE_ANNOT_CMD, B_TRANSLATE("Add line annotation"), "ANNOT_LINE"},
	{ PDFWindow::ADD_SQUARE_ANNOT_CMD, B_TRANSLATE("Add square annotation"), "ANNOT_SQUARE"},
	{ PDFWindow::ADD_CIRCLE_ANNOT_CMD, B_TRANSLATE("Add circle annotation"), "ANNOT_CIRCLE"},
	{ PDFWindow::ADD_HIGHLIGHT_ANNOT_CMD, B_TRANSLATE("Add highlight annotation"), "ANNOT_HIGHLIGHT"},
	{ PDFWindow::ADD_UNDERLINE_ANNOT_CMD, B_TRANSLATE("Add underline annotation"), "ANNOT_UNDERLINE"},
	{ PDFWindow::ADD_SQUIGGLY_ANNOT_CMD, B_TRANSLATE("Add squiggly annotation"), "ANNOT_SQUIGGLY"},
	{ PDFWindow::ADD_STRIKEOUT_ANNOT_CMD, B_TRANSLATE("Add strikeout annotation"), "ANNOT_STRIKEOUT"},
	{ PDFWindow::ADD_STAMP_ANNOT_CMD, B_TRANSLATE("Add stamp annotation"), "ANNOT_STAMP"},
	{ PDFWindow::ADD_INK_ANNOT_CMD, B_TRANSLATE("Add ink annotation"), "ANNOT_INK"},
	{ PDFWindow::ADD_POPUP_ANNOT_CMD, B_TRANSLATE("Add popup annotation"), "ANNOT_POPUP"},
	{ PDFWindow::ADD_FILEATTACHMENT_ANNOT_CMD, B_TRANSLATE("Add fileattachment annotation"), "ANNOT_FILEATTACHMENT"},
	{ PDFWindow::ADD_SOUND_ANNOT_CMD, B_TRANSLATE("Add sound annotation"), "ANNOT_SOUND"},
	{ PDFWindow::ADD_MOVIE_ANNOT_CMD, B_TRANSLATE("Add movie annotation"), "ANNOT_MOVIE"},
	{ PDFWindow::ADD_WIDGET_ANNOT_CMD, B_TRANSLATE("Add widget annotation"), "ANNOT_WIDGET"},
	{ PDFWindow::ADD_PRINTERMARK_ANNOT_CMD, B_TRANSLATE("Add printer mark annotation"), "ANNOT_PRINTERMARK"},
	{ PDFWindow::ADD_TRAPNET_ANNOT_CMD, B_TRANSLATE("Add trapnet annotation"), "ANNOT_TRAPNET"},
	{ kAnnotDescEOL, NULL, NULL}
};

BToolBar* PDFWindow::BuildAnnotToolBar(BRect rect, const char* name,
	AnnotDesc* desc)
{
	BToolBar* toolbar = new BToolBar(rect, B_VERTICAL);
	toolbar->SetName(name);
	toolbar->SetResizingMode(B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT_RIGHT);
	toolbar->SetFlags(B_WILL_DRAW | B_FRAME_EVENTS);

	// label also used in PDFView!
	toolbar->AddAction(DONE_EDIT_ANNOT_CMD, this, LoadBitmap("DONE_ANNOT_ON"),
		B_TRANSLATE("Leave annotation editing mode"));
	toolbar->AddAction(SAVE_FILE_AS_CMD, this, LoadBitmap("SAVE_FILE_AS_ON"),
		B_TRANSLATE("Save file as"));

	toolbar->AddSeparator();

	// add buttons for supported annotations
	for (desc = annotDescs; desc->mCmd != kAnnotDescEOL; desc ++) {
		if (desc->mCmd == kAnnotDescSeparator) {
			toolbar->AddSeparator();
			continue;
		}

		Annotation* annot = GetAnnotTemplate(desc->mCmd);
		if (annot == NULL) continue;
		BString name(desc->mButtonPrefix);
		BString on(desc->mButtonPrefix);
		name.ToLower();
		name << "_btn";
		on << "_ON";

		toolbar->AddAction(desc->mCmd, this, LoadBitmap(on.String()),
			desc->mToolTip, NULL, true);
	}
	return toolbar;
}

bool PDFWindow::TryEditAnnot() {
	if (mMainView->GetPDFDoc()->isEncrypted()) {
		BAlert* alert = new BAlert(B_TRANSLATE("Warning"), B_TRANSLATE("Editing of annotations in an encrypted PDF file isn't supported yet!"), B_TRANSLATE("OK"));
		alert->Go();
		return false;
	} else {
		EditAnnotation(true);
		return true;
	}
}

void PDFWindow::EditAnnotation(bool edit) {
	if (edit == mMainView->EditingAnnot()) {
		return;
	}
	if (edit) {
		mMainView->BeginEditAnnot();
	} else {
		ReleaseAnnotationButton();
		mMainView->EndEditAnnot();
	}
	mMainView->Invalidate();
	UpdateInputEnabler();
}

void PDFWindow::InitAnnotTemplates() {
	for (int i = 0; i < NUM_ANNOTS; i++) mAnnotTemplates[i] = NULL;

	PDFRectangle rect;
	// rect.x1 == -1 means that when the annotation is added to the the page
	// resize mode should be enabled otherwise the rectangle should be used
	// as default and move mode should be enabled.
	rect.x1 = -1; rect.x2 = 40;
	rect.y1 = 0; rect.y2 = 40;

	for (int i = 0; i < TextAnnot::no_of_types-1; i++) {
		BBitmap* bitmap = gApp->GetTextAnnotImage(i);
		PDFRectangle rect;
		BRect bounds(bitmap->Bounds());
		rect.x1 = rect.y1 = 0;
		rect.x2 = bounds.right;
		rect.y2 = bounds.bottom;
		SetAnnotTemplate(ADD_COMMENT_TEXT_ANNOT_CMD+i, new TextAnnot(rect, (TextAnnot::text_annot_type)i));
	}

	PDFPoint line[2];
	line[0] = PDFPoint(rect.x1, rect.y1);
	line[1] = PDFPoint(rect.x2, rect.y1);

	PDFFont* font = BePDFAcroForm::GetStandardFonts()->FindByName("Helvetica");
	ASSERT(font != NULL);
	SetAnnotTemplate(ADD_FREETEXT_ANNOT_CMD, new FreeTextAnnot(rect, font));

	SetAnnotTemplate(ADD_LINE_ANNOT_CMD, new LineAnnot(rect, line));

	SetAnnotTemplate(ADD_SQUARE_ANNOT_CMD, new SquareAnnot(rect));
	SetAnnotTemplate(ADD_CIRCLE_ANNOT_CMD, new CircleAnnot(rect));
	SetAnnotTemplate(ADD_HIGHLIGHT_ANNOT_CMD, new HighlightAnnot(rect));
	SetAnnotTemplate(ADD_UNDERLINE_ANNOT_CMD, new UnderlineAnnot(rect));
	SetAnnotTemplate(ADD_SQUIGGLY_ANNOT_CMD, new SquigglyAnnot(rect));
	SetAnnotTemplate(ADD_STRIKEOUT_ANNOT_CMD, new StrikeOutAnnot(rect));
}

void PDFWindow::DeleteAnnotTemplates() {
	for (int i = 0; i < NUM_ANNOTS; i++) {
		delete mAnnotTemplates[i];
		mAnnotTemplates[i] = NULL;
	}
}

void PDFWindow::SetAnnotTemplate(int cmd, Annotation* a) {
	ASSERT(FIRST_ANNOT_CMD <= cmd && cmd <= LAST_ANNOT_CMD);
	ASSERT(mAnnotTemplates[cmd - FIRST_ANNOT_CMD] == NULL);
	if (CanWrite(a)) {
		mAnnotTemplates[cmd - FIRST_ANNOT_CMD] = a;
		// add popup annotation to annotation if it's not a FreeTextAnnot
		if (dynamic_cast<FreeTextAnnot*>(a) == NULL) {
			PDFRectangle rect;
			rect.x1 = 0; rect.x2 = 300;
			rect.y1 = 0; rect.y2 = 200;
			PopupAnnot* popup = new PopupAnnot(rect);
			a->SetPopup(popup);
		}
	} else {
		delete a;
	}
}

Annotation* PDFWindow::GetAnnotTemplate(int cmd) {
	ASSERT(FIRST_ANNOT_CMD <= cmd && cmd <= LAST_ANNOT_CMD);
	return mAnnotTemplates[cmd - FIRST_ANNOT_CMD];
}

void PDFWindow::InsertAnnotation(int cmd) {
	ReleaseAnnotationButton();

	if (!mMainView->EditingAnnot() && !TryEditAnnot()) {
		return;
	}

	PressAnnotationButton();
	Annotation* templateAnnotation = GetAnnotTemplate(cmd);
	if (templateAnnotation != NULL) {
		mMainView->InsertAnnotation(templateAnnotation);
	} else {
		ReleaseAnnotationButton();
	}
}

class SaveFileThread : public SaveThread {
public:
	SaveFileThread(const char* title, XRef* xref, const char* path, PDFView* view)
		: SaveThread(title, xref)
		, mPath(path)
		, mMainView(view)
	{

	}

	int32 Run() {
		BAlert* alert = NULL;

		AnnotWriter writer(GetXRef(), mMainView->GetPDFDoc(),
			mMainView->GetPageRenderer()->GetAnnotsList(),
			mMainView->GetBePDFAcroForm());
		if (writer.WriteTo(mPath.String())) {
			alert = new BAlert("Information", B_TRANSLATE("PDF file successfully written!"), B_TRANSLATE("OK"), 0, 0, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		} else {
			alert = new BAlert("Error", B_TRANSLATE("Could not write PDF file!"), B_TRANSLATE("OK"), 0, 0, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		}

		alert->Go();
		delete this;
		return 0;
	}

private:
	BString  mPath;
	PDFView* mMainView;
};

void PDFWindow::SaveFile(BMessage* msg) {
	entry_ref dir;
	BString   name;
	if (msg->FindRef("directory", &dir) == B_OK &&
		msg->FindString("name", &name) == B_OK) {
		BEntry entry(&dir);
		BPath  path(&entry);
		path.Append(name.String());
		BEntry newFile(path.Path());
		if (newFile != mCurrentFile) {
			gPdfLock->Lock();
			mMainView->SyncAnnotation(false);
			gPdfLock->Unlock();

			SaveFileThread* thread = new SaveFileThread(
				B_TRANSLATE("Saving copy of PDF file:"),
				mMainView->GetPDFDoc()->getXRef(),
				path.Path(),
				mMainView);

			thread->Resume();
		} else {
			BAlert* alert = NULL;
			alert = new BAlert(B_TRANSLATE("Warning"), B_TRANSLATE("Can not overwrite a PDF file that's currently opened in BePDF! Please choose another file name."), B_TRANSLATE("OK"));
			alert->Go();
		}
	}
}

void PDFWindow::PressAnnotationButton() {
	BMessage* msg = CurrentMessage();
	BControl* control;
	if (msg && msg->FindPointer("source", (void**)&control) == B_OK) {
		control->SetValue(B_CONTROL_ON);
		mPressedAnnotationButton = control;
	}
}

void PDFWindow::ReleaseAnnotationButton() {
	if (mPressedAnnotationButton) {
		mPressedAnnotationButton->SetValue(B_CONTROL_OFF);
		mPressedAnnotationButton = NULL;
	}
}
