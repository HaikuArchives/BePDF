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

#ifndef _PAGE_CACHE_H
#define _PAGE_CACHE_H

// xpdf
#include<XRef.h>
#include<Link.h>
#include<TextOutputDev.h>
#include<PDFDoc.h>
// BeOS
#include<Bitmap.h>
#include<SupportDefs.h>
#include<Looper.h>

#include "CachedPage.h"

class PageCache { 
public:
	PageCache();
	virtual ~PageCache();

	void SetDoc(PDFDoc *doc);
	void StartDoc(bool useT1lib, bool t1libAA, bool t1libAAHigh, bool useFreeType, bool freeTypeAA, color_space colorSpace);
	void EndDoc();
	
	void SetListener(BLooper *looper, BHandler *handler);

	// start rendering of a page asynchronosly
	// returns an unique identifier in id (id is greater than or equal to zero)
	void Start(CachedPage *page, int pageNo, int zoom, int rotate, GBool doLinks, thread_id *id);
	// abort rendering process asynchronosly
	void Abort();
	// waits for rendering process to finish; returns immediatly when no process runs
	void Wait();
/*	
	void DisplayPage(int32 page, int32 zoom, int32 rotation, BBitmap **bitmap);
	void RenderPage(int32 page, int32 zoom, int32 rotation);

	// interrupt the page rendering process and 
	// releases the PDFDoc resource 
	void Stop(bool waitForDisplayedPage = false);
	// restart the page rendering process
	void Start();
	// empty the page cache
	void Empty();
*/
protected:
	PDFDoc *mDoc;
	CachedPage mPage[3];
};

#endif