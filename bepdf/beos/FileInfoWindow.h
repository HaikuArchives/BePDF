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

#ifndef FILE_INFO_WINDOW_H
#define FILE_INFO_WINDOW_H

#include <time.h>
// xpdf
#include <XRef.h>
#include <PDFDoc.h>
#include <GfxFont.h>
// BeOS
#include <be/storage/Entry.h>
#include <be/app/Looper.h>
#include <be/support/List.h>
#include <be/support/String.h>
// layout
#include <MWindow.h>
#include <MRadioGroup.h>
#include <HGroup.h>
#include <MListView.h>
#include <MBorder.h>
#include <MBViewWrapper.h>
// Santa's Gift Bag
#include <ColumnListView/ColumnListView.h>
#include <ColumnListView/CLVColumn.h>
#include <ColumnListView/CLVListItem.h>
#include <VGroup.h>

#include "Settings.h"

class PDFDoc;

class FileInfoWindow : public MWindow {
	BLooper *mLooper;
	GlobalSettings *mSettings;
	BView *mView;
	
	static const int16 noKeys;
	static const char *systemKeys[9];
	
	enum  {
		NORMAL, // shows fonts of current page
		QUERY_ALL_FONTS, // query is in progress
		ALL_FONTS, // shows all fonts of current document
		STOP, // query progress stopped by user
		QUIT // 
	} mState;
	
	// font list
	ColumnListView *mFontList;
	MBViewWrapper *mFontListWrapper;
	struct Font {
		Ref ref;
		BString name;
		Font(Ref r, GString *s) {
			ref = r; 
			if (s) {
				name = s->getCString();
			}
		};
	};
	bool AddFont(BList *list, GfxFont *font);
	BListItem *FontItem(GfxFont *font);
	void QueryFonts(PDFDoc *doc, int page);
	void AddPair(VGroup *l, VGroup *r, BView *lv, BView *rv);	 
	void CreateProperty(VGroup *l, VGroup *r, Dict *dict, const char *key, const char *title);
	
	MButton *mShowAllFonts, *mStop;
	MBorder *mFontsBorder;

public:
	static const char *authorKey, *creationDateKey, *modDateKey, *creatorKey, 
		*producerKey, *titleKey, *subjectKey, *keywordsKey, *trappedKey;
	
	static bool IsSystemKey(const char *key);	
	static BString *GetProperty(Dict *dict, const char *key, time_t *time = NULL);

	enum {
		// notify main window that this window quits
		QUIT_NOTIFY = 'FInQ',
		
		START_QUERY_ALL_FONTS_MSG = 'FIsf',
		// 
		FONT_QUERY_STOPPED_NOTIFY = 'FQst',
		SHOW_ALL_FONTS_MSG = 'Show',
		STOP_MSG = 'Stop'
	};
	FileInfoWindow(GlobalSettings *settings, BEntry *file, PDFDoc *doc, BLooper *looper, int page);
	// new document
	void Refresh(BEntry *file, PDFDoc *doc, int page);
	// new page
	void RefreshFontList(BEntry *file, PDFDoc *doc, int page);
	// search for all fonts in all pages
	void QueryAllFonts(PDFDoc *doc);
	
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
	virtual void FrameMoved(BPoint point);
	virtual void FrameResized(float w, float h);
};

#endif
