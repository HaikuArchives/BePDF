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

#ifndef PRINT_SETTINGS_WINDOW_H
#define PRINT_SETTINGS_WINDOW_H

// xpdf
#include <XRef.h>
#include <PDFDoc.h>
// BeOS
#include <Looper.h>
// layout
#include <MPopup.h>
#include <MRadioGroup.h>
#include <MStringView.h>
#include <MTextControl.h>
#include <MWindow.h>

#include "Settings.h"

class PrintSettingsWindow : public MWindow {
	PDFDoc *mDoc;
	BLooper *mLooper;
	GlobalSettings *mSettings;
	int32 mZoomValue;
	MTextControl *mZoom;
	
	MButton      *mPrint;
	MTextControl *mPage;
	MStringView  *mWidth, 
	             *mHeight;
	             
	enum {
		MSG_SELECTION_CHANGED  = 'Sele',
		MSG_ORDER_CHANGED      = 'Orde',
		MSG_DPI_CHANGED        = 'Dpi',
		MSG_ROTATION_CHANGED   = 'Rota',
		MSG_ZOOM_CHANGED       = 'Zoom',
		MSG_COLOR_MODE_CHANGED = 'CMod',
		MSG_PRINT              = 'Prin',	
		MSG_PAGE_CHANGED       = 'Page'
	};
	
	void GetPageSize(uint32 page);
	
	char* MakeLabel(BString& string, const char* text);
	void AddItem(MPopup* popup, const char* label, uint32 what);

public:
	enum {
		QUIT_NOTIFY  = 'PSeQ',
		PRINT_NOTIFY = 'PSeP'
	};
	PrintSettingsWindow(PDFDoc *doc, GlobalSettings *settings, BLooper *looper);
	void Refresh(PDFDoc *doc);
	void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
	virtual void FrameMoved(BPoint point);
	virtual void FrameResized(float w, float h);
};

#endif
