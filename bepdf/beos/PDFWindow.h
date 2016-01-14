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

#ifndef _PDF_WINDOW_
#define _PDF_WINDOW_


#include <be/interface/ListView.h>
#include <be/interface/PictureButton.h>
#include <be/interface/TextControl.h>
#include <be/storage/Entry.h>
#include <be/storage/Path.h>

// BePDF
#include "EntryChangedMonitor.h"
#include "FindTextWindow.h"
#include "HWindow.h"
#include "InputEnabler.h"
#include "PDFView.h"
#include "SplitView.h"
#include "ToolTip.h"

// xpdf
#include <Catalog.h>

class AnnotationWindow;
class AttachmentView;
class LayerView;
class OutlinesView;
class ResourceBitmapButton;
class ToolBar;

typedef struct {
	int mCmd;
	const char* mToolTip;
	const char* mButtonPrefix;
} AnnotDesc;

class RecentDocumentsMenu : public BMenu
{
public:
	RecentDocumentsMenu(const char *title, uint32 what, menu_layout layout = B_ITEMS_IN_COLUMN);
	bool AddDynamicItem(add_state s);

private:
	void UpdateRecentDocumentsMenu();
	uint32 fWhat;
};


class PDFWindow
	: public HWindow
	, public EntryChangedListener
	, public PositionChangedListener
{
public:
	enum {
		// File
		OPEN_FILE_CMD = 10,
		OPEN_IN_NEW_WINDOW_CMD,
		NEW_WINDOW_CMD,
		CLOSE_FILE_CMD,
		RELOAD_FILE_CMD,
		SAVE_FILE_AS_CMD,
		QUIT_APP_CMD,
		ABOUT_APP_CMD,
		KEYBOARD_SHORTCUTS_CMD,
		TYPE3_FONT_RENDERER_CMD,
		PREFERENCES_FILE_CMD,
		FILE_INFO_CMD,
			// Printing
			PAGESETUP_FILE_CMD,
			PRINT_SETTINGS_CMD,
			// PRINT_FILE_CMD,

		// Edit
		COPY_SELECTION_CMD,
		SELECT_ALL_CMD,
		SELECT_NONE_CMD,
		COPY_LINK_CMD,

		// Zoom
		SET_ZOOM_VALUE_CMD,
		SET_CUSTOM_ZOOM_FACTOR_CMD,
		ZOOM_IN_CMD,
		ZOOM_OUT_CMD,
		FIT_TO_PAGE_WIDTH_CMD,
		FIT_TO_PAGE_CMD,

		// Annotations
		FIRST_ANNOT_CMD,
			ADD_COMMENT_TEXT_ANNOT_CMD = FIRST_ANNOT_CMD,
			ADD_HELP_TEXT_ANNOT_CMD,
			ADD_INSERT_TEXT_ANNOT_CMD,
			ADD_KEY_TEXT_ANNOT_CMD,
			ADD_NEW_PARAGRAPH_TEXT_ANNOT_CMD,
			ADD_NOTE_TEXT_ANNOT_CMD,
			ADD_PARAGRAPH_TEXT_ANNOT_CMD,
		ADD_LINK_ANNOT_CMD,
		ADD_FREETEXT_ANNOT_CMD,
		ADD_LINE_ANNOT_CMD,
		ADD_SQUARE_ANNOT_CMD,
		ADD_CIRCLE_ANNOT_CMD,
		ADD_HIGHLIGHT_ANNOT_CMD,
		ADD_UNDERLINE_ANNOT_CMD,
		ADD_SQUIGGLY_ANNOT_CMD,
		ADD_STRIKEOUT_ANNOT_CMD,
		ADD_STAMP_ANNOT_CMD,
		ADD_INK_ANNOT_CMD,
		ADD_POPUP_ANNOT_CMD,
		ADD_FILEATTACHMENT_ANNOT_CMD,
		ADD_SOUND_ANNOT_CMD,
		ADD_MOVIE_ANNOT_CMD,
		ADD_WIDGET_ANNOT_CMD,
		ADD_PRINTERMARK_ANNOT_CMD,
		ADD_TRAPNET_ANNOT_CMD,
		LAST_ANNOT_CMD = ADD_TRAPNET_ANNOT_CMD,
		DONE_EDIT_ANNOT_CMD,

		// Page
		FIRST_PAGE_CMD,
		NEXT_N_PAGE_CMD,
		NEXT_PAGE_CMD,
		PREVIOUS_PAGE_CMD,
		PREVIOUS_N_PAGE_CMD,
		LAST_PAGE_CMD,
		GOTO_PAGE_CMD,
		GOTO_PAGE_MENU_CMD,
		HISTORY_BACK_CMD,
		HISTORY_FORWARD_CMD,
		PAGE_SELECTED_CMD,

		// Rotation
		SET_ROTATE_VALUE_CMD,
		ROTATE_CLOCKWISE_CMD,
		ROTATE_ANTI_CLOCKWISE_CMD,
		// Attachments
		ATTACHMENT_SELECTION_CHANGED_MSG,

		SHOW_TRACER_CMD,

		// Search
		FIND_CMD,
		FIND_NEXT_CMD,

		// User defined bookmarks
		ADD_BOOKMARK_CMD,
		DELETE_BOOKMARK_CMD,
		EDIT_BOOKMARK_CMD,

		// Help
		HELP_CMD,
		ONLINE_HELP_CMD,
		BUG_REPORT_CMD,
		HOME_PAGE_CMD,
		CHECK_FOR_UPDATE_CMD,
		// Type 1 Font Renderer
		T1_OFF,
		T1_ON,
		T1_AA,
		T1_AA_HIGH,
		// Freetype Font Renderer
		FT_OFF,
		FT_ON,
		FT_AA,
		// show/hide Page List
		SHOW_BOOKMARKS_CMD,
		SHOW_PAGE_LIST_CMD,
		SHOW_ANNOT_TOOLBAR_CMD,
		SHOW_ATTACHMENTS_CMD,
		HIDE_LEFT_PANEL_CMD,
		// full screen
		FULL_SCREEN_CMD,
	};

	enum {
		TOOLBAR_HEIGHT = 30,
		TOOLBAR_WIDTH = 30,
		NUM_ANNOTS = LAST_ANNOT_CMD - FIRST_ANNOT_CMD + 1
	};

	// active view in left panel
	enum {
		PAGE_LIST_PANEL = 0,
		BOOKMARKS_PANEL = 1,
		ANNOTATIONS_PANEL = 2,
		ATTACHMENTS_PANEL = 3,
	};

	// pending mask
	enum {
		UPDATE_PAGE_LIST_PENDING =    1 << 0,
		UPDATE_OUTLINE_LIST_PENDING = 1 << 1,
		FILE_INFO_PENDING =           1 << 2,
		PRINT_SETTINGS_PENDING =      1 << 3,
	};

private:
	InputEnabler   mInputEnabler;
	InputEnabler   mControlValueSetter;

	BEntry         mCurrentFile;
	FileAttributes mFileAttributes;
	EntryChangedMonitor mEntryChangedMonitor;

	ToolBar        *mToolBar;
	BTextControl   *mPageNumberItem;
	BStringView    *mTotalPageNumberItem;
	SplitView      *mSplitView;
	PDFView        *mMainView;
	LayerView      *mLayerView;
	BListView      *mPagesView;
	OutlinesView   *mOutlinesView;
	ToolBar        *mAnnotationBar;
	AttachmentView *mAttachmentView;
	BStringView    *mStatusText;

	BMessage       *mPrintSettings;
	FindTextWindow *mFindWindow;
	BMenuItem      *mPreferencesItem, *mFileInfoItem, // *mPrintSettingsItem,
	               *mFullScreenItem;
	BMenu          *mOpenMenu, *mNewMenu, *mWindowsMenu;

	float          mMenuHeight;

	uint32         mFindState;
	BString        mFindText;
	bool           mFindInProgress;

	BMenu          *mZoomMenu,
	               *mRotationMenu;
	ToolTip        *mToolTip;
	BMessenger     *mOWMessenger;  // outlines window messenger
	BMessenger     *mFIWMessenger; // file info window messenger
	BMessenger     *mPSWMessenger; // printing settings window messenger
	BMessenger     *mAWMessenger;  // annotation window messenger
	bool           mPrintSettingsWindowOpen;

	bool           mShowLeftPanel;

	BRect          mWindowFrame;
	bool           mFullScreen;
	int32          mCurrentWorkspace;

	uint32         mPendingMask;

	Annotation*    mAnnotTemplates[NUM_ANNOTS];
	BControl*      mPressedAnnotationButton;

public:
	PDFWindow (entry_ref * ref, BRect frame, bool quitWhenClosed, const char *ownerPassword, const char *userPassword, bool *encrypted);
	virtual ~PDFWindow();

	virtual bool QuitRequested();
	void SetStatus(const char *text);
	virtual	bool CanClose();
	bool IsOk();
	BMenuBar* BuildMenu();
	ToolBar* BuildToolBar();
	LayerView* BuildLeftPanel(BRect rect);
	void SetUpViews (entry_ref * ref, const char *ownerPassword, const char *userPassword, bool *encrypted);
	void CleanUpBeforeLoad();
	bool IsCurrentFile(entry_ref* ref) const;
	bool LoadFile(entry_ref *ref, const char *ownerPassword, const char *userPassword, bool *encrypted);
	void Reload(void);
	void EntryChanged();
	void StoreFileAttributes();
	FileAttributes *GetFileAttributes() { return &mFileAttributes; };

	virtual void FrameMoved (BPoint p);
	virtual void FrameResized (float width, float height);
	virtual void MessageReceived (BMessage * message);
	virtual void PositionChanged(SplitView* view, float position);
	void SetZoomSize (float w, float h);
	void SetZoom(int16 zoom);
	void SetRotation(float rotation);
	void SetPage(int16 page);

	static void OpenPDF(const char* file);
	static bool OpenPDFHelp(const char* name);
	static void OpenHelp();
	static void LaunchHTMLBrowser(const char *file);
	static void LaunchInHome(const char *rel_path);
	static bool FindFile(BPath* path);
	static bool GetEntryRef(const char* file, entry_ref* ref);
	static void Launch(const char *file);
	static void OpenInWindow(const char* file);
	AnnotationWindow* GetAnnotationWindow();
	AnnotationWindow* ShowAnnotationWindow();

	// hook function
	void NewDoc(PDFDoc *doc);
	void NewPage(int page);

	enum {
		PAGE_CHANGE_NOTIFY_MSG = 'Page',	/*message of page change notification*/
		CUSTOM_ZOOM_FACTOR_MSG = 'CuZo'
	};

	static char * PAGE_MSG_LABEL;

	const BEntry* CurrentFile() const { return &mCurrentFile; }
	void UpdateInputEnabler();

	void UpdateWindowsMenu();

	void ClearPending()          { mPendingMask = 0; }
	bool IsPending(uint32 mask)  { return (mPendingMask & mask) != 0; }
	void SetPending(uint32 mask) { mPendingMask |= mask; }
	bool SetPendingIfLocked(uint32 mask);

	void FillPageList();
	void UpdatePageList();
	void HandlePendingActions(bool ok);

	void EditAnnotation(bool edit);
	void ReleaseAnnotationButton();

protected:
	bool CancelCommand(int32 cmd, BMessage * msg);
	virtual void HandleCommand ( int32 cmd, BMessage * msg );
	void Find(const char *s);
	void AddItem(BMenu *subMenu, const char *label, uint32 cmd, bool marked, char shortcut = 0, uint32 modifiers = 0);
	typedef ResourceBitmapButton *RBBP;

	// register control depending on behavior at input enabler (behavior == B_ONE_STATE_BUTTON)
	// or control value setter (behavior == B_TWO_STATE_BUTTON).
	void Register(uint32 behavior, BControl* control, int32 cmd);

	ResourceBitmapButton * AddButton(ToolBar* toolBar, const char *name,
		const char *off, const char *on,
		int32 cmd, const char *info,
		uint32 behavior = B_ONE_STATE_BUTTON);

	ResourceBitmapButton * AddButton(ToolBar* toolBar, const char *name,
		const char *off, const char *on,
		const char *off_grey, const char *on_grey,
		int32 cmd, const char *info,
		uint32 behavior = B_ONE_STATE_BUTTON);

	void ActivateOutlines();

	bool ActivateWindow(BMessenger *messenger);
	void WorkspaceActivated(int32 workspace, bool active);
	void SetTotalPageNumber(int pages);
	void InitAfterOpen();

	void ToggleLeftPanel(); // show / hide panel
	void ShowLeftPanel(int panel);
	void ShowBookmarks();
	void ShowPageList();
	void ShowAnnotationToolbar();
	void ShowAttachments();
	void HideLeftPanel();

	void OnFullScreen();

	// User defined bookmarks
	void AddBookmark();
	void DeleteBookmark();
	void EditBookmark();

	// Annotations
	void PressAnnotationButton();
	ToolBar* BuildAnnotToolBar(BRect rect, const char* name, AnnotDesc* desc);
	bool TryEditAnnot();
	void InitAnnotTemplates();
	void DeleteAnnotTemplates();
	void SetAnnotTemplate(int cmd, Annotation* a);
	Annotation* GetAnnotTemplate(int cmd);
	void InsertAnnotation(int cmd);
	void SaveFile(BMessage* msg);
};


//////////////////////////////////////////////////////////////////
inline bool PDFWindow::IsOk()
{
	if (mMainView != NULL) {
		return mMainView->IsOk();
	}
	else {
		return false;
	}
}




//////////////////////////////////////////////////////////////////


#endif
