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
#include <PDFDoc.h>
#include <XRef.h>
// BeOS
#include <Looper.h>
#include <Window.h>

#include "Settings.h"

class BTextControl;
class BButton;
class BStringView;
class BPopUpMenu;
class BGridLayout;

class PrintSettingsWindow : public BWindow {
	PDFDoc *mDoc;
	BLooper *mLooper;
	GlobalSettings *mSettings;
	int32 mZoomValue;
	BTextControl *mZoom;
	
	BButton      *mPrint;
	BTextControl *mPage;
	BStringView  *mWidth, 
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
	
	const char* MakeLabel(const char* text);
	void AddItem(BPopUpMenu* popup, const char* label, uint32 what);
	BPopUpMenu* MakePopup(const char *label, BGridLayout *layout, int32 &row);

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
