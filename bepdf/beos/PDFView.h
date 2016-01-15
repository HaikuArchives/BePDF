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

#ifndef _PDFVIEW_H_
#define _PDFVIEW_H_

#include <be/interface/Bitmap.h>
#include <be/interface/Menu.h>
#include <be/interface/View.h>

#include "Page.h"
#include "XRef.h"
#include "Catalog.h"

#include "PDFDoc.h"
#include "BeSplashOutputDev.h"
#include "History.h"
#include "FindTextWindow.h"
#include "PageRenderer.h"

class PDFWindow;
class CachedPage;
class Annotation;

#define MIN_ZOOM	0
#define MAX_ZOOM	10

#define ZOOM_DPI_MIN  29
// 360 / 72 = 500%
#define ZOOM_DPI_MAX 360  

inline float RealSize (float x, float zoomDPI)
{
	return zoomDPI / 72 * x;
}

class PDFView
	: public BView
{
private:
	bool mLoading;
	PDFDoc * mDoc;
	BePDFAcroForm* mBePDFAcroForm;
	bool mOk;
	int mZoom;
	BBitmap * mBitmap;
	CachedPage *mPage;
	int mCurrentPage;
	float mRotation;
	PageRenderer mPageRenderer;
	BString *mOwnerPassword;
	BString *mUserPassword;

	color_space mColorSpace;
	
	bool mInvertVerticalScrolling;

	BString *mTitle;
	float mLeft, mTop;	// position of page inside the view
	float mWidth, mHeight;		//document width and height
	LinkAction *mLinkAction;
	Annotation *mAnnotation;
	Annotation *mAnnotInEditor;
	History mHistory;
	enum {
		kNotInHistory, kInHistory
	} mNavigationState;
	
	BCursor *mViewCursor;
	enum mouse_action {
		NO_ACTION,
		MOVE_ACTION,
		SELECT_ACTION,
		DND_ACTION,
		ZOOM_ACTION,
		RESIZE_ANNOT_ACTION,
		MOVE_ANNOT_ACTION
	} mMouseAction;
	BPoint mMousePosition;
	bool mDragStarted;
	bool mEditAnnot;
	bool mResizeVertOnly;
	PDFRectangle mAnnotStartRect;
	Annotation*  mInsertAnnot;	
	
	float mMouseWheelDY;
	enum { 
		MOUSE_WHEEL_THRESHHOLD = 2 
	};

	thread_id mRendererID;
	bool mRendering;

	enum { 
		NOT_SELECTED = 0,
		DO_SELECTION = 1, 
		SELECTED = 2
	} mSelected;
	bool mFilledSelection;
	
	BPoint mSelectionStart;
	BRect mSelection;
	
	BMessage * mPrintSettings;

	// find
	bool mStopFindThread;
	
	BPoint CorrectMousePos(const BPoint point);
	PDFPoint CvtDevToUser(BPoint dev);
	BPoint CvtUserToDev(PDFPoint user);
	BRect  CvtUserToDev(PDFRectangle* user);
	void OnMouseWheelChanged(BMessage *msg);

	PDFWindow* GetPDFWindow();
	void SetStatus(const char* s);
	
	void SaveFileAttachment(BMessage* msg);
	
public:
	PDFView (entry_ref * ref, FileAttributes *fileAttributs, BRect frame,
								const char *name,
								uint32 resizeMask,
								uint32 flags, 
								const char *ownerPassword,
								const char *userPassword,
								bool *encrypted);
	virtual ~PDFView();
	
	void SetPassword(const char *owner, const char *user);
	GString *ConvertPassword(const char *password);
	
	void EndDoc();
	
	void UpdatePanelDirectory(BPath* path);
	void MakeTitleString(BPath* path);
	
	bool OpenFile(entry_ref *ref, const char *ownerPassword, const char *userPassword, bool *encrypted);
	bool LoadFile(entry_ref *ref, FileAttributes *fileAttributs, const char *ownerPassword, const char *userPassword, bool init, bool *encrypted);
	void SetViewCursor(BCursor *cursor, bool sync = true);
	void LoadFileSettings(entry_ref* ref, FileAttributes *fileAttributes, float& left, float& top);
	
	void RestoreWindowFrame(BWindow* w);
	
	bool InPage(BPoint p);  // NOT USED
	BPoint LimitToPage(BPoint p);
	
	void DrawAnnotations(BRect updateRect);
	void DrawPage(BRect updateRect);
	void DrawBackground(BRect updateRect);
	void DrawSelection(BRect updateRect);
	virtual	void Draw (BRect updateRect);
	
	virtual void FrameResized (float width, float height);
	virtual void AttachedToWindow ();
	
	void SkipMouseMoveMsgs();
	virtual void KeyDown (const char * bytes, int32 numBytes);
	void SetAction(mouse_action action);
	
	void CurrentDate(BString& date);
	bool OnAnnotation(BPoint p);
	bool OnAnnotResizeRect(BPoint p, bool& vertOnly);
	void InsertAnnotation(BPoint where, bool* hasFixedSize);
	void AnnotMoveOrResize(BPoint point, bool annotInserted, bool fixedSize);
	bool AnnotMouseDown(BPoint point, uint32 button);
	void MoveAnnotation(BPoint point);
	void ResizeAnnotation(BPoint point);
	bool AnnotMouseMoved (BPoint point, uint32 transit, const BMessage *msg);
	bool AnnotMouseUp (BPoint point);
	
	uint32 GetButtons();
	virtual void MouseDown (BPoint point);
	void ScrollIfOutside (BPoint point);
	void ResizeSelection (BPoint point);
	void InitViewCursor(uint32 transit);
	virtual void MouseMoved (BPoint point, uint32 transit, const BMessage *msg);
	virtual void MouseUp (BPoint point);
	virtual void ScrollTo (BPoint point);
	void ScrollTo(float x, float y);
	virtual void MessageReceived(BMessage *msg);
	LinkAction* OnLink(BPoint p);
	void LinkToString(LinkAction* action, BString* string);
	BMenuItem* AddAnnotItem(BMenu* menu, const char* label, uint32 what);
	void ShowAnnotPopUpMenu(BPoint point);
	void ShowPopUpMenu(BPoint point, LinkAction* action);
	void CopyText(BString *str);
	bool IsOk() { return mOk; }

	void SetPage (int page);

	void MoveToPage (int page, bool top = true);
	void MoveToPage (int num, int gen, bool top = true);
	void MoveToPage (const char *string, bool top = true);
	int Page()      { return mCurrentPage; } ;
	
	// history
	void BeginHistoryNavigation();
	void EndHistoryNavigation();
	void RecordHistory();
	void RecordHistory(entry_ref ref, const char* owner, const char* user);
	void RestoreHistory();
	void Back();
	void Forward();
	bool CanGoBack()    { return mHistory.CanGoBack(); }
	bool CanGoForward() { return mHistory.CanGoForward(); }
		
	void SetZoom ( int zoom );
	void Zoom(bool zoomIn);
	void FitToPageWidth();
	void FitToPage();
	
	int16 GetZoomDPI() const;
	void SetRotation ( float rot );
	void RotateClockwise();
	void RotateAntiClockwise();
	void Redraw(PDFDoc *doc = NULL);
	void PostRedraw(thread_id id, BBitmap *bitmap);
	void RedrawAborted(thread_id id, BBitmap *bitmap);
	void WaitForPage(bool abort = false);
	// Rerender this page with new parameters for font renderer and colorspace
	void RestartDoc();
	
	// called when size of window changes
	void Resize();
	void CenterPage();	
	void FixScrollbars ();

	int GetNumPages() 		    { return mDoc->getNumPages(); };
	int GetPageWidth(int page)  { return (int)mDoc->getPageCropWidth (page); };
	int GetPageHeight(int page) { return (int)mDoc->getPageCropHeight (page); };
		
	status_t PageSetup();
	void Print();
	void SetPrintingDpi(int dpi);
	
	bool IsLinkToPDF(LinkAction* action, BString* path);
	bool HandleLink(BPoint point);
	void GotoDest(LinkDest* dest);
	void DisplayLink(BPoint point);
	
	void Find(const char *s, bool ignoreCase, bool backward, FindTextWindow *findWindow);
	void StopFind();
	
	void Dump(); // called from BeOutputDev
	friend class PrintView;
	
	void SelectionChanged();
	void SetSelection(int xMin, int yMin, int xMax, int yMax, bool display = false);
	void GetSelection(int &xMin, int &yMin, int &xMax, int &yMax);
	void CopySelection();
	void SelectAll();
	void SelectNone();
	void SetFilledSelection(bool filled);
	
	// caller must delete returned string object
	BString *GetSelectedText();
	void SendDragMessage(uint32 protocol);
	void SendDataMessage(BMessage *msg);
	
	void ScrollVertical(bool down, float by);
	void ScrollHorizontal(bool right, float by);
	
	void SetColorSpace(color_space colorSpace);

	void SetInvertVerticalScrolling(bool reverse) { mInvertVerticalScrolling = reverse; }
	
	PDFDoc* GetPDFDoc() { return mDoc; }
	CachedPage* GetPage() { return mPage; }
	PageRenderer* GetPageRenderer() { return &mPageRenderer; }
	bool HasSelection() { return mSelected != NOT_SELECTED; }
	
	void UpdateSettings(GlobalSettings* settings);
	
	// Annotation
	BePDFAcroForm* GetBePDFAcroForm() { return mBePDFAcroForm; }
	void BeginEditAnnot();
	void InsertAnnotation(Annotation* a);
	void ClearAnnotationWindow();
	void SyncAnnotation(bool clearWindow);
	void UpdateAnnotation(Annotation* a, const char* contents, const char* font, float size, const char* align);
	void UpdateAnnotation(Annotation* a, BMessage* data);
	void EndEditAnnot();
	bool EditingAnnot() const { return mEditAnnot; }
	void ShowAnnotWindow(bool editable, bool updateOnly = false);
};

#endif
