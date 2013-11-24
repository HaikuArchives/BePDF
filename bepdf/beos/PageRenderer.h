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

#ifndef _PAGE_RENDERER_H
#define _PAGE_RENDERER_H

#include <Bitmap.h>
#include <Looper.h>
#include <Handler.h>
#include <View.h>

#include <XRef.h>
#include <PDFDoc.h>
#include "Annotation.h"
#include "BeSplashOutputDev.h"

class CachedPage;

// renders a pdf page in its own thread
class PageRenderer {
public:
	void DrawAnnotations(BView* view, bool edit);

protected:
	void DeletePDFLink();
	void GetSize(int pageNo, float *width, float *height, int32 zoom);
	void ResizeBitmap(float width, float height);
	void SetPDFPage(int index, bool valid, float width, float height);
	void Draw(int page, int pageNo, float left, float top);	

	Links *CreateLinks(int pageNo);
	void DrawAnnotations();
	// callback from rendering thread; does the rendering
	void Render();
	static void RedrawCallback(void *data, int left, int top, int right, int bottom, bool composited);
	static GBool AbortCheckCallback(void *data);
	static GBool AnnotDisplayDecideCallback(Annot *annot, void *data);
	void Notify(uint32 what);

	BString      mGSDriver;

	BString       *mOwnerPassword;
	BString       *mUserPassword;
	PDFDoc        *mDoc;
	BView         *mOffscreenView;
	float          mWidth, mHeight;
	BeSplashOutputDev   *mOutputDev;
	BLooper       *mLooper;
	BHandler      *mHandler;
	color_space    mColorSpace;
#if 0
	PageMode       mPageMode;
#endif	
	// temporary fields valid while rendering
	struct {
		bool valid;
		float width, height;
	} mPDFPage[2];
	
	thread_id   mRenderingThread;
	CachedPage *mPage;
	BBitmap    *mBitmap;
	int         mPageNo;
	int         mZoom;
	int         mRotate;
	bool        mEditAnnot;
	bool        mDoRendering;

	AcroForm   *mAcroForm;
	// the annotation of the current document
	AnnotsList   mAnnotations;
	Annotations* GetAnnotations();
	
public:
	PageRenderer();	
	~PageRenderer();
	AnnotsList* GetAnnotsList() { return &mAnnotations; }
	Annotations* GetAnnotationsForPage(int pageNo);
	AcroForm* GetAcroForm() { return mAcroForm; }

	// have to be set, before Start() may be called
    void NewFile(entry_ref *ref);
	void SetDoc(PDFDoc *doc, AcroForm* acroForm);
	void StartDoc(color_space colorSpace);
	void SetPassword(BString *owner, BString *user);
	
	void SetListener(BLooper *looper, BHandler *handler);
	
	enum {
		// page contents has change; update required
		UPDATE_MSG = 'PRup',
		// sent to listener when page has been rendered
		FINISH_MSG = 'PRfi',
		// sent to listener when rendering process has been aborted
		ABORT_MSG = 'PRab'
	};
	
	// start rendering of a page asynchronosly
	// returns an unique identifier in id (id is greater than or equal to zero)
	void Start(CachedPage *page, int pageNo, int zoom, int rotate, thread_id *id, bool editAnnot);
	// abort rendering process asynchronosly
	void Abort();
	// waits for rendering process to finish; returns immediatly when no process runs
	void Wait();
	
	friend int32 page_rendering_thread(void *data);
	
	static void GetParameter(BMessage *msg, thread_id *id, BBitmap **bitmap);
	
	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }
	BeSplashOutputDev *GetOutputDev() const { return mOutputDev; }
	
#if 0	
	enum PageMode {
		ONE_PAGE,
		TWO_PAGES
	};
	
	void SetPageMode(PageMode mode);
	PageMode GetPageMode() const;
#endif
};

#endif