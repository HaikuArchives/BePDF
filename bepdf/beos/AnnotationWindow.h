/*  
    BeOS Front-end of PDF file reader xpdf.
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
#ifndef _ANNOTATION_WINDOW_H
#define _ANNOTATION_WINDOW_H

#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <Looper.h>

#include <MWindow.h>
#include <MButton.h>
#include <MStringView.h>
#include <MPopup.h>
#include "LayoutUtils.h"

#include "Settings.h"
#include "Annotation.h"

class AnnotationWindow : public MWindow {
public:
	AnnotationWindow(GlobalSettings *settings, BLooper *looper);
	void MessageReceived(BMessage *msg);
	void Update(Annotation* a, const char* label, const char* date, const char* contents, const char* font, float size, const char* align);
	void MakeEditable(bool e);
	void GetContents(Annotation* forAnnot, BMessage* data);
	enum public_messages {
		// notify main window that this window quits
		QUIT_NOTIFY = 'AnnQ',
		// or contents has changed
		CHANGE_NOTIFY = 'AncH', 
	};
	virtual void FrameMoved(BPoint point);
	virtual void FrameResized(float w, float h);
	bool QuitRequested();
	
protected:
	enum private_messages {
		FONT_SELECTED = 'Font',
		SIZE_CHANGED = 'Size',
		ALIGNMENT_CHANGED = 'Alig',
	};

	void PopulateFontMenu(BMenu* menu);
	void AddSizeItem(BMenu* menu, const char* label, float value);
	void PopulateSizeMenu(BMenu* menu);
	void PopulateAlignmentMenu(BMenu* menu);
	BMenuItem* FindItem(BMenu* menu, const char* key, const char* value);
	BMenuItem* FindFontItem(const char* name);
	BMenuItem* FindSizeItem(float value);
	BMenuItem* FindAlignmentItem(const char* value);
	BMessage* FindMarked(BMenu* menu);
	void WriteMessage(BMessage* msg);
	void Notify(uint32 what);
	void EnableFreeTextControls(bool enable);
	bool mSendNotification;
	BLooper *mLooper;
	GlobalSettings *mSettings;
	BPoint mWindowPos;
	MStringView *mLabel, *mDate;
	MTextView *mContents;
	MPopup* mFont;
	MPopup* mSize;
	MPopup* mAlignment;
	Annotation* mAnnotation;
	bool mEditable;
};

#endif
