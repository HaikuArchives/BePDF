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

#ifndef PREFERENCES_WINDOW_H
#define PREFERENCES_WINDOW_H

// xpdf
#include <XRef.h>
#include <PDFDoc.h>
// BeOS
#include <FindDirectory.h>
#include <Looper.h>
// layout
#include <MWindow.h>
#include <MRadioGroup.h>
#include <MListView.h>
#include <MOutlineListView.h>
#include <LayeredGroup.h>
#include <MPopup.h>
#include <MCheckBox.h>

#include "DisplayCIDFonts.h"
#include "Settings.h"

class PreferencesWindow : public MWindow {
public:
	// message sent to mLooper has this fields:
	enum {
		// what
		CHANGE_NOTIFY            = 'PreC',
		QUIT_NOTIFY              = 'PreQ',
		FONT_CHANGED_NOTIFY      = 'FntC',
		RESTART_DOC_NOTIFY       = 'PreR',
		UPDATE_NOTIFY            = 'PruP',
		// kind
		DISPLAY                  = 0,
		// which
		DISPLAY_FILLED_SELECTION = 0,
		DISPLAY_FULLSCREEN,
		DISPLAY_NUM_MAX,
		// font
		FONT_PREDECESSOR_INDEX   = -1,
		NO_OF_FONTS,
	};
	
	static bool DecodeMessage(BMessage *msg, int16 &kind, int16 &which, int16 &index);

	PreferencesWindow(GlobalSettings *settings, BLooper *looper);
	~PreferencesWindow();
	
	void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
	virtual void FrameMoved(BPoint point);
	virtual void FrameResized(float w, float h);
	
private:
	int DISPLAY_NUM;

	enum {
		PREFERENCE_SELECTED           = 'Sele',
		LANGUAGE_SELECTED             = 'LSel',
		RESTORE_PAGE_NO_CHANGED       = 'RPch',
		RESTORE_WINDOW_FRAME_CHANGED  = 'RWch',
		QUASI_FULLSCREEN_MODE_CHANGED = 'FSch',
		FILLED_SELECTION_CHANGED      = 'FlSc',
		FONT_SELECTED                 = 'FtSl',
		OPEN_IN_WORKSPACE_CHANGED     = 'OpWS',
		WORKSPACE_CHANGED             = 'WSch',
		AUTHOR_CHANGED                = 'Atch',
		INVERT_VERTICAL_SCROLLING_CHANGED = 'IvSl',
		DISPLAY_CID_FONT_SELECTED     = 'DCFs',
		HINTING_CHANGED               = 'Hint',
	};
	MView            *mView;
	BLooper          *mLooper;
	MOutlineListView *mPreferences;
	LayeredGroup     *mLayers;
	MRadioGroup      *mDisplay[DISPLAY_NUM_MAX];
	GlobalSettings   *mSettings;
	MListView        *mList;
	MPopup           *mOpenInWorkspace;
	DisplayCIDFonts  *mDisplayCIDFonts;
	BMessage          mFontMenuFields;
		
	void SetupView();
	MView* BuildAsianFontsView();
	DisplayCIDFonts::Type GetType(const char* file);
	void FillFontFileMenu(BMenuField* menuField, const char* name, const char* file);
	void FillFontFileMenu(BMenuField* menuField, directory_which which, const char* name, const char* label, const char* file);
	void DisplayCIDFontSelected(BMessage* msg);
	void ClearView();
	bool FindRadioGroup(MRadioGroup *group, bool &displayPrinter, int32 &index);
	void BuildWorkspaceMenu(BMenu *menu);
	void SelectMenuItem(int kind, BMessage* msg);
	void Notify(uint32 what);
	void NotifyRestartDoc();
	void UpdateWorkspace();
};

#endif
