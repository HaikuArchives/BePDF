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

#include <stdio.h>
#include <math.h>
// BeOS
#include <locale/Catalog.h>

#include <be/app/Application.h>
#include <be/app/Clipboard.h>
#include <be/app/Looper.h>
#include <be/app/MessageQueue.h>
#include <be/app/Roster.h>

#include <be/interface/ScrollBar.h>
#include <be/interface/PrintJob.h>
#include <be/interface/Alert.h>
#include <be/interface/StringView.h>
#include <be/interface/PopUpMenu.h>
#include <be/interface/MenuItem.h>

#include <be/storage/Path.h>
#include <be/storage/Entry.h>
#include <be/storage/Directory.h>
#include <be/storage/File.h>
#include <be/storage/NodeInfo.h>
#include <be/translation/BitmapStream.h>
#include <be/translation/TranslationUtils.h>
#include <be/translation/TranslatorRoster.h>
#include <be/support/String.h>
#include <be/support/Debug.h>

// xpdf
#include <TextOutputDev.h>
#include <Gfx.h>
#include <gfile.h>
// BePDF
#include "AnnotationWindow.h"
#include "AnnotWriter.h"
#include "BePDF.h"
#include "BepdfApplication.h"
#include "CachedPage.h"
#include "FileInfoWindow.h"
#include "FindTextWindow.h"
#include "PageRenderer.h"
#include "PDFWindow.h"
#include "PDFView.h"
#include "PrintingProgressWindow.h"
#include "ResourceLoader.h"
#include "SaveThread.h"
#include "StatusWindow.h"
#include "TextConversion.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PDFView"

// zoom factor is 1.2 (similar to DVI magsteps)
#if 0
static const int kZoomDPI[MAX_ZOOM - MIN_ZOOM + 1] = {
	29, 35, 42, 50, 60,
	72,
	86, 104, 124, 149, 179
};
#else
static const int kZoomDPI[MAX_ZOOM - MIN_ZOOM + 1] = {
	18, 24, 36, 48, 54,
	72,
	90, 108, 127, 144, 216
};
#endif

#define OPEN_FILE_MSG                  'open'
#define COPY_LINK_MSG                  'cplk'
#define DELETE_ANNOT_MSG               'dele'
#define PROPERTIES_ANNOT_MSG           'prp'
#define EDIT_ANNOT_MSG                 'edit'
#define SAVE_FILE_ATTACHMENT_ANNOT_MSG 'save'

///////////////////////////////////////////////////////////////////////////
PDFView::PDFView (entry_ref* ref, FileAttributes *fileAttributes,
	const char *name, uint32 flags, const char *ownerPassword,
	const char *userPassword, bool *encrypted)
	: BView(name, flags)
{
	GlobalSettings *settings = gApp->GetSettings();
	SetViewColor(B_TRANSPARENT_COLOR);
	// init member variables
	mDoc = NULL; mBePDFAcroForm = NULL;
	mOk = false;
	mZoom = settings->GetZoom();
	mBitmap = NULL;
	mPage = new CachedPage();
	mCurrentPage = 0;
	mRotation = settings->GetRotation(); // 0.0f;
	mOwnerPassword = mUserPassword = NULL;
	SetPassword(ownerPassword, userPassword);

	mColorSpace = B_RGB32;

	mInvertVerticalScrolling = settings->GetInvertVerticalScrolling();

	mTitle = NULL;
	mLeft = mTop = 0;
	mWidth = 100; mHeight = 100;
	mLinkAction = NULL;
	mAnnotation = NULL;
	mAnnotInEditor = NULL;
	mNavigationState = kNotInHistory;

	mViewCursor = NULL;
	mMouseAction = NO_ACTION;
	mMousePosition.Set(0, 0);
	mDragStarted = false;
	mEditAnnot = false;
	mInsertAnnot = NULL;

	mMouseWheelDY = 0;

	mRendererID = -1;
	mRendering = false;

	mSelected = NOT_SELECTED;
	mFilledSelection = settings->GetFilledSelection();

	mPrintSettings = NULL;

	#if JAPANESE_SUPPORT
	SetJapaneseFont(settings->GetJapaneseFontFamily(), settings->GetJapaneseFontStyle());
	#endif

	#if CHINESE_CNS_SUPPORT
	SetChineseTFont(settings->GetChineseTFontFamily(), settings->GetChineseTFontStyle());
	#endif

	#if CHINESE_GB_SUPPORT
	SetChineseSFont(settings->GetChineseSFontFamily(), settings->GetChineseSFontStyle());
	#endif

	#if KOREAN_SUPPORT
	SetKoreanFont(settings->GetKoreanFontFamily(), settings->GetKoreanFontStyle());
	#endif

	mPageRenderer.SetPassword(mOwnerPassword, mUserPassword);

	if (LoadFile(ref, fileAttributes, ownerPassword, userPassword, true, encrypted)) {
		SetViewCursor(gApp->handCursor, true);
		mOk = true;
	}
}

PDFWindow*
PDFView::GetPDFWindow() {
	return dynamic_cast<PDFWindow*>(Window());
}

///////////////////////////////////////////////////////////////////////////
void PDFView::SetPassword(const char* ownerPassword, const char* userPassword) {
	delete mOwnerPassword; mOwnerPassword = ownerPassword ? new BString(ownerPassword) : NULL;
	delete mUserPassword; mUserPassword = userPassword ? new BString(userPassword) : NULL;
}

///////////////////////////////////////////////////////////////////////////
GString* PDFView::ConvertPassword(const char* password) {
	GString *pwd = NULL;
	if (password != NULL) {
		BString *s = ToAscii(password);
		if (s) {
			pwd = new GString(s->String());
			delete s;
		}
	}
	return pwd;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::EndDoc() {
	mSelected = NOT_SELECTED;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::UpdatePanelDirectory(BPath* path) {
	BPath directory;
	if (strcmp(path->Path(), gApp->DefaultPDF()->Path()) != 0 &&
		B_OK == path->GetParent(&directory)) {
		// don't set path to default pdf file
		gApp->GetSettings()->SetPanelDirectory(directory.Path());
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::MakeTitleString(BPath* path) {
	delete mTitle;
	mTitle = new BString("Rog BePDF: ");

	Object obj;
	if (mDoc->getDocInfo(&obj) && obj.isDict()) {
		Dict *dict = obj.getDict();
		BString *s = FileInfoWindow::GetProperty(dict, FileInfoWindow::titleKey);
		if (s) {
			*mTitle << *s << " (" << path->Leaf() << ")";
			delete s;
		} else
			*mTitle << path->Leaf();
	} else
		*mTitle << path->Leaf();
	obj.free();
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::OpenFile(entry_ref *ref, const char *ownerPassword, const char *userPassword, bool *encrypted) {
	BEntry entry (ref, true);
	BPath path;
	entry.GetPath (&path);

	GString *fileName = new GString ((char*)path.Path ());
	GString *owner = ConvertPassword(ownerPassword);
	GString *user  = ConvertPassword(userPassword);

	PDFDoc *newDoc = new PDFDoc (fileName, owner, user, NULL);
	delete owner; delete user;

	UpdatePanelDirectory(&path);

	bool ok = newDoc->isOk();
	// xpdf 3.01 returns false even PDF file is password protected?!?
	*encrypted = true; // newDoc->isEncrypted();
//	fprintf(stderr, "ok %s encrypted %s\n",
//		ok ? "yes" : "no",
//		(*encrypted) ? "yes" : "no");
	if (ok) {
		delete mDoc;
		mDoc = newDoc;
		MakeTitleString(&path);
	} else {
		delete newDoc;
	}
	return ok;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::LoadFileSettings(entry_ref* ref, FileAttributes* fileAttributes, float& left, float& top) {
	GlobalSettings *s = gApp->GetSettings();
	if (fileAttributes->Read(ref, s) && s->GetRestorePageNumber()) {
		mCurrentPage = fileAttributes->GetPage();
		if (mCurrentPage > mDoc->getNumPages()) {
			mCurrentPage = mDoc->getNumPages();
		}
		mZoom = s->GetZoom();
		mRotation = s->GetRotation();
		fileAttributes->GetLeftTop(left, top);
	} else {
		left = top = 0;
		mCurrentPage = 1;
		fileAttributes->SetPage(mCurrentPage);
		fileAttributes->SetLeftTop(left, top);
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::RestoreWindowFrame(BWindow* w) {
	GlobalSettings* s = gApp->GetSettings();
	if (s->GetRestoreWindowFrame()) {
		// restore window position and size
		w->MoveTo(s->GetWindowPosition());
		float width, height;
		s->GetWindowSize(width, height);
		w->ResizeTo(width, height);
	}
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::LoadFile(entry_ref *ref, FileAttributes *fileAttributes, const char *ownerPassword, const char *userPassword, bool init, bool *encrypted) {
	BString s(B_TRANSLATE("BePDF reading file: "));
	s += ref->name;
	ShowLoadProgressStatusWindow statusWindow(s.String());
	EndDoc();

	SetPassword(ownerPassword, userPassword);
	mPageRenderer.SetPassword(mOwnerPassword, mUserPassword);
	WaitForPage(true);

	// We use the application thread to load a PDF file.
	// To keep the window responsive while loading, we unlock the window lock
	// and have to ensure that the window thread does not access data
	// that is being loaded (Draw() just fills the entire view with a background color).
	mLoading = true;
	bool isLocked = Window()->IsLocked();
	if (isLocked) {
		Invalidate();
		Window()->Unlock();
	}
	bool opened = OpenFile(ref, ownerPassword, userPassword, encrypted);
	if (isLocked) Window()->Lock();
	mLoading = false;
	mPageRenderer.StartDoc(mColorSpace);
	if (!opened) {
		// show previous document
		if (Window()->Lock()) {
			Invalidate();
			Window()->Unlock();
		}
		return false;
	}
	delete mBePDFAcroForm;
	mBePDFAcroForm = new BePDFAcroForm(mDoc->getXRef(),
		mDoc->getCatalog()->getAcroForm());
	mPageRenderer.SetDoc(mDoc, mBePDFAcroForm);
	BepdfApplication::UpdateFileAttributes(mDoc, ref);

	float left, top;
	LoadFileSettings(ref, fileAttributes, left, top);

	RecordHistory(*ref, ownerPassword, userPassword);

	PDFWindow *w = GetPDFWindow();
	if (w && !init && w->Lock()) {
		RestoreWindowFrame(w);
		w->NewDoc(mDoc);
		w->SetTitle (mTitle->String());
		Redraw();
		ScrollTo(left, top);
		w->Unlock();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
PDFView::~PDFView()
{
	delete mDoc;
	delete mTitle;
	delete mPage;
	delete mOwnerPassword;
	delete mUserPassword;
}

///////////////////////////////////////////////////////////////////////////
void PDFView::MessageReceived(BMessage *msg) {
	BString string;
	switch (msg->what) {
	case B_SIMPLE_DATA: {
			entry_ref ref;
			if (B_OK == msg->FindRef("refs", 0, &ref)) {
				be_app->RefsReceived(msg);
				return;
			}
		}
		break;
	case B_COPY_TARGET:
		SendDataMessage(msg);
		break;
	case B_MOUSE_WHEEL_CHANGED:
		OnMouseWheelChanged(msg);
		break;
	case COPY_LINK_MSG:
		if (B_OK == msg->FindString("link", &string)) {
			CopyText(&string);
		}
		break;
	case OPEN_FILE_MSG:
		if (B_OK == msg->FindString("file", &string)) {
			PDFWindow::Launch(string.String());
		}
		break;
	case DELETE_ANNOT_MSG:
		{
			void* p;
			if (msg->FindPointer("annot", &p) == B_OK && p == mAnnotation) {
				if (p == mAnnotInEditor) mAnnotInEditor = NULL;
				mAnnotation->SetDeleted(true);
				Invalidate(CvtUserToDev(mAnnotation->GetRect()));
				ClearAnnotationWindow();
			}
			mAnnotation = NULL;
		}
		break;
	case EDIT_ANNOT_MSG:
		{
			PDFWindow* win = GetPDFWindow();
			if (win) {
				win->EditAnnotation(!mEditAnnot);
			}
		}
		break;
	case SAVE_FILE_ATTACHMENT_ANNOT_MSG:
		{
			PDFWindow* win = GetPDFWindow();
			if (win == NULL) {
				break;
			}

			FileAttachmentAnnot* fileAttachment = dynamic_cast<FileAttachmentAnnot*>(mAnnotation);
			if (fileAttachment == NULL) {
				break;
			}

			BMessage msg(B_SAVE_REQUESTED);
			msg.AddPointer("fileAttachment", fileAttachment);
			gApp->OpenSaveFilePanel(this, NULL, &msg, fileAttachment->GetFileName());
		}
		break;
	case PROPERTIES_ANNOT_MSG:
		ShowAnnotWindow(true);
		break;
	case B_SAVE_REQUESTED:
		SaveFileAttachment(msg);
		break;
	default:
		BView::MessageReceived(msg);
	}
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::InPage(BPoint p) {
	return p.x >= 0.0 && p.x < mWidth && p.y >= 0.0 && p.y < mHeight;
}

///////////////////////////////////////////////////////////////////////////
BPoint
PDFView::LimitToPage(BPoint p) {
	if (p.x < 0) p.x = 0.0;
	else if (p.x > mWidth) p.x = mWidth;

	if (p.y < 0) p.y = 0.0;
	else if (p.y > mHeight) p.y = mHeight;
	return p;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::OnMouseWheelChanged(BMessage *msg) {
	float dy, dx;
	if (msg->FindFloat("be:wheel_delta_y", &dy) == B_OK && dy != 0.0) {
		bool down = dy > 0;
		// intelliMouse driver uses command key to simulate wheel_detla_x!
		if ((modifiers() & (B_COMMAND_KEY | B_OPTION_KEY))) {
			Zoom(!down); // zoom in / out
		} else if ((modifiers() & (B_SHIFT_KEY | B_CONTROL_KEY))) {
			// next/previous page
			MoveToPage(mCurrentPage + (down ? 1 : -1));
		} else {
			ScrollVertical(down, 0.20);
		}
	}
	if (msg->FindFloat("be:wheel_delta_x", &dx) == B_OK && dx != 0.0) {
		bool right = dx > 0;
		ScrollHorizontal(right, 0.20);
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::DrawAnnotations(BRect updateRect)
{
	if (!mRendering && mEditAnnot) {
		SetOrigin(mLeft, mTop);
		mPageRenderer.DrawAnnotations(this, mEditAnnot);
		SetOrigin(0, 0);
	}
}


///////////////////////////////////////////////////////////////////////////
void
PDFView::DrawPage(BRect updateRect)
{
	if (mBitmap == NULL) {
#ifdef DEBUG
		fprintf (stderr, "WARNING: PDFView::Draw() NULL bitmap\n");
#endif
	} else {
		DrawBitmap(mBitmap, BRect(0, 0, mWidth, mHeight), BRect(mLeft, mTop, mLeft + mWidth, mTop + mHeight));
		DrawAnnotations(updateRect);
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::DrawBackground(BRect updateRect)
{
	BRect rect(Bounds());
	float right = mLeft + mWidth, bottom = mTop + mHeight;
	SetLowColor(128, 128, 128, 0);
	if (rect.left < mLeft) {
		FillRect(BRect(rect.left, rect.top, mLeft - 2, rect.bottom), B_SOLID_LOW);
	}
	if (rect.top < mTop) {
		FillRect(BRect(rect.left, rect.top, rect.right, mTop - 2), B_SOLID_LOW);
	}
	if (right < rect.right) {
		FillRect(BRect(right + 2, rect.top, rect.right, rect.bottom), B_SOLID_LOW);
	}
	if (bottom < rect.bottom) {
		FillRect(BRect(rect.left, bottom + 2, rect.right, rect.bottom), B_SOLID_LOW);
	}

	SetLowColor(0, 0, 0, 0);
	StrokeRect(BRect(mLeft - 1, mTop - 1, right + 1, bottom + 1), B_SOLID_LOW);
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::DrawSelection(BRect updateRect)
{
	BRect selection(mSelection);
	selection.OffsetBy(mLeft, mTop);

	rgb_color fill_color = {0, 0, 255, 64}; // transparent blue
	SetHighColor(fill_color); // fill color for selection
	SetPenSize(1.0);

	switch (mSelected) {
		case DO_SELECTION:
			StrokeRect(selection);
			break;
		case SELECTED:
			SetDrawingMode(B_OP_ALPHA);
			if (mFilledSelection) {
				FillRect(selection);
			} else {
				StrokeRect(selection);
			}
			SetDrawingMode(B_OP_COPY);
			break;
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::Draw(BRect updateRect)
{
	if (mLoading) {
		SetLowColor(128, 128, 128, 0);
		FillRect(updateRect, B_SOLID_LOW);
	} else {
		DrawBackground(updateRect);
		DrawPage(updateRect);
		BRect rect(Bounds());
		if (GetPDFWindow()) {
			GetPDFWindow()->GetFileAttributes()->SetLeftTop(rect.left, rect.top);
		}
		DrawSelection(updateRect);
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::ScrollTo (BPoint point) {
	BView::ScrollTo(point);
	BPoint mouse; uint32 buttons;
	GetMouse(&mouse, &buttons);
	DisplayLink(mouse);
}

void
PDFView::ScrollTo(float x, float y) {
	BRect bounds(Bounds());
	float xMax = mWidth - bounds.Width();
	float yMax = mHeight - bounds.Height();

	if ((x < 0) || (mLeft > 0))
		x = 0;
	else if ((xMax > 0) && (x > xMax))
		x = xMax;

	if ((y < 0) || (mTop > 0))
		y = 0;
	else if ((yMax > 0) && (y > yMax))
		y = yMax;

	BView::ScrollTo(x, y);
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::FrameResized (float width, float height)
{
	Resize();
}


///////////////////////////////////////////////////////////////////////////
void
PDFView::AttachedToWindow ()
{
	Window()->SetTitle (mTitle->String());
	SetViewCursor(gApp->handCursor);
	mPageRenderer.SetListener(Window(), this);
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::ScrollVertical (bool down, float by) {
	BRect rect(Bounds ());
	float scrollBy = (by > 0) ? rect.Height() * by : -by;
	if (down) {
		if (rect.bottom < mHeight-1) {
			ScrollBy (0, scrollBy);
		} else {
			if (mCurrentPage != mDoc->getNumPages()) { // bottom of last page not reached
				MoveToPage(mCurrentPage + 1, true);
			}
		}
	} else { // up
		if (rect.top != 0) {
			ScrollBy (0, -scrollBy);
		} else {
			if (mCurrentPage != 1) { // top of first page not reached
				MoveToPage(mCurrentPage - 1, false);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::ScrollHorizontal (bool right, float by) {
	BRect rect(Bounds());
	float scrollBy = (by > 0) ? rect.Width() * by : -by;
	if (right) {
		if (rect.right < mWidth - 1) {
			ScrollBy (scrollBy, 0);
		}
	} else {
		if (rect.left != 0) {
			ScrollBy (-scrollBy, 0);
		}
	}
}
///////////////////////////////////////////////////////////////////////////
void
PDFView::KeyDown (const char * bytes, int32 numBytes)
{
	switch (*bytes) {
	case B_PAGE_UP:
		MoveToPage (mCurrentPage - 1);
		break;
	case B_SPACE:
	case B_ENTER:
	case B_BACKSPACE:
		ScrollVertical (*bytes != B_BACKSPACE, 0.95);
		break;
	case B_DOWN_ARROW:
	case B_UP_ARROW:
		ScrollVertical (*bytes == B_DOWN_ARROW, -20);
		break;
	case B_LEFT_ARROW:
	case B_RIGHT_ARROW:
		ScrollHorizontal(*bytes == B_RIGHT_ARROW, -20);
		break;
	case B_PAGE_DOWN:
		MoveToPage (mCurrentPage + 1);
		break;
	case B_HOME:
		MoveToPage (1);
		break;
	case B_END:
		MoveToPage (GetNumPages ());
		break;
	default:
		BView::KeyDown (bytes, numBytes);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
void PDFView::SetAction(mouse_action action) {
	mMouseAction = action;
}

///////////////////////////////////////////////////////////////////////////
void PDFView::SetViewCursor(BCursor *cursor, bool sync) {
	if (Window()->Lock()) {
		mViewCursor = cursor;
		BView::SetViewCursor(cursor, sync);
		Window()->Unlock();
	}
}

///////////////////////////////////////////////////////////////////////////
BPoint
PDFView::CorrectMousePos(const BPoint point) {
	BPoint p(point);
	p.x -= mLeft;
	p.y -= mTop;
	return p;
}

///////////////////////////////////////////////////////////////////////////
PDFPoint
PDFView::CvtDevToUser(BPoint dev) {
	double x, y;
	mPage->CvtDevToUser((int)dev.x, (int)dev.y, &x, &y);
	return PDFPoint(x, y);
}

///////////////////////////////////////////////////////////////////////////
BPoint
PDFView::CvtUserToDev(PDFPoint user) {
	int x, y;
	mPage->CvtUserToDev(user.x, user.y, &x, &y);
	return BPoint(x, y);
}

///////////////////////////////////////////////////////////////////////////
BRect
PDFView::CvtUserToDev(PDFRectangle* user) {
	// Note: Keep in sync with AnnotationRenderer::ToRect()
	BRect r;
	int x, y;
	mPage->CvtUserToDev(user->x1, user->y1, &x, &y);
	r.top = r.bottom = y;
	r.right = r.left = x;
	mPage->CvtUserToDev(user->x2, user->y2, &x, &y);
	if (y < r.top) r.top = y; else r.bottom = y;
	if (x < r.left) r.left = x; else r.right = x;
	r.top = floor(r.top);
	r.left = floor(r.left);
	r.bottom = ceil(r.bottom+1.0);
	r.right = ceil(r.right+1.0);
	// The next line exists not in AnnotationRenderer::ToRect():
	r.OffsetBy(mLeft, mTop);
	return r;
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::OnAnnotResizeRect(BPoint point, bool& vertOnly) {
	BRect r = CvtUserToDev(mAnnotation->GetRect());
	r.left = r.right - 5;
	if (r.Contains(point)) {
		r.top  = r.bottom - 5;
		vertOnly = !r.Contains(point);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::OnAnnotation(BPoint point) {
	if (mRendering) return false;

	BPoint p = CorrectMousePos(point);
	PDFPoint u = CvtDevToUser(p);
	Annotation* annot = mPage->GetAnnotations()->OverAnnotation(u.x, u.y, mEditAnnot);
	// XXX maybe allow deleting the annotation even if we can't write one
	if (!CanWrite(annot)) annot = NULL;
	mAnnotation = annot;
	if (mAnnotation) {
		bool vertOnly;
		if (OnAnnotResizeRect(point, vertOnly)) {
	  		if (vertOnly) {
	  			SetViewCursor(gApp->splitVCursor);
	  		} else {
	  			SetViewCursor(gApp->resizeCursor);
	  		}
		} else {
	  	SetViewCursor(gApp->pointerCursor);
	  	}
	} else {
		SetViewCursor(gApp->handCursor);
	}
	return mAnnotation != NULL;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::CurrentDate(BString& date) {
	GString s;
	AnnotUtils::CurrentDate(&s);
	date = s.getCString();
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::InsertAnnotation(BPoint where, bool* hasFixedSize) {
	BString date;
	CurrentDate(date);

	if (GetPDFWindow()) {
		GetPDFWindow()->ReleaseAnnotationButton();
	}

	mAnnotation = mInsertAnnot->Clone();
	mPage->GetAnnotations()->Append(mAnnotation);

	if (mAnnotation->GetRect()->x1 == -1) {
		mAnnotation->GetRect()->x1 = 0;
	} else {
		*hasFixedSize = true;
	}

	mAnnotation->MoveTo(CvtDevToUser(CorrectMousePos(where)));
	GString* t = Utf8ToUcs2(gApp->GetSettings()->GetAuthor());
	mAnnotation->SetTitle(t);
	delete t;
	mAnnotation->SetDate(date.String());

	PopupAnnot* popup = mAnnotation->GetPopup();
	if (popup) {
		popup->MoveTo(CvtDevToUser(CorrectMousePos(where)));
		// inherited from parent annotation:
		//popup->SetTitle(gApp->GetSettings()->GetAuthor());
		//popup->SetDate(date.String());
	}

	if (!*hasFixedSize) {
		SetViewCursor(gApp->resizeCursor);
	}
	SyncAnnotation(false);
	Invalidate(CvtUserToDev(mAnnotation->GetRect()));
	mInsertAnnot = NULL;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::AnnotMoveOrResize(BPoint point, bool annotInserted, bool fixedSize) {
	SetAction(MOVE_ANNOT_ACTION);
	SyncAnnotation(false);
	mAnnotStartRect = *mAnnotation->GetRect();
	mDragStarted = true;
	SetMouseEventMask(B_POINTER_EVENTS);
	mResizeVertOnly = false;
	if (fixedSize || (!annotInserted && !OnAnnotResizeRect(point, mResizeVertOnly))) {
		// move
  		PDFPoint p = CvtDevToUser(CorrectMousePos(point)) - mAnnotation->LeftTop();
		if (fixedSize) {
			float dx = (mAnnotation->GetRect()->x2 - mAnnotation->GetRect()->x1)/2;
			float dy = (mAnnotation->GetRect()->y1 - mAnnotation->GetRect()->y2)/2;
			p.x = dx;
			p.y = dy;
		}
		mMousePosition.Set(p.x, p.y);
	} else {
		// resize
		if (annotInserted) {
			// move point over resize rectangle
			BRect rect = CvtUserToDev(mAnnotation->GetRect());
			point.Set(rect.right, rect.bottom);
		}
		SetAction(RESIZE_ANNOT_ACTION);
		mMousePosition = point;
	}
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::AnnotMouseDown(BPoint point, uint32 buttons) {
	BPoint screen = ConvertToScreen(point);
	switch (buttons) {
		case B_PRIMARY_MOUSE_BUTTON:
			if (!mEditAnnot) return false;
			// move, resize or insert annotation
			{
				bool annotInserted = false;
				bool fixedSize = false;
				if (mInsertAnnot) {
					InsertAnnotation(point, &fixedSize);
					annotInserted = true;
				}
				ShowAnnotWindow(true, true);
				if (annotInserted || OnAnnotation(point)) {
					AnnotMoveOrResize(point, annotInserted, fixedSize);
					return true;
				} else {
					SetAction(NO_ACTION);
					return false;
				}
			}
			break;
		case B_SECONDARY_MOUSE_BUTTON:
			// context menu, copy is not implemented in annotation editing mode
			if (mAnnotation) {
				ShowAnnotPopUpMenu(screen);
				return true;
			}
			// don't allow copy in annotation editing mode
			if (mEditAnnot) {
				return true;
			}
			break;
		default:;
	}
	return mInsertAnnot != NULL;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::MoveAnnotation(BPoint point) {
	PDFPoint p = CvtDevToUser(CorrectMousePos(point)) - PDFPoint(mMousePosition.x, mMousePosition.y);
	if (!(mAnnotation->LeftTop() == p)) {
		BRect oldRect = CvtUserToDev(mAnnotation->GetRect());
		mAnnotation->MoveTo(PDFPoint(p.x, p.y));
		mAnnotation->SetChanged(true);
		BRect newRect = CvtUserToDev(mAnnotation->GetRect());
		BRect invRect = oldRect | newRect;
		Invalidate(invRect);
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::ResizeAnnotation(BPoint point) {
	mInsertAnnot = NULL;
	if (mResizeVertOnly) point.y = mMousePosition.y;
	PDFPoint delta = CvtDevToUser(mMousePosition - point);
	PDFPoint origin = CvtDevToUser(BPoint(0.0, 0.0));
	delta = origin - delta;
	PDFPoint end(delta.x + mAnnotStartRect.x2, delta.y + mAnnotStartRect.y1);
	BRect oldRect = CvtUserToDev(mAnnotation->GetRect());
	PDFPoint size(end.x - mAnnotStartRect.x1, mAnnotStartRect.y2 - end.y);
	size.x = max_c(size.x, 8);
	size.y = max_c(size.y, 8);
	mAnnotation->ResizeTo(size.x, size.y);
	mAnnotation->SetChanged(true);
	BRect newRect = CvtUserToDev(mAnnotation->GetRect());
	BRect invRect = oldRect | newRect;
	if (oldRect != newRect) Invalidate(invRect);
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::AnnotMouseMoved(BPoint point, uint32 transit, const BMessage* msg) {
	if (mEditAnnot && mMouseAction == NO_ACTION) {
		if (mInsertAnnot == NULL) OnAnnotation(point);
		return true;
	} else if (mInsertAnnot != NULL) {
		return true;
	} else {
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::AnnotMouseUp(BPoint point) {
	bool consumed;
	consumed = AnnotMouseMoved(point, 0, NULL);
	if (consumed) {
		SetAction(NO_ACTION);
	}
	return consumed;
}

///////////////////////////////////////////////////////////////////////////
uint32
PDFView::GetButtons() {
	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons, false);
	if (buttons == B_PRIMARY_MOUSE_BUTTON) {
		if ((modifiers() & B_CONTROL_KEY)) {
			buttons = B_SECONDARY_MOUSE_BUTTON; // simulate secondary button
		} else if ((modifiers() & B_SHIFT_KEY)) {
			buttons = B_TERTIARY_MOUSE_BUTTON; // simulate tertiary button
		}
	}
	return buttons;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::MouseDown (BPoint point) {
	LinkAction *action;
	BPoint screen;

	MakeFocus(true);
	uint32 buttons = GetButtons();
	screen = ConvertToScreen(point);
	if (AnnotMouseDown(point, buttons)) {
		return;
	}
	switch (buttons) {
		case B_PRIMARY_MOUSE_BUTTON:
			if ((mSelected == SELECTED) && mSelection.Contains(CorrectMousePos(point))) {
				SendDragMessage(B_MIME_DATA); // start text drag and drop
				break;
			}
			// follow link or move view
			SetAction(MOVE_ACTION);
			if (!HandleLink(point)) {
				mDragStarted = true;
				SetMouseEventMask(B_POINTER_EVENTS);
		  		SetViewCursor(gApp->grabCursor);
				mMousePosition = ConvertToScreen(point);
			} else {
				SetAction(NO_ACTION);
			}
			break;
		case B_SECONDARY_MOUSE_BUTTON:
			if ((mSelected == SELECTED) && mSelection.Contains(CorrectMousePos(point ))) {
				SendDragMessage(B_SIMPLE_DATA); // start negotiated drag and drop
				return;
			}
			if (mSelected == NOT_SELECTED) {
				if ((action = OnLink(point)) != NULL) {
					ShowPopUpMenu(screen, action);
					return;
				}
			}
			// text selection: fall through
		case B_TERTIARY_MOUSE_BUTTON: // zoom to selection
			if (mSelected != NOT_SELECTED) {
				mSelection.OffsetBy(mLeft, mTop);
				Invalidate(mSelection);
				mSelected = NOT_SELECTED;
			}

			// is copying allowed?
			if (buttons == B_SECONDARY_MOUSE_BUTTON && !mDoc->okToCopy()) {
				return;
			}

			SetAction(buttons == B_TERTIARY_MOUSE_BUTTON ? ZOOM_ACTION : SELECT_ACTION);

			mSelected = DO_SELECTION;
			mMousePosition = screen;
			point = CorrectMousePos(point);

			if (buttons == B_SECONDARY_MOUSE_BUTTON) {
			  	SetViewCursor(gApp->textSelectionCursor);
			  	point = LimitToPage(point);
			} else {
			  	SetViewCursor(gApp->zoomCursor);
			}
			mSelectionStart = point;
			mSelection.SetLeftTop(point);
			mSelection.SetRightBottom(point);
			SetMouseEventMask(B_POINTER_EVENTS);

			break;
	}
}


void
PDFView::ScrollIfOutside(BPoint point) {
	float x, y, r_min, r_max;
	BRect bounds(Bounds());

	BScrollBar *scroll = ScrollBar(B_VERTICAL);

	scroll->GetRange(&r_min, &r_max);

	if (point.x < bounds.left) { // scroll left
		x = point.x;
	} else if (point.x > bounds.right) { // scroll right
		x = point.x - bounds.Width();
	} else {
		x = bounds.left;
	}
	x = min_c(r_max, max_c(x, r_min));

	scroll = ScrollBar(B_VERTICAL);
	scroll->GetRange(&r_min, &r_max);
	if (point.y < bounds.top) { // scroll up
		y = point.y;
	} else if (point.y > bounds.bottom) { // scroll down
		y = point.y - bounds.Height();
	} else {
		y = bounds.top;
	}
	y = min_c(r_max, max_c(y, r_min));
	if ((x != bounds.left) || (y != bounds.top)) {
		ScrollTo(x, y);
	}
}


void
PDFView::ResizeSelection(BPoint point) {
	point = CorrectMousePos(point);
	BRect rect(mSelection);
	if (mMouseAction == SELECT_ACTION) point = LimitToPage(point);
	if (point.x < mSelectionStart.x) {
		mSelection.left = point.x; mSelection.right = mSelectionStart.x;
	} else {
		mSelection.left = mSelectionStart.x; mSelection.right = point.x;
	}

	if (point.y < mSelectionStart.y) {
		mSelection.top = point.y; mSelection.bottom = mSelectionStart.y;
	} else {
		mSelection.top = mSelectionStart.y; mSelection.bottom = point.y;
	}

	if (rect != mSelection) {
		rect = rect | mSelection;
		rect.OffsetBy(mLeft, mTop);
		Invalidate(rect);
	}
}

///////////////////////////////////////////////////////////////////////////
void PDFView::SkipMouseMoveMsgs() {
	BMessage *mouseMovedMsg;
	while ((mouseMovedMsg = Looper()->MessageQueue()->FindMessage(B_MOUSE_MOVED, 0)))
	{
		Looper()->MessageQueue()->RemoveMessage(mouseMovedMsg);
		delete mouseMovedMsg;
	}
}

void
PDFView::InitViewCursor(uint32 transit) {
	// FIXME: Where is the best place to set the initial Cursor of a view?
	if ((transit == B_ENTERED_VIEW) && (mViewCursor != NULL)) {
		if (Window()->Lock()) {
			BView::SetViewCursor(mViewCursor);
			Window()->Unlock();
			mViewCursor = NULL;
		}
	}
}

void
PDFView::MouseMoved (BPoint point, uint32 transit, const BMessage *msg) {
	#define UPDATE_INTERVAL 4
	int updateCounter = UPDATE_INTERVAL;

	InitViewCursor(transit);

	if (AnnotMouseMoved(point, transit, msg)) {
		return;
	}

	switch (mMouseAction) {
		case NO_ACTION:
			if (!mDragStarted)
				DisplayLink(point);
			break;
		case MOVE_ACTION: // move view
		{
			SkipMouseMoveMsgs();

			BPoint mousePosition = point;
			uint32 buttons = GetButtons();
			BPoint offset;
			float x, y, r_min, r_max;
			BScrollBar *scroll;

			point = ConvertToScreen(mousePosition);
			offset = point - mMousePosition;
			if (mInvertVerticalScrolling) {
				offset.y = mMousePosition.y - point.y;
			}
			mMousePosition = point;

			scroll = ScrollBar(B_HORIZONTAL);
			scroll->GetRange(&r_min, &r_max);
			x = min_c(r_max, max_c(scroll->Value() - offset.x, r_min));

			scroll = ScrollBar(B_VERTICAL);
			scroll->GetRange(&r_min, &r_max);
			y = min_c(r_max, max_c(scroll->Value() - offset.y, r_min));

			ScrollTo(x, y);

			if ((buttons & B_PRIMARY_MOUSE_BUTTON) == 0) {
				MouseUp(mousePosition);
			}
			break;
		}
		case SELECT_ACTION: // text selection
		case ZOOM_ACTION: // zoom to selection
		case MOVE_ANNOT_ACTION:
		case RESIZE_ANNOT_ACTION:
		 	while(true) {
				SkipMouseMoveMsgs();

				uint32 buttons;

				ScrollIfOutside(point);

				switch (mMouseAction) {
					case SELECT_ACTION:
					case ZOOM_ACTION:
						ResizeSelection(point);
						break;
					case MOVE_ANNOT_ACTION:
						MoveAnnotation(point);
						break;
					case RESIZE_ANNOT_ACTION:
						ResizeAnnotation(point);
						break;
					default:;
				}

				GetMouse(&point, &buttons, false);
				if (buttons == 0) {
					MouseUp(point);
					return;
				}
				if (updateCounter == UPDATE_INTERVAL) {
					Window()->UpdateIfNeeded();
					updateCounter = 0;
				} else {
					updateCounter ++;
				}
				snooze(10000);
			}
			break;
		default:;
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::MouseUp (BPoint point) {
	if (AnnotMouseUp(point)) {
		mDragStarted = false;
	} else {
		if (mMouseAction == SELECT_ACTION) { // copy selection
			if (mSelection.Width() * mSelection.Height() * mSelection .Height() > 200) {
				mSelected = SELECTED;
				Invalidate(mSelection.OffsetByCopy(mLeft, mTop));
				CopySelection();
			} else {
				mSelected = NOT_SELECTED;
				Invalidate(mSelection.OffsetByCopy(mLeft, mTop));
			}
		} else if (mMouseAction == ZOOM_ACTION) { // zoom to selection
			mSelected = NOT_SELECTED;
			Invalidate(mSelection.OffsetByCopy(mLeft, mTop));

			if (mSelection.Width() * mSelection .Height() > 200) {
				float a = mSelection.Width() + 1, b = mSelection.Height() + 1;
				BRect bounds(Bounds());
				float n = bounds.Width() + 1, m = bounds.Height() + 1;

				int32 zoomDPI = GetZoomDPI(), newZoomDPI;
				if (a / b > n / m) {
					newZoomDPI = (int32)(zoomDPI * n / a);
				} else {
					newZoomDPI = (int32)(zoomDPI * m / b);
				}

				if (newZoomDPI > ZOOM_DPI_MAX) newZoomDPI = ZOOM_DPI_MAX;
				float x = mSelection.left * newZoomDPI / zoomDPI,
					y = mSelection.top * newZoomDPI / zoomDPI;

				int i;
				for (i = MIN_ZOOM; i <= MAX_ZOOM; i++) {
					if (newZoomDPI == kZoomDPI[i]) {
						newZoomDPI = i; break;
					}
				}

				if (i > MAX_ZOOM)
					newZoomDPI = -newZoomDPI;

				if (mZoom != newZoomDPI) {
					PDFWindow* w = GetPDFWindow();
					if (w) w->SetZoom(newZoomDPI);
					SetZoom(newZoomDPI);
				}

				ScrollTo(x, y);
			}
		}

		SelectionChanged();

		if ((mMouseAction != NO_ACTION) || mDragStarted) {
			mDragStarted = false;
			DisplayLink(point);
			mMouseAction = NO_ACTION;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
LinkAction*
PDFView::OnLink(BPoint point) {
	double x, y;
	if (mRendering || (mDoc == NULL) || (mDoc->getNumPages() == 0)) return NULL;

	point = CorrectMousePos(point);

	// PDFLock lock;
	mPage->CvtDevToUser(point.x, point.y, &x, &y);
	return mPage->FindLink(x, y);
}

bool
PDFView::IsLinkToPDF(LinkAction* action, BString* path) {
	LinkDest *dest = NULL;
	GString *namedDest = NULL;
	GString *fileName;
	char *s;

	if (action->getKind() == actionGoToR) {
		dest = NULL;
		namedDest = NULL;
		if ((dest = ((LinkGoToR *)action)->getDest()))
			dest = dest->copy();
		else if ((namedDest = ((LinkGoToR *)action)->getNamedDest()))
			namedDest = namedDest->copy();
		s = ((LinkGoToR *)action)->getFileName()->getCString();
		if (isAbsolutePath(s))
			fileName = new GString(s);
		else
			fileName = appendToPath(grabPath(mDoc->getFileName()->getCString()), s);
		*path = fileName->getCString(); delete fileName;
		return true;
	} else if (action->getKind() == actionLaunch) {
		fileName = ((LinkLaunch *)action)->getFileName();
		s = fileName->getCString();
		if (!strcmp(s + fileName->getLength() - 4, ".pdf") ||
			!strcmp(s + fileName->getLength() - 4, ".PDF")) {

			if (isAbsolutePath(s))
				fileName = fileName->copy();
			else
				fileName = appendToPath(grabPath(mDoc->getFileName()->getCString()), s);
			*path = fileName->getCString(); delete fileName; return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
bool
PDFView::HandleLink(BPoint point) {
	LinkAction *action = NULL;
	LinkActionKind kind;
	LinkDest *dest = NULL;
	GString *namedDest = NULL;
	GString *fileName;
	GString *actionName;
	char *s;
	BString pdfFile;

	action = OnLink(point);
	if (mAnnotation) {
		LinkAnnot* link = dynamic_cast<LinkAnnot*>(mAnnotation);
		if(link == NULL) {
			ShowAnnotWindow(false);
			return true;
		} else {
			action = link->GetLinkAction();
		}
	}

	if (action != NULL) {
		// PDFLock lock;

		if (IsLinkToPDF(action, &pdfFile)) {
			RecordHistory();
			if ((modifiers() & B_COMMAND_KEY)) {
				PDFWindow::Launch(pdfFile.String());
			} else {
				PDFWindow::OpenInWindow(pdfFile.String());
			}
			return true;
		}

		switch (kind = action->getKind()) {

		// GoTo / GoToR action
		case actionGoTo:
		case actionGoToR:
			if (kind == actionGoTo) {
				dest = NULL;
				namedDest = NULL;
				if ((dest = ((LinkGoTo *)action)->getDest()))
					dest = dest->copy();
				else if ((namedDest = ((LinkGoTo *)action)->getNamedDest()))
					namedDest = namedDest->copy();
			}
			if (namedDest) {
				dest = mDoc->findDest(namedDest);
				delete namedDest;
			}
			if (!dest) {
				if (kind == actionGoToR)
					MoveToPage(1);
			} else {
				GotoDest(dest);
				delete dest;
				return true;
			}
			break;

			// Launch action
		case actionLaunch: {
			fileName = ((LinkLaunch *)action)->getFileName();
			s = fileName->getCString();
			fileName = fileName->copy();
			if (((LinkLaunch *)action)->getParams()) {
				fileName->append(' ');
				fileName->append(((LinkLaunch *)action)->getParams());
			}

			fileName->append(" &");

			BString string(B_TRANSLATE("Execute the command:"));
			string += fileName->getCString();
			string += "?";
			BAlert *dialog = new BAlert(B_TRANSLATE("BePDF: Launch"),
					 string.String(),
					 B_TRANSLATE("OK"), B_TRANSLATE("Cancel"));
			if (dialog->Go() == 0)
				system(fileName->getCString());
			delete dialog;
			delete fileName;
			return true;
			}

		// URI action
		case actionURI:
			if (GetPDFWindow()) {
				GetPDFWindow()->LaunchHTMLBrowser(((LinkURI *)action)->getURI()->getCString());
			}
			return true;

		// Named action
		case actionNamed:
			actionName = ((LinkNamed *)action)->getName();
			if (!actionName->cmp("NextPage")) {
				MoveToPage( mCurrentPage + 1 );
			} else if (!actionName->cmp("PrevPage")) {
				MoveToPage( mCurrentPage - 1 );
			} else if (!actionName->cmp("FirstPage")) {
				MoveToPage( 1 );
			} else if (!actionName->cmp("LastPage")) {
				MoveToPage( GetNumPages() );
			} else if (!actionName->cmp("GoBack")) {
				Back();
			} else if (!actionName->cmp("GoForward")) {
				Forward();
			} else if (!actionName->cmp("Quit")) {
				Window()->PostMessage(B_QUIT_REQUESTED);
			} else {
				// error(-1, "Unknown named action: '%s'", actionName->getCString());
			}
      break;
      	// TODO
      	case actionMovie: // fall through
		// unknown action type
		case actionUnknown:
			fprintf(stdout, B_TRANSLATE("Unknown link action type: '%s'"),
				((LinkUnknown *)action)->getAction()->getCString());
			break;
		}
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////
void
PDFView::GotoDest(LinkDest* dest) {
	int dx, dy;
	int pg;
	Ref pageRef;

	if (dest->isPageRef()) {
		pageRef = dest->getPageRef();
		pg = mDoc->findPage(pageRef.num, pageRef.gen);
	} else {
		pg = dest->getPageNum();
	}
	if (pg > 0 && pg != mCurrentPage)
		MoveToPage(pg);
	else if (pg <= 0)
		MoveToPage(1);
	switch (dest->getKind()) {
	case destXYZ:
		mPage->CvtUserToDev(dest->getLeft(), dest->getTop(), &dx, &dy);
		if (dest->getChangeLeft() || dest->getChangeTop()) {
			BRect bounds(Bounds());
			if (dest->getChangeLeft())
				bounds.left = dx;
			if (dest->getChangeTop())
				bounds.top = dy;
			ScrollTo(bounds.left, bounds.top);
		}
		//~ what is the zoom parameter?
		break;
	case destFit:
	case destFitB:
		//~ do fit
		ScrollTo(0, 0);
		break;
	case destFitH:
	case destFitBH:
		//~ do fit
		mPage->CvtUserToDev(0, dest->getTop(), &dx, &dy);
		ScrollTo(0, dy);
		break;
	case destFitV:
	case destFitBV:
		//~ do fit
		mPage->CvtUserToDev(dest->getLeft(), 0, &dx, &dy);
		ScrollTo(dx, 0);
		break;
	case destFitR:
		//~ do fit
		mPage->CvtUserToDev(dest->getLeft(), 0, &dx, &dy);
		ScrollTo(dx, dy);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
BMenuItem*
PDFView::AddAnnotItem(BMenu* menu, const char* label, uint32 what) {
	BMessage* msg = new BMessage(what);
	msg->AddPointer("annot", mAnnotation);
	BMenuItem* item = new BMenuItem(B_TRANSLATE(label), msg);
	menu->AddItem(item);
	item->SetTarget(this);
	return item;
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::ShowAnnotPopUpMenu(BPoint point) {
	ASSERT (mAnnotation != NULL);

	BMenuItem* item;
	BPopUpMenu* menu = new BPopUpMenu("PopUpMenu");
	menu->SetAsyncAutoDestruct(true);

	AddAnnotItem(menu, mEditAnnot ?
		B_TRANSLATE("Leave annotation editing mode") : B_TRANSLATE("Edit"), EDIT_ANNOT_MSG);

	item = AddAnnotItem(menu, B_TRANSLATE("Delete"), DELETE_ANNOT_MSG);
	item->SetEnabled(mEditAnnot);

	if (dynamic_cast<FileAttachmentAnnot*>(mAnnotation) != NULL) {
		menu->AddSeparatorItem();
		AddAnnotItem(menu, B_TRANSLATE("Save file attachment as" B_UTF8_ELLIPSIS),
			SAVE_FILE_ATTACHMENT_ANNOT_MSG);
	}

	menu->AddSeparatorItem();

	AddAnnotItem(menu, B_TRANSLATE("Properties"), PROPERTIES_ANNOT_MSG);

	point -= BPoint(10, 10);
	menu->Go(point, true, false, false);
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::ShowPopUpMenu(BPoint point, LinkAction* action) {
	BPopUpMenu* menu = new BPopUpMenu("PopUpMenu");
	menu->SetAsyncAutoDestruct(true);

	BMessage* msg;
	BMenuItem* i;
	BString s;

	// Open PDF file in new window
	if (IsLinkToPDF(action, &s)) {
		msg = new BMessage(OPEN_FILE_MSG);
		msg->AddString("file", s);
		i = new BMenuItem(B_TRANSLATE("Open in new window"), msg);
		i->SetTarget(this);
		menu->AddItem(i);
	}

	// Copy link location
	msg = new BMessage(COPY_LINK_MSG);
	LinkToString(action, &s);
	msg->AddString("link", s);
	i = new BMenuItem(B_TRANSLATE("Copy link"), msg);
	i->SetTarget(this);
	menu->AddItem(i);

	point -= BPoint(10, 10);
	menu->Go(point, true, false, false);
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::LinkToString(LinkAction* action, BString* string) {
	const char *s = NULL;
	char *t;
	BString str;
	LinkDest *dest;
	int pg;

	switch (action->getKind()) {
	case actionGoTo:
		s = B_TRANSLATE("[internal link]");
		dest = ((LinkGoTo *)action)->getDest();
		if (!dest) {
			PDFLock lock;
			dest = mDoc->findDest(((LinkGoTo *)action)->getNamedDest());
			if (!dest) break;
		}
		if (dest->isPageRef()) {
			Ref ref = dest->getPageRef();
			PDFLock lock;
			pg = mDoc->findPage(ref.num, ref.gen);
		} else {
			pg = dest->getPageNum();
		}
		s = B_TRANSLATE("Go to page %d");
		t = str.LockBuffer(strlen(s)+20);
		sprintf(t, s, pg);
		str.UnlockBuffer();
		s = str.String();
		break;
	case actionGoToR:
		s = ((LinkGoToR *)action)->getFileName()->getCString();
		break;
	case actionLaunch:
		s = ((LinkLaunch *)action)->getFileName()->getCString();
		break;
	case actionURI:
		s = ((LinkURI *)action)->getURI()->getCString();
		break;
	case actionNamed:
		s = ((LinkNamed *)mLinkAction)->getName()->getCString();
		break;
	case actionMovie:
		// TODO
		s = B_TRANSLATE("[link type 'movie' not supported]");
		break;
	case actionUnknown:
		s = B_TRANSLATE("[unknown link]");
		break;
	}
	*string = s;
}


void
PDFView::DisplayLink(BPoint point)
{
	LinkAction *action;
	BString str;
	if (mRendering || mDragStarted || (mDoc == NULL) ||
		(mDoc->getNumPages() == 0))
		return;

	BPoint p = CorrectMousePos(point);
	// over selection?
	if (((mSelected == SELECTED) && mSelection.Contains(p)) ||
		p.x < 0 || p.y < 0 || p.x >= mWidth || p.y >= mHeight) {
		SetViewCursor((BCursor*)B_CURSOR_SYSTEM_DEFAULT);
		return;
	}

	double x, y;
	mPage->CvtDevToUser((int)p.x, (int)p.y, &x, &y);
	Annotation* annot = mPage->GetAnnotations()->OverAnnotation(x, y, mEditAnnot);
	// over annotation?
	if (annot) {
		// new annotation?
		if (mAnnotation != annot) {
			mLinkAction = NULL;
			mAnnotation = annot;
			SetViewCursor(gApp->linkCursor);
			LinkAnnot* link = dynamic_cast<LinkAnnot*>(annot);
			if(link == NULL) {
				SetToolTip(B_TRANSLATE("Annotation"));
			} else {
				LinkToString(link->GetLinkAction(), &str);
				SetToolTip( str.String() );
			}
			ShowToolTip();
		}
		return;
	} else if (mAnnotation) {
		// moved out side of annotation
		SetToolTip("");
		HideToolTip();
		SetViewCursor(gApp->handCursor);
		mAnnotation = NULL;
	}

	// over link?
	if ((action = OnLink(point)) != NULL) {
		// new link?
		if (action != mLinkAction) {
			SetViewCursor(gApp->linkCursor);
			mLinkAction = action;
			LinkToString(action, &str);
			SetToolTip( str.String() );
			ShowToolTip();
		}
	} else {
		if (mLinkAction) {
			mLinkAction = NULL;
			SetToolTip("");
			HideToolTip();
		}
		SetViewCursor(gApp->handCursor);
	}
}
///////////////////////////////////////////////////////////////////////////
void
PDFView::FixScrollbars ()
{
	BRect frame = Bounds();
	BScrollBar * scroll;
	float x, y;
	float bigStep, smallStep;

	x = mWidth - frame.Width();
	if (x < 0.0) {
		x = 0.0;
	}
	y = mHeight - frame.Height();
	if (y < 0.0) {
		y = 0.0;
	}

	scroll = ScrollBar (B_HORIZONTAL);
	scroll->SetRange (0.0, x);
	scroll->SetProportion ((mWidth - x) / mWidth);
	bigStep = frame.Width() - 2;
	smallStep = bigStep / 10.;
	scroll->SetSteps (smallStep, bigStep);

	scroll = ScrollBar (B_VERTICAL);
	scroll->SetRange (0.0, y);
	scroll->SetProportion ((mHeight - y) / mHeight);
	bigStep = frame.Height() - 2;
	smallStep = bigStep / 10.;
	scroll->SetSteps (smallStep, bigStep);
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::Redraw(PDFDoc *mDoc)
{
	PDFWindow* parentWin = GetPDFWindow();

	mMouseWheelDY = 0;

	// abort rendering process if neccesary and wait for it to finish
	WaitForPage(true);
	if (parentWin) {
		parentWin->NewPage(mCurrentPage);
	}
	mRendering = true;

	if (mDoc == NULL) mDoc = this->mDoc;

	mSelected = NOT_SELECTED;
	mPageRenderer.Start(mPage, mCurrentPage, GetZoomDPI(), mRotation, &mRendererID, mEditAnnot);
	mAnnotation = NULL;
	mLinkAction = NULL;

	mBitmap = mPage->GetBitmap();
	mWidth = mPage->GetWidth(); mHeight = mPage->GetHeight();
	CenterPage();
	FixScrollbars();

	if (parentWin) {
		parentWin->GetFileAttributes()->SetPage(mCurrentPage);
		parentWin->SetPage (mCurrentPage);
		parentWin->SetZoomSize (mWidth, mHeight);
	}

	Invalidate();
}

///////////////////////////////////////////////////////////
void
PDFView::RestartDoc() {
	WaitForPage(true);
	mPageRenderer.StartDoc(mColorSpace);
	Redraw();
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::PostRedraw(thread_id id, BBitmap *bitmap) {
	// TODO
	if (id != -1) {
		mRendering = false;
		mRendererID = -1;
		Invalidate();
		BPoint mouse; uint32 buttons;
		GetMouse(&mouse, &buttons);
		DisplayLink(mouse);
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::RedrawAborted(thread_id id, BBitmap *bitmap) {
	if ((mRendererID == id) && (id != -1)) {
		mRendering = false;
		mRendererID = -1;
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::WaitForPage(bool abort) {
	if (abort) {
		mPageRenderer.Abort();
	}
	mPageRenderer.Wait();
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::CenterPage() {
	BRect bounds(Bounds());
	if (bounds.Width() + 1 > mWidth) { // center page horizontally
		mLeft = (bounds.Width() - mWidth) / 2;
	} else {
		mLeft = 0;
	}

	if (bounds.Height() + 1 > mHeight) { // center page vertically
		mTop = (bounds.Height() - mHeight) / 2;
	} else {
		mTop = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::Resize() {
	CenterPage();
	FixScrollbars();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::Dump() {
}

///////////////////////////////////////////////////////////////////////////
void
PDFView::SetPage (int page)
{
	mSelected = NOT_SELECTED;

	int currentPage = mCurrentPage;
	if (mCurrentPage != page) {
		if (page < 1) {
			mCurrentPage = 1;
		}
		else if (page > GetNumPages()) {
			mCurrentPage = GetNumPages();
		}
		else {
			mCurrentPage = page;
		}

		if (currentPage != mCurrentPage) {
			Redraw ();
		}
	}
}


//////////////////////////////////////////////////////////////////
void
PDFView::MoveToPage(int page, bool top) {
	if (page > GetNumPages()) page = GetNumPages();
	if (page <= 0) page = 1;
	bool notChanged = mCurrentPage == page;
	if (notChanged) return;

	SyncAnnotation(true);
	RecordHistory();

	BRect bounds(Bounds());
	ScrollTo(bounds.left, top ? 0 : mHeight);
	SetPage(page);
}

//////////////////////////////////////////////////////////////////
void
PDFView::MoveToPage(int num, int gen, bool top) {
	MoveToPage(mDoc->findPage(num, gen), top);
}

//////////////////////////////////////////////////////////////////
void
PDFView::MoveToPage(const char *string, bool top) {
	WaitForPage(true);
	GString* s = new GString(string);
	LinkDest* link = mDoc->getCatalog()->findDest(s);
	delete s;
	if (link) {
		if (link->isPageRef()) {
			Ref r = link->getPageRef();
			MoveToPage(r.num, r.gen, top);
		} else {
			MoveToPage(link->getPageNum(), top);
		}
		delete link;
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::BeginHistoryNavigation() {
	// Note: We are going into history navigation mode and
	// have to store the current position state in the history.
	if (mNavigationState == kNotInHistory) {
		mNavigationState = kInHistory;
		BRect bounds(Bounds());
		mHistory.AddPosition(mCurrentPage, mZoom, bounds.left, bounds.top, mRotation);
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::EndHistoryNavigation() {
	// Note: When not navigating through the history (= kNotInHistory) the
	// current position information is not stored in the history.
	// The state is stored prior to changes of the state to the history.
	// When in navigating through the history (kInHistory), the restored
	// state is the top of the history. This state has to be replaced
	// when we record the current state (= Back()).
	if (mNavigationState == kInHistory) {
		mNavigationState = kNotInHistory;
		mHistory.Back();
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::RecordHistory() {
	EndHistoryNavigation();
	BRect bounds(Bounds());
	mHistory.AddPosition(mCurrentPage, mZoom, bounds.left, bounds.top, mRotation);
}

//////////////////////////////////////////////////////////////////
void
PDFView::RecordHistory(entry_ref ref, const char* ownerPassword, const char* userPassword) {
	// XXX: record file open events too, otherwise they are missed if page state does not change between
	// open events.
	mHistory.SetFile(ref, ownerPassword, userPassword);
}

//////////////////////////////////////////////////////////////////
void
PDFView::RestoreHistory() {
	HistoryEntry* e = mHistory.GetTop();
	if (e == NULL) return;

	HistoryPosition* pos = dynamic_cast<HistoryPosition*>(e);
	if (pos) {
		PDFWindow* w = GetPDFWindow();
		HistoryFile* file = pos->GetFile();
		entry_ref ref = file->GetRef();

		if (w && !w->IsCurrentFile(&ref)) {
			bool encrypted;
			w->LoadFile(&ref, file->GetOwnerPassword(), file->GetUserPassword(), &encrypted);
			return;
		}

		int page; int32 left, top;
		page = pos->GetPage();
		mZoom = pos->GetZoom();
		left = pos->GetLeft();
		top = pos->GetTop();
		mRotation = pos->GetRotation();

		if (w) {
			w->SetZoom(mZoom); w->SetRotation(mRotation);
		}
		mCurrentPage = -1;
		SetPage(page);
		ScrollTo(left, top);
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::Back() {
	BeginHistoryNavigation();
	if (mHistory.Back()) {
		RestoreHistory();
	}
}
//////////////////////////////////////////////////////////////////
void
PDFView::Forward() {
	BeginHistoryNavigation();
	if (mHistory.Forward()) {
		RestoreHistory();
	}
}
//////////////////////////////////////////////////////////////////
void
PDFView::SetZoom (int zoom)
{
	if (mZoom != zoom) {
		RecordHistory();
		mZoom = zoom;
		Redraw ();
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::Zoom(bool zoomIn) {
	int32 zoomOld = GetZoomDPI();
	int32 zoomNew;
	if (zoomIn) {
		zoomNew = (int32)(zoomOld * 1.2);
		if (zoomNew > ZOOM_DPI_MAX) zoomNew = ZOOM_DPI_MAX;
	} else {
		zoomNew = (int32)(zoomOld / 1.2);
		if (zoomNew < ZOOM_DPI_MIN) zoomNew = ZOOM_DPI_MIN;
	}
	if (zoomNew != zoomOld) {
		SetZoom(-zoomNew);
		PDFWindow* w = GetPDFWindow();
		if (w) w->SetZoom(-zoomNew);
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::FitToPageWidth() {
	BRect r(Bounds());
	int32 zoomOld = GetZoomDPI();
	int32 zoomNew = (int32)(r.Width() * zoomOld / mWidth);
	if (zoomOld != zoomNew) {
		SetZoom(-zoomNew);
		PDFWindow* w = GetPDFWindow();
		if (w) w->SetZoom(-zoomNew);
	}
}

//////////////////////////////////////////////////////////////////
void
PDFView::FitToPage() {
	BRect r(Bounds());
	int32 zoomOld = GetZoomDPI();
	int32 zoomNewH = (int32)(r.Width() * zoomOld / mWidth);
	int32 zoomNewV = (int32)(r.Height() * zoomOld / mHeight);
	int32 zoomNew = (zoomNewH < zoomNewV) ? zoomNewH : zoomNewV;
	if (zoomOld != zoomNew) {
		SetZoom(-zoomNew);
		PDFWindow* w = GetPDFWindow();
		if (w) w->SetZoom(-zoomNew);
	}
}

//////////////////////////////////////////////////////////////////
int16
PDFView::GetZoomDPI() const {
	if (mZoom >= MIN_ZOOM)
		return kZoomDPI[mZoom -  MIN_ZOOM];
	else
		return -mZoom;
}

//////////////////////////////////////////////////////////////////
void
PDFView::SetRotation (float rotation)
{
	if (mRotation != rotation) {
		RecordHistory();
		gApp->GetSettings()->SetRotation(rotation);
		mRotation = rotation;
		PDFWindow* w = GetPDFWindow();
		if (w) w->SetRotation(mRotation);
		Redraw ();
	}
}

void
PDFView::RotateClockwise() {
	SetRotation(((int)mRotation + 90) % 360);
}

void
PDFView::RotateAntiClockwise() {
	SetRotation(((int)mRotation - 90 + 360) % 360);
}

//////////////////////////////////////////////////////////////////
void PDFView::SetSelection(int xMin, int yMin, int xMax, int yMax, bool display) {
	BRect rect(mSelection);
	mSelection.Set(xMin, yMin, xMax, yMax);
	if (Window()->Lock()) {
		if (mSelected == NOT_SELECTED) {
			Invalidate(mSelection.OffsetByCopy(mLeft, mTop));
		} else {
			rect = mSelection | rect; rect.OffsetBy(mLeft, mTop);
			Invalidate(rect);
		}
		mSelected = SELECTED;

		if (display) { // make selection visible
			BRect bounds(Bounds());
			xMin -= (int)mLeft; xMax -= (int)mLeft;
			yMin -= (int)mTop; yMax -= (int)mTop;
			float x, y;

			if ((bounds.left <= xMin) && (xMax <= bounds.right))
				x = bounds.left;
			else
				x = xMin;

			if ((bounds.top <= yMin) && (yMax <= bounds.bottom))
				y = bounds.top;
			else
				y = yMin;

			ScrollTo(x, y);
		}

		Window()->Unlock();
	}
}

//////////////////////////////////////////////////////////////////
void PDFView::GetSelection(int &xMin, int &yMin, int &xMax, int &yMax) {
	if (mSelected == SELECTED) {
		xMin = (int)mSelection.left;
		yMin = (int)mSelection.top;
		xMax = (int)mSelection.right;
		yMax = (int)mSelection.bottom;
	} else {
		xMin = yMin = xMax = yMax = 0;
	}
}
//////////////////////////////////////////////////////////////////
BString *PDFView::GetSelectedText() {
	if (!mRendering && (mSelected == SELECTED) && (mSelection.left < mSelection.right) && (mSelection.top < mSelection.bottom)) {
		GString *s = mPage->GetText(mSelection.left, mSelection.top, mSelection.right, mSelection.bottom);
		if (s) {
			BString *str = new BString(s->getCString());
			delete s;
			return str;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////
void PDFView::CopyText(BString *str) {
	if (be_clipboard->Lock()) {
		be_clipboard->Clear();

		BMessage *clip = NULL;
		if ((clip = be_clipboard->Data()) != NULL) {
			// copy text to clipboard
			clip->AddData("text/plain", B_MIME_TYPE, str->String(), str->Length());
			be_clipboard->Commit();
		}
		be_clipboard->Unlock();
	}
}
//////////////////////////////////////////////////////////////////
void PDFView::CopySelection() {
	if (!mRendering && (mSelected == SELECTED) && (mSelection.left < mSelection.right) && (mSelection.top < mSelection.bottom)) {
		if (be_clipboard->Lock()) {
			be_clipboard->Clear();

			BMessage *clip = NULL;
			if ((clip = be_clipboard->Data()) != NULL) {
				// copy bitmap to clipboard
				BMessage data;
				BRect sel(max_c(mSelection.left, 0), max_c(mSelection.top, 0),
						  min_c(mSelection.right, mWidth), min_c(mSelection.bottom, mHeight));
				BRect rect(0, 0, sel.Width(), sel.Height());
				BView view(rect, NULL, B_FOLLOW_NONE, B_WILL_DRAW);
				BBitmap bitmap(rect, mBitmap->ColorSpace(), true);
				if (bitmap.Lock()) {
					bitmap.AddChild(&view);
					view.DrawBitmap(mBitmap, sel, rect);
					view.Sync();
					bitmap.RemoveChild(&view);
					bitmap.Unlock();
				}
				bitmap.Archive(&data);
				clip->AddMessage("image/x-vnd.Be-bitmap", &data);
				clip->AddRect("rect", rect);

				// copy text to clipboard
				BString *str = GetSelectedText();
				if (str) {
					clip->AddData("text/plain", B_MIME_TYPE, str->String(), str->Length());
					delete str;
				}
				be_clipboard->Commit();
			}
			be_clipboard->Unlock();
		}
	}
}


///////////////////////////////////////////////////////////
void PDFView::SelectAll() {
	if ((mSelected == NOT_SELECTED) || (mSelected == SELECTED)) {
		mSelected = SELECTED;
		mSelection.Set(0, 0, mWidth, mHeight);
		SelectionChanged();
		Invalidate();
	}
}

///////////////////////////////////////////////////////////
void PDFView::SelectNone() {
	if (mSelected == SELECTED) {
		mSelected = NOT_SELECTED;
		SelectionChanged();
		Invalidate();
	}
}

///////////////////////////////////////////////////////////
void PDFView::SetFilledSelection(bool filled) {
	mFilledSelection = filled;
	gApp->GetSettings()->SetFilledSelection(filled);

	if (mSelected == SELECTED) {
		Invalidate();
	}
}

///////////////////////////////////////////////////////////
void PDFView::SelectionChanged() {
	PDFWindow* w = GetPDFWindow();
	if (w) {
		w->UpdateInputEnabler();
	}
}

///////////////////////////////////////////////////////////
void PDFView::SendDragMessage(uint32 protocol) {
	mDragStarted = true;
	SetMouseEventMask(B_POINTER_EVENTS);
	if (protocol == B_SIMPLE_DATA) {
		BMessage drag(B_SIMPLE_DATA);
		drag.AddString("be:types", "text/plain");
		drag.AddString("be:types", B_FILE_MIME_TYPE);

		BTranslatorRoster *roster = BTranslatorRoster::Default();
		BBitmapStream stream(mBitmap);

		translator_info *outInfo;
		int32 outNumInfo;

		drag.AddString("be:filetypes", "text/plain");
		drag.AddString("be:type_descriptions", "Text");

		if ((B_OK == roster->GetTranslators(&stream, NULL, &outInfo, &outNumInfo)) &&
			(outNumInfo >= 1)) {
			for (int32 i = 0; i < outNumInfo; i++) {
				const translation_format *fmts;
				int32 num_fmts;
				roster->GetOutputFormats(outInfo[i].translator, &fmts, &num_fmts);
				for (int32 j = 0; j < num_fmts; j++) {
					if (strcmp(fmts[j].MIME, "image/x-be-bitmap") != 0) {
						drag.AddString("be:filetypes", fmts[j].MIME);
						drag.AddString("be:type_descriptions", fmts[j].name);
					}
				}
			}
			drag.AddInt32("be:actions", B_COPY_TARGET);
			drag.AddString("be:clip_name", "Untitled clipping");
			DragMessage(&drag, mSelection.OffsetByCopy(mLeft, mTop));
		}
		BBitmap *bm;
		stream.DetachBitmap(&bm);
		if (bm != mBitmap) delete bm;
	} else if (protocol == B_MIME_DATA) {
		BMessage drag(B_MIME_DATA);
		BString *str = GetSelectedText();
		if (str) {
			drag.AddInt32("be:actions", B_TRASH_TARGET);
			drag.AddData("text/plain", B_MIME_DATA, str->String(), str->Length());
			delete str;
			DragMessage(&drag, mSelection.OffsetByCopy(mLeft, mTop));
		}
	}
}

void PDFView::SendDataMessage(BMessage *reply) {
	BMessage data(B_MIME_DATA);

	entry_ref dir;
	BString name, filetype;
	if (B_OK != reply->FindString("be:filetypes", &filetype)) {
		// send abort message to target application
		// reply->SendReply(&data);
		return;
	}
	bool saveToFile = (B_OK == reply->FindRef("directory", &dir)) &&
					  (B_OK == reply->FindString("name", &name));

	if (filetype == "text/plain") {
		BString *str = GetSelectedText();
		if (str) {
			if (saveToFile) {
				// write text to file
				BDirectory d(&dir);
				BNode node(&d, name.String());
				// set mime type
				BNodeInfo info(&node);
				if (info.InitCheck() == B_OK) {
					info.SetType("text/plain");
				}
				// write data
				BFile file(&d, name.String(), B_WRITE_ONLY);
				file.Write(str->String(), str->Length());
			} else {
				// send text in message to target application
				data.AddString("text/plain", str->String());
				reply->SendReply(&data);
			}
			delete str;
		}
		return;
	}
#if MORE_DEBUG
	BString s;
	s << name << " " << filetype;
	BAlert *a = new BAlert("Info", s.String(), "OK");
	a->Go();
#endif
	//~ sending image in message to target application not implemented
	if (!saveToFile) return;

	// copy selection to bitmap
	BRect sel(max_c(mSelection.left, 0), max_c(mSelection.top, 0),
			  min_c(mSelection.right, mWidth), min_c(mSelection.bottom, mHeight));
	BRect rect(0, 0, sel.Width(), sel.Height());
	BView view(rect, NULL, B_FOLLOW_NONE, B_WILL_DRAW);
	BBitmap *bitmap = new BBitmap(rect, mBitmap->ColorSpace(), true);
	if (bitmap->Lock()) {
		bitmap->AddChild(&view);
		view.DrawBitmap(mBitmap, sel, rect);
		view.Sync();
		bitmap->RemoveChild(&view);
		bitmap->Unlock();
	}

	BBitmapStream stream(bitmap); // destructor frees bitmap

	// identify type
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	translator_info *outInfo;
	int32 outNumInfo;
	if ((B_OK == roster->GetTranslators(&stream, NULL, &outInfo, &outNumInfo)) &&
		(outNumInfo >= 1)) {

		for (int32 i = 0; i < outNumInfo; i++) {
			const translation_format *fmts;
			int32 num_fmts;
			roster->GetOutputFormats(outInfo[i].translator, &fmts, &num_fmts);
			for (int32 j = 0; j < num_fmts; j++) {
				if (strcmp(fmts[j].MIME, filetype.String()) == 0) {
					// save bitmap to file
					BTranslatorRoster *roster = BTranslatorRoster::Default();
					BDirectory d(&dir);
					BNode node(&d, name.String());
					// set mime type
					BNodeInfo info(&node);
					if (info.InitCheck() == B_OK) {
						info.SetType(fmts[j].MIME);
					}
					// write data
					BFile file(&d, name.String(), B_WRITE_ONLY);
					roster->Translate(&stream, NULL, NULL, &file, fmts[j].type);
					// send finished message to target application
					// reply->SendReply(&data);
					return;
				}
			}
		}
	}
	// send finished message to target application
	// reply->SendReply(&data);
}

///////////////////////////////////////////////////////////
void
PDFView::SetColorSpace(color_space colorSpace) {
	bool refresh = mColorSpace != colorSpace;
	if (refresh) {
		mColorSpace = colorSpace;
		RestartDoc();
	}
}

///////////////////////////////////////////////////////////
void
PDFView::UpdateSettings(GlobalSettings* settings) {
	mInvertVerticalScrolling = settings->GetInvertVerticalScrolling();
}


///////////////////////////////////////////////////////////
void
PDFView::BeginEditAnnot() {
	SelectNone();
	SetAction(NO_ACTION);
	mEditAnnot = true;
	mAnnotation = NULL;
	mAnnotInEditor = NULL;
	mInsertAnnot = NULL;
	mLinkAction = NULL;
	Redraw();

}

///////////////////////////////////////////////////////////
void
PDFView::EndEditAnnot() {
	SyncAnnotation(true);
	SetAction(NO_ACTION);
	mEditAnnot = false;
	mInsertAnnot = NULL;
	mAnnotInEditor = NULL;
	mAnnotation = NULL;
	mLinkAction = NULL;
	Redraw();
}

///////////////////////////////////////////////////////////
void
PDFView::InsertAnnotation(Annotation* a) {
	mInsertAnnot = a;
	mAnnotation = NULL;
	SetViewCursor(gApp->pointerCursor);
	const bool updateOnly = dynamic_cast<FreeTextAnnot*>(a) == NULL;
	ShowAnnotWindow(true, updateOnly);
}

///////////////////////////////////////////////////////////
void
PDFView::UpdateAnnotation(Annotation* a, const char* contents, const char* font, float size, const char* align) {
	if (mAnnotInEditor == a) {
		ASSERT(a != NULL);
		GString* c = Utf8ToUcs2(contents);
		mAnnotInEditor->SetContents(c);
		mAnnotInEditor->SetChanged();
		FreeTextAnnot* ft = dynamic_cast<FreeTextAnnot*>(mAnnotInEditor);
		if (ft && font) {
			ft->SetFont(BePDFAcroForm::GetStandardFonts()->FindByName(font));
			ft->SetFontSize(size);
			ft->SetJustification(ToFreeTextJustification(align));
		}

		BRect invRect = CvtUserToDev(mAnnotInEditor->GetRect());
		Invalidate(invRect);
		delete c;
	}
}

///////////////////////////////////////////////////////////
void
PDFView::UpdateAnnotation(Annotation* a, BMessage* data) {
	const char* contents;
	const char* font;
	const char* align;
	float size;
	if (data->FindString("contents", &contents) != B_OK) {
		return; // failure
	}
	if (data->FindString("font", &font) != B_OK) {
		font = NULL;
	}
	if (data->FindFloat("size", &size) != B_OK) {
		size = 0.0;
	}
	if (data->FindString("alignment", &align) != B_OK) {
		align = "left";
	}
	UpdateAnnotation(a, contents, font, size, align);
}


void
PDFView::ShowAnnotWindow(bool editable, bool updateOnly)
{
	SyncAnnotation(false);
	if (editable) {
		mAnnotInEditor = mAnnotation;
		if (mAnnotInEditor == NULL) mAnnotInEditor = mInsertAnnot;
	}

	Annotation* annotation = mAnnotation;
	if (annotation == NULL) {
		annotation = mInsertAnnot;
		if (annotation == NULL) return;
	}
	BString *label, *date, *contents;
	char buffer[80];
	const char* d = to_date(annotation->GetDate(), buffer);
	if (annotation->GetTitle()) {
		label = TextToUtf8(annotation->GetTitle()->getCString(), annotation->GetTitle()->getLength());
	} else {
		label = new BString();
	}
	date     = TextToUtf8(d, strlen(d));
	contents = TextToUtf8(annotation->GetContents()->getCString(), annotation->GetContents()->getLength());
	AnnotationWindow* w = NULL;
	PDFWindow* win = GetPDFWindow();
	if (win) {
		// window already open?
		w = win->GetAnnotationWindow();
		if (w == NULL && !updateOnly) {
			// open it if not open and requested
			w = win->ShowAnnotationWindow();
		}
		if (w) {
			const char* font = NULL;
			const char* align = NULL;
			float size = 0;
			FreeTextAnnot* ft = dynamic_cast<FreeTextAnnot*>(annotation);
			if (ft) {
				font = ft->GetFont()->GetName();
				size = ft->GetFontSize();
				align = ToString(ft->GetJustification());
			}
			w->MakeEditable(editable);
			w->Update(annotation, label->String(), date->String(), contents->String(), font, size, align);
			w->Unlock();
		}
	}
	delete contents; delete date; delete label;
}

///////////////////////////////////////////////////////////
void
PDFView::ClearAnnotationWindow() {
	PDFWindow* win = GetPDFWindow();
	if (win == NULL) return;
	AnnotationWindow* w = win->GetAnnotationWindow();
	if (w) {
		w->MakeEditable(false);
		w->Update(NULL, "", "", "", NULL, 0, NULL);
		w->Unlock();
	}
}

///////////////////////////////////////////////////////////
// read info from AnnotationWindow, e.g. before a new annotation is selected
void
PDFView::SyncAnnotation(bool clearWindow) {
	if (mAnnotInEditor == NULL) return;
	PDFWindow* win = GetPDFWindow();
	if (win == NULL) return;
	AnnotationWindow* w = win->GetAnnotationWindow();
	if (w) {
		BMessage msg;
		w->GetContents(mAnnotInEditor, &msg);
		UpdateAnnotation(mAnnotInEditor, &msg);
		if (clearWindow) {
			ClearAnnotationWindow();
		}
		w->Unlock();
	}
}

class SaveFileAttachmentThread : public SaveThread {
public:

	SaveFileAttachmentThread(const char* title, XRef* xref, const BMessage* message)
		: SaveThread(title, xref)
		, mMessage(*message)
	{
	}

	int32 Run() {
		entry_ref dir;
		BString name;

		if (mMessage.FindRef("directory", &dir) != B_OK ||
			mMessage.FindString("name", &name) != B_OK) {
			// should not happen
			return -1;
		}

		BPath  path(&dir);
		path.Append(name.String());

		void* pointer;
		if (mMessage.FindPointer("fileAttachment", &pointer) != B_OK) {
			// should not happen
			return -1;
		}

		// TODO validate pointer
		FileAttachmentAnnot* fileAttachment = (FileAttachmentAnnot*)pointer;

		if (fileAttachment != NULL) {
			fileAttachment->Save(GetXRef(), path.Path());
		}

		return 0;
	}

private:
	BMessage mMessage;
};

void PDFView::SaveFileAttachment(BMessage* msg) {
	const char* title = B_TRANSLATE("Saving file attachment:");
	SaveFileAttachmentThread* thread = new SaveFileAttachmentThread(
		title,
		mDoc->getXRef(),
		msg);
	thread->Resume();
}
