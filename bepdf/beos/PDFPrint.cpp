/*  
	BeOS Front-end du PDF file reader xpdf.
    Copyright (C) 1998 Hubert Figuiere
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

// BeOS
#include <be/interface/PrintJob.h>
// xpdf
#include <Object.h>
#include <Gfx.h>
// BePDF
#include "BepdfApplication.h"
#include "PDFView.h"
#include "PrintingProgressWindow.h"
#include "BePDF.h"
#include "AnnotationRenderer.h"

///////////////////////////////////////////////////////////////////////////
/*
	based up Be sample code in "Be Newsletter Volume 2, Issue 18 -- May 6, 1998"
*/

status_t
PDFView::PageSetup()
{
	status_t result = B_ERROR;

	BPrintJob  printJob(this->mTitle->String());


	if (mPrintSettings != NULL) {
		/* page setup has already been run */
		printJob.SetSettings(new BMessage(*mPrintSettings));
	}

	result = printJob.ConfigPage();

	if (result == B_NO_ERROR) {
	
		delete mPrintSettings;
		mPrintSettings = printJob.Settings();
	}

	return result;
}


///////////////////////////////////////////////////////////////////////////
class PrintView : public BView {
public:
	PrintView(PDFView *view, PDFDoc *mDoc, PageRenderer* pageRenderer, BMessage *printSettings, const char *title, BRect rect);

	void SetPage(int32 page);
	void Draw(BRect updateRect);
	friend int32 printing_thread(void *data);
	
	void Print();

private:
	static void RedrawCallback(void *data, int left, int top, int right, int bottom, bool composited);
	void Redraw(int left, int top, int right, int bottom, bool composited);
	static GBool AbortCheckCallback(void *data);

	PDFView *mView;
	PDFDoc *mDoc;
	bool mColorMode;
	PageRenderer *mPageRenderer;
	BeSplashOutputDev *mOutputDev;
	int mPageWidth; // of print page
	int mPageHeight; // of print page
	int mSliceY;
	int mSliceHeight;
	int32 mCurrentPage;
	char *mTitle;
	int mZoom;
	double mScale;
	int mRotation;
	int16 mPrintSelection;
	int16 mPrintOrder;
	BMessage *mPrintSettings;
	BRect mRect;
	PrintingProgressWindow *mProgressWindow;
};

///////////////////////////////////////////////////////////////////////////
PrintView::PrintView(PDFView *view, PDFDoc *doc, PageRenderer* pageRenderer, BMessage *printSettings, const char *title, BRect rect) :
	BView (BRect(1000, 1000, 1000+rect.Width(), 1000+rect.Height()), "print_view", B_FOLLOW_NONE, B_WILL_DRAW) {
	GlobalSettings *s = gApp->GetSettings();
	mView = view; // PDFView
	mDoc = doc; 
	mPageRenderer = pageRenderer;
	mPrintSettings = printSettings;
	mTitle = (char*)title;
	mZoom = s->GetZoomPrinter(); 
	mRotation = (int)s->GetRotationPrinter();
	mRect = rect;
	mScale = s->GetDPI() / 72.0;
	mPrintSelection = s->GetPrintSelection();
	mPrintOrder = s->GetPrintOrder();

	SplashColor backgroundColor;
	backgroundColor[0] = 255;
	backgroundColor[1] = 255;
	backgroundColor[2] = 255;
	
	BeSplashOutputDev::ColorMode colorMode;
	mColorMode = s->GetPrintColorMode() == GlobalSettings::PRINT_COLOR_MODE;
	if (mColorMode) {
		colorMode = BeSplashOutputDev::kColorMode;
	} else {
		colorMode = BeSplashOutputDev::kGrayScaleMode;
	}
	
	mOutputDev = new BeSplashOutputDev(gFalse, backgroundColor, gFalse, RedrawCallback, this, colorMode);
	
	mOutputDev->startDoc(NULL);
	mOutputDev->startDoc(doc->getXRef());
}


///////////////////////////////////////////////////////////////////////////
GBool PrintView::AbortCheckCallback(void *data) {
	PrintView *printView = static_cast<PrintView*>(data);
	PrintingProgressWindow *progress = printView->mProgressWindow;
	return progress->Stopped() || progress->Aborted();
}

///////////////////////////////////////////////////////////////////////////
void PrintView::RedrawCallback(void *data, int left, int top, int right, int bottom, bool composited) {
	PrintView *printView = (PrintView*)data;
	printView->Redraw(left, top, right, bottom, composited);
}

#define SLICE 1

void PrintView::Redraw(int left, int top, int right, int bottom, bool composited) {
#if SLICE
	mOutputDev->redraw(0, 0, this, 0, mSliceY, mPageWidth, mSliceHeight, composited);
#else
	mOutputDev->redraw(0, 0, this, 0, 0, mPageWidth, mPageHeight, composited);
#endif
}

///////////////////////////////////////////////////////////////////////////
void PrintView::SetPage(int32 page) {
	mCurrentPage = page;
}

///////////////////////////////////////////////////////////////////////////
void
PrintView::Draw(BRect updateRect)
{
	if (Window()->Lock()) {
		// PDFLock lock;
		int32 zoomDPI = mZoom * 72 / 100;
		double dpi = mScale * zoomDPI;
#if SLICE
		// About 4 MB per slice (color mode requires 4 bytes per pixel;
		// monochrome mode requires 1 bytes per pixel).
		// Note because xpdf rasterizes into a bitmap and when the
		// rendered slice is drawn into the view, this it is converted to a
		// BBitmap actually twice as much memory is allocated temporary.
		const int64 maxSize = 
			mColorMode ? 
				1024 * 1024 : 
				4 * 1024 * 1024;
		int64 slices = mPageWidth * (int64)mPageHeight / maxSize;
		if (slices <= 0) {
			slices = 1;
		}		
		mSliceHeight = mPageHeight / slices;
		if (mSliceHeight <= 0) {
			mSliceHeight = 1;
		}
		for (mSliceY = 0; mSliceY < mPageHeight; mSliceY += mSliceHeight) {
			if (mSliceY + mSliceHeight > mPageHeight) {
				mSliceHeight = mPageHeight - mSliceY;
			}
		
			// fprintf(stderr, "sliceY %d sliceHeight %d\n",
			//	mSliceY, mSliceHeight);
			
			if (mSliceHeight <= 0) {
				break;
			}
		
			mDoc->displayPageSlice (mOutputDev, mCurrentPage, 
				dpi, dpi, // h/v DPI
				mRotation, 
				gFalse, // use media box
				gFalse, // crop
				gTrue, // printing
				0, // slice X
				mSliceY,
				mPageWidth, // slice width
				mSliceHeight,
				AbortCheckCallback, this);
			
		}
#else
		mDoc->displayPage (mOutputDev, mCurrentPage, 
			dpi, dpi, // h/v DPI
			mRotation, 
			gFalse, // use media box
			gFalse, // crop
			gTrue, // printing
			AbortCheckCallback, this);
#endif		
		
		Annotations* annots = mPageRenderer->GetAnnotationsForPage(mCurrentPage);
		{ // Don't remove this block; ar-dtor must be called after Iterate()!
			AnnotationRenderer ar(this, mOutputDev->getDefCTM(), mScale * zoomDPI, false);
			annots->Iterate(&ar);
		}
		Flush();
		Window()->Unlock();
	}
}


// printing thread
int32 printing_thread(void *data) {
	PrintView *view = (PrintView*)data;
	view->Print();
	delete view;
	return 0;
}


void PrintView::Print() {
	BPrintJob printJob(mTitle);
	printJob.SetSettings(new BMessage(*mPrintSettings));
	PrintingProgressWindow *progress = NULL;
	PrintingHiddenWindow *hiddenWin = NULL;
	
	if (printJob.ConfigJob() == B_OK) {
		int32  curPage = 1;
		int32  firstPage;
		int32  lastPage;
		int32  pagesInDocument;
		BRect  pageRect = printJob.PrintableRect();

		pagesInDocument = mDoc->getNumPages ();
		firstPage = printJob.FirstPage();
		lastPage = printJob.LastPage();
		if (firstPage < 1) {
			firstPage = 1;
		}
		if (lastPage > pagesInDocument) {
			lastPage = pagesInDocument;
		}
		
		if (mScale == 0) { // set DPI to maximum of printer resolution
		int32 xdpi, ydpi;
			printJob.GetResolution(&xdpi, &ydpi);
			// Max. 300 DPI otherwise we might run out of memory
			// and freeze BeOS!
			// TODO Change if/when Zeta/Haiku can handle more memory!
			if (xdpi > 300) {
				xdpi = 300;
			}
			if (ydpi > 300) {
				ydpi = 300;
			}
#ifdef MORE_DEBUG
			fprintf(stderr, "print resolution= %d %d\n", xdpi, ydpi);
#endif
			if (xdpi > 0 && ydpi > 0) {
				if (xdpi > ydpi) {
					mScale = xdpi / 72;
				} else {
					mScale = ydpi / 72;
				}
			} else {
				mScale = 300 / 72; // default
			}
		}
		
		bool normalOrder = mPrintOrder == GlobalSettings::NORMAL_PRINT_ORDER;
		int16 incr = (mPrintSelection == GlobalSettings::PRINT_ALL_PAGES) ? 1 : 2;
		int32 pages = 0;

		switch (mPrintSelection) {
			case GlobalSettings::PRINT_ALL_PAGES:
				pages = lastPage - firstPage + 1;
				break;
			case GlobalSettings::PRINT_EVEN_PAGES:
				if (firstPage % 2 == 1) firstPage ++;
				if (lastPage % 2 == 1) lastPage --;
				pages = (lastPage - firstPage) / 2 + 1;
				break;
			case GlobalSettings::PRINT_ODD_PAGES:
				if (firstPage % 2 == 0) firstPage ++;
				if (lastPage % 2 == 0) lastPage --;
				pages = (lastPage - firstPage) / 2 + 1;
				break;
		}

		if (normalOrder) {
			curPage = firstPage;
		} else {
			curPage = lastPage;
			incr = -incr;
		}
		
		hiddenWin = new PrintingHiddenWindow(BRect(-100, -100, -10, -10));
		mProgressWindow = progress = new PrintingProgressWindow(mTitle, mRect, pages);
		if (hiddenWin->Lock()) {
			hiddenWin->AddChild(this);
			SetScale(1.0 / mScale);
			hiddenWin->Unlock();
		}

		int32 zoomDPI = mZoom * 72 / 100;
		zoomDPI = (int32) (zoomDPI * mScale);

		printJob.BeginJob();	

		for (; ((normalOrder && (curPage <= lastPage)) || (!normalOrder && (curPage >= firstPage))) && !progress->Stopped(); curPage += incr) {
			SetPage(curPage);
			progress->SetPage(curPage);
			float width = RealSize (mDoc->getPageCropWidth (curPage), zoomDPI);
			float height = RealSize (mDoc->getPageCropHeight (curPage), zoomDPI);

			if ((mDoc->getPageRotate(curPage) == 90) ||
				(mDoc->getPageRotate(curPage) == 270)) {
				float h = width; width = height; height = h;
			}

			if ((mRotation == 90) || (mRotation == 270)) {
				float h = width; width = height; height = h;
			}
			
			mPageWidth = (int)width;
			mPageHeight = (int)height;
			BRect curPageRect(0, 0, width, height);
			// center page
			BPoint origin((pageRect.Width() - width / mScale) / 2,
							(pageRect.Height() - height / mScale) / 2);

			printJob.DrawView(this, curPageRect, origin);
			
			printJob.SpoolPage();
			if (!printJob.CanContinue() || progress->Aborted()) {

				if (hiddenWin->Lock()) {
					hiddenWin->RemoveChild(this);
					hiddenWin->Unlock();
				}
				goto catastrophic_exit;
			}
		}
		if (hiddenWin->Lock()) {
			hiddenWin->RemoveChild(this);
			hiddenWin->Unlock();
		}

		printJob.CommitJob();
	}

catastrophic_exit:
	if (progress != NULL) progress->PostMessage(B_QUIT_REQUESTED);
	if (hiddenWin != NULL) hiddenWin->PostMessage(B_QUIT_REQUESTED);
	delete mOutputDev;
	
	// restore the page 
	BWindow *w = mView->Window();
	if (w->Lock()) {
		mView->RestartDoc();
		w->Unlock();
	}
}

///////////////////////////////////////////////////////////////////////////
void 
PDFView::Print()
{
	if (mPrintSettings == NULL && PageSetup() != B_NO_ERROR) {
		return;
	}
	
	PrintView *pView = new PrintView(this, mDoc, &mPageRenderer, mPrintSettings, 
		mTitle->String(), 
		Bounds());

	thread_id tid = spawn_thread(printing_thread, "printing_thread", B_NORMAL_PRIORITY, pView);
	resume_thread(tid);
}

