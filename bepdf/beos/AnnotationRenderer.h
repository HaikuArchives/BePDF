/*  
	BeOS Front-end du PDF file reader xpdf.
	Copyright (C) 2001 Michael Pfeiffer

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

#ifndef _ANNOTATION_RENDERER_H
#define _ANNOTATION_RENDERER_H

#include <XRef.h>
#include <PDFDoc.h>

#include <View.h>
#include <Region.h>

#include "Annotation.h"

// Render Annotation 
class AnnotationRenderer : public AnnotVisitor {
	BView*		mView;
	int         mZoom;
	double      mCtm[6];
	bool        mEdit;

	drawing_mode   mDrawingMode;
	source_alpha   mSourceAlpha;
	alpha_function mAlphaFunction;

	class ClipToRect {
		AnnotationRenderer* mRenderer;
		Annotation*         mAnnot;
		BRect               mRect;
		BRegion             mOldClippingRegion;
		
		BView* View() { return mRenderer->mView; }
	public:
		ClipToRect(AnnotationRenderer* r, Annotation* a);
		~ClipToRect();
	};

	friend class ClipToRect;

	float CvtUserToDev(float f);
	void CvtUserToDev(double ux, double uy, int *dx, int *dy);
	BRect ToRect(PDFRectangle* r);
	void CvtUserToDev(PDFPoint* u, BPoint* d, int n);
	BPoint PointBetween(BPoint p1, BPoint p2, float f);
	void StrokeSquiggly(BPoint p1, BPoint p2, float height);
	void DrawBitmap(BBitmap* image, BPoint p);	
	void SetPenSize(int width);
	void DrawRect(Annotation* a, BRect rect);
	bool DrawAnnotation(Annotation* a);
	float LayoutText(BString* text, BFont font, free_text_justification justification, BRect bounds, bool draw);
	
public:
	AnnotationRenderer(BView* v, double* ctm, int zoom, bool edit);
	~AnnotationRenderer();

	static rgb_color GetColor(GfxRGB* c, double opacity = 1);
	static BBitmap* ColorBitmap(BBitmap* image, rgb_color color);
	
	virtual void DoText(TextAnnot* a);
	virtual void DoLink(LinkAnnot* a);
	virtual void DoFreeText(FreeTextAnnot* a);
	virtual void DoLine(LineAnnot* a);
	virtual void DoSquare(SquareAnnot* a);
	virtual void DoCircle(CircleAnnot* a);
	virtual void DoHighlight(HighlightAnnot* a);
	virtual void DoUnderline(UnderlineAnnot* a);
	virtual void DoSquiggly(SquigglyAnnot* a);
	virtual void DoStrikeOut(StrikeOutAnnot* a);
	virtual void DoStamp(StampAnnot* a);
	virtual void DoInk(InkAnnot* a);
	virtual void DoPopup(PopupAnnot* a);
	virtual void DoFileAttachment(FileAttachmentAnnot* a);
	virtual void DoSound(SoundAnnot* a);
	virtual void DoMovie(MovieAnnot* a);
	virtual void DoWidget(WidgetAnnot* a);
	virtual void DoPrinterMark(PrinterMarkAnnot* a);
	virtual void DoTrapNet(TrapNetAnnot* a);
};

#endif
