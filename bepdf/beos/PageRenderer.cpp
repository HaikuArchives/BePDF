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
#include <Message.h>

#include <gtypes.h>
#include <Object.h>
#include <Gfx.h>
#include <PSOutputDev.h>

#include <be/support/String.h>
#include <be/support/StopWatch.h>
#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <be/storage/Path.h>
#include <Debug.h>

#include "BepdfApplication.h" // for images only
#include "PageRenderer.h"
#include "CachedPage.h"
#include "BePDF.h"
#include "Annotation.h"
#include "AnnotationRenderer.h"

#define MEASURE_RENDERING_TIME 0

inline static float RealSize (float x, float zoomDPI)
{
	return zoomDPI / 72 * x;
}

///////////////////////////////////////////////////////////////////////////

static SplashColorPtr getPaperColor() {
	static SplashColor color;
	color[0] = 255;
	color[1] = 255;
	color[2] = 255;
	return color;
}

void PageRenderer::RedrawCallback(void *data, int left, int top, int right, int bottom, bool composited) {
	PageRenderer* renderer = static_cast<PageRenderer*>(data);
	BView* view = renderer->mOffscreenView;
	BeSplashOutputDev* outputDev = renderer->mOutputDev;
	outputDev->redraw(left, top, view, left, top, right-left+1, bottom-top+1, composited);
	renderer->Notify(UPDATE_MSG);
}

GBool PageRenderer::AbortCheckCallback(void *data) {
	PageRenderer* renderer = static_cast<PageRenderer*>(data);
	return !renderer->mDoRendering;
}

GBool PageRenderer::AnnotDisplayDecideCallback(Annot *annot, void *data) {
	return false;
}

///////////////////////////////////////////////////////////////////////////
PageRenderer::PageRenderer() : 
	mOwnerPassword(NULL),
	mUserPassword(NULL),
	mDoc(NULL), 
	mOffscreenView(
		new BView(BRect(0, 0, 100, 100), "", B_FOLLOW_NONE, B_WILL_DRAW | B_SUBPIXEL_PRECISE)),
	mOutputDev(new BeSplashOutputDev(gFalse, 
		getPaperColor(), 
		gTrue, // incremental
		RedrawCallback, this)),
	mLooper(NULL), 
	mHandler(NULL),
	mRenderingThread(-1),
	mPage(NULL),
	mBitmap(NULL),
	mAcroForm(NULL)
	/* mPageMode(ONE_PAGE) */
{ 
	mOutputDev->startDoc(NULL);	
}

///////////////////////////////////////////////////////////////////////////
PageRenderer::~PageRenderer() {
	delete mOutputDev;
	delete mOffscreenView;
	delete mOwnerPassword;
	delete mUserPassword;
}

///////////////////////////////////////////////////////////////////////////
void PageRenderer::SetDoc(PDFDoc *doc, AcroForm* acroForm) {
	mDoc = doc; 
	mAcroForm = acroForm;
	mAnnotations.SetSize(doc->getNumPages());
	mOutputDev->startDoc(doc->getXRef());
}

void PageRenderer::SetPassword(BString *owner, BString *user) {
	delete mOwnerPassword; mOwnerPassword = owner ? new BString(*owner) : NULL;
	delete mUserPassword;  mUserPassword  = user  ? new BString(*user)  : NULL;
}

///////////////////////////////////////////////////////////////////////////
void PageRenderer::StartDoc(color_space colorSpace) {
	mColorSpace = colorSpace;
	if (mDoc != NULL) {
		// flush font engine / cache
		mOutputDev->startDoc(mDoc->getXRef());
	}
}


#if 0
///////////////////////////////////////////////////////////////////////////
void PageRenderer::SetPageMode(PageMode mode) {
	mPageMode = mode;
}

///////////////////////////////////////////////////////////////////////////
PageRenderer::PageMode PageRenderer::GetPageMode() const {
	return mPageMode;
}
#endif

///////////////////////////////////////////////////////////////////////////
void PageRenderer::SetListener(BLooper *looper, BHandler *handler) {
	mLooper = looper; mHandler = handler;
}

///////////////////////////////////////////////////////////////////////////
void PageRenderer::SetPDFPage(int index, bool valid, float width, float height) {
	if ((index >= 0) && (index <= 1)) {
		mPDFPage[index].valid = valid;
		mPDFPage[index].width = width;
		mPDFPage[index].height = height;
	}
}

///////////////////////////////////////////////////////////////////////////
void PageRenderer::GetSize(int pageNo, float *width, float *height, int32 zoom) {
	// determine width and height
	*width = ceil(RealSize (mDoc->getPageCropWidth (pageNo), zoom));
	*height = ceil(RealSize (mDoc->getPageCropHeight (pageNo), zoom));

	if ((mDoc->getPageRotate(pageNo) == 90) ||
		(mDoc->getPageRotate(pageNo) == 270)) {
		float h = *width; *width = *height; *height = h;
	}

	if ((mRotate == 90) || (mRotate == 270)) {
		float h = *width; *width = *height; *height = h;
	}
}

///////////////////////////////////////////////////////////////////////////
void PageRenderer::ResizeBitmap(float width, float height) {
	// (re-)create bitmap
	mBitmap = mPage->GetBitmap();
	if (!mBitmap || (mColorSpace != mBitmap->ColorSpace()) || (width > mPage->GetWidth()) || (height > mPage->GetHeight())) {
		delete mBitmap;
		
		mBitmap = new BBitmap(BRect (0, 0, width, height), 
								mColorSpace, true, false);
		mPage->SetBitmap(mBitmap, width, height);
	} else {
		mPage->SetBitmapSize(width, height);
	}
}

///////////////////////////////////////////////////////////////////////////
// prototype
int32 page_rendering_thread(void *data);

void PageRenderer::Start(CachedPage *page, int pageNo, int zoom, int rotation, thread_id *id, bool editAnnot) {
	// stop thread
	if (mRenderingThread != -1) {
		Abort();
		Wait();
	}

	mZoom = zoom;
	mPage = page;
	mEditAnnot = editAnnot;
	mPageNo = pageNo; mZoom = zoom; mRotate = rotation;
	
	GetSize(pageNo, &mWidth, &mHeight, mZoom);
	
	SetPDFPage(0, true, mWidth, mHeight);
#if 0	
	if ((mPageMode == TWO_PAGES) && (pageNo + 1 <= mDoc->getNumPages())) {
		float width, height;
		GetSize(pageNo + 1, &width, &height);
		SetPDFPage(1, true, width, height);
		mWidth += width; mHeight = max_c(mHeight, height);
	} else {
		SetPDFPage(1, false, 0, 0);
	}
#endif
	ResizeBitmap(mWidth, mHeight);
	
	mPage->MakeEmpty();
	mPage->SetState(CachedPage::RENDERING);
	
	// start new thread
	mDoRendering = true;
	mRenderingThread = spawn_thread(page_rendering_thread, "page_rendering_thread", B_NORMAL_PRIORITY, this);
	*id = mRenderingThread;
	resume_thread(mRenderingThread);
}

///////////////////////////////////////////////////////////////////////////
void PageRenderer::Abort() {
	mDoRendering = false;
}
///////////////////////////////////////////////////////////////////////////
void PageRenderer::Wait() {
	if (mRenderingThread != -1) {
		status_t status;
		wait_for_thread(mRenderingThread, &status);
		mRenderingThread = -1;
	}
}

///////////////////////////////////////////////////////////////////////////
int32 page_rendering_thread(void *data) {
	PageRenderer *pr = (PageRenderer*)data;
	pr->Render();
	return 0;
}

void PageRenderer::Draw(int page, int pageNo, float left, float top) {
#if MEASURE_RENDERING_TIME
	BStopWatch *timer = new BStopWatch("Renderer");
#endif
	// set view to right position and size
	mOffscreenView->MoveTo(left, top);
	mOffscreenView->ResizeTo(mPDFPage[page].width, mPDFPage[page].height);
	// attach view
	mBitmap->AddChild(mOffscreenView);
	// render pdf page

	mDoc->displayPage (mOutputDev, pageNo, 
		mZoom, mZoom, // h/v DPI 
		mRotate, 
		gFalse, // use media box
		gFalse, // crop
		gTrue, // printing
		AbortCheckCallback, this, AnnotDisplayDecideCallback, this);
	mOffscreenView->Sync ();
	// detach offscreen view
	mOffscreenView->RemoveSelf();
#if MEASURE_RENDERING_TIME
	delete timer;
#endif
}



Annotations* PageRenderer::GetAnnotationsForPage(int pageNo) {
	int i = pageNo-1;
	if (mAnnotations.Get(i) == NULL) {
		Object annotsDict;
		mDoc->getCatalog()->getPage(pageNo)->getAnnots(&annotsDict);
		mAnnotations.Set(i, new Annotations(&annotsDict, mAcroForm));
		annotsDict.free();
	}
	return mAnnotations.Get(i);
}


Annotations* PageRenderer::GetAnnotations() {
	return GetAnnotationsForPage(mPageNo);
}


void PageRenderer::DrawAnnotations(BView* view, bool edit) {
	AnnotationRenderer ar(view, mPage->GetCTM(), mZoom, edit);
	mPage->GetAnnotations()->Iterate(&ar);
}

void PageRenderer::DrawAnnotations() {
	if (!mEditAnnot) {
		// attach view
		mBitmap->AddChild(mOffscreenView);
	
		DrawAnnotations(mOffscreenView, false);	
		AnnotSorter s;
		mPage->GetAnnotations()->Sort(&s);
		
		mOffscreenView->Sync();
		// detach offscreen view
		mOffscreenView->RemoveSelf();
	}
}

Links *PageRenderer::CreateLinks(int pageNo) {
	Page *page = mDoc->getCatalog()->getPage(pageNo);
	Object obj;
	Links *links = new Links(page->getAnnots(&obj), mDoc->getCatalog()->getBaseURI());
	obj.free();
	return links;
}

void PageRenderer::Render() {
	gPdfLock->Lock();
	mBitmap->Lock ();
	mPage->SetAnnotations(GetAnnotations());
	// attach offscreen view
	mOffscreenView->MoveTo(0, 0);
	mOffscreenView->ResizeTo(mWidth, mHeight);
	mBitmap->AddChild(mOffscreenView);
	// fill page with background color
	mOffscreenView->SetHighColor (255, 255, 255);
	mOffscreenView->FillRect (BRect(0, 0, mWidth, mHeight));
	mOffscreenView->Sync();
	// detach offscreen view
	mOffscreenView->RemoveSelf();
	Draw(0, mPageNo, 0, 0);
	mPage->InitCTM(mOutputDev);
	mPage->SetLinks(CreateLinks(mPageNo));
	mPage->SetText(mOutputDev->acquireText());
	DrawAnnotations();	
#if 0	
	if (mDoRendering && mPDFPage[1].valid) {
		Draw(1, mPageNo+1, mPDFPage[0].width, 0);
	}
#endif
	mBitmap->Unlock ();
	
	// notify listener
	uint32 what;
	if (!mDoRendering) {
		mPage->SetState(CachedPage::WAITING);
		what = ABORT_MSG;
	} else {
		mPage->SetState(CachedPage::READY);
		what = FINISH_MSG;
	}
	gPdfLock->Unlock();

	Notify(what);
}

void PageRenderer::Notify(uint32 what) {
	BMessage msg(what);
	msg.AddInt32("bepdf:id", mRenderingThread);
	msg.AddPointer("bepdf:bitmap", mBitmap);
#ifdef DEBUG
	if (B_OK != mLooper->PostMessage(&msg))
		fprintf(stderr, "Error Sending Message %4.4s!\n", (char*)&msg.what);
#else
	mLooper->PostMessage(&msg);
#endif
}

void PageRenderer::GetParameter(BMessage *msg, thread_id *id, BBitmap **bitmap) {
	if (B_OK != msg->FindInt32("bepdf:id", id))
		*id = -1;
	if (B_OK != msg->FindPointer("bepdf:bitmap", (void**)bitmap))
		*bitmap = NULL;
}



