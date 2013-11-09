//========================================================================
//
// AnnotAppearance.h
//
// Copyright 2002 Michael Pfeiffer
//
//========================================================================

#ifndef ANNOTATION_APPEARANCE_H
#define ANNOTATION_APPEARANCE_H

// xpdf
#include <Object.h>
#include <XRef.h>
#include <Catalog.h>
#include <PDFDoc.h>

#include "Annotation.h"

class AnnotWriter;

class GraphicsStream {
	GString     mStream;

public:	
	int         GetLength()   { return mStream.getLength(); }
	const char* GetStream()   { return mStream.getCString(); }
	GString*    GetString()   { return &mStream; }

	void Append(const char* s);
	void Append(GString* s);
	void Append(double f);
	void Add(const char* s);
	void Add(double f);
	void AddCr(const char* s);
	void AddCr(double f);
	void Add(PDFPoint p);

	void Transform(float sx, float sy, float dx, float dy);

	enum {
		stroke = true,
		fill   = false
	};	

	void SetColor(bool stroke, GfxColorComp r, GfxColorComp g, GfxColorComp b);
	void SetColor(bool stroke, GfxRGB* c);

	void SetLineWidth(float w);
	enum line_cap_style {
		butt_cap = 0,
		round_cap = 1,
		square_cap = 2
	};
	void SetLineCap(line_cap_style style);
	enum line_join_style {
		line_join = 0,
		round_join = 1,
		bevel_join = 2
	};
	void SetLineJoin(line_join_style style);
	
	void MoveTo(PDFPoint p);
	void LineTo(PDFPoint p);
	void BezierTo(PDFPoint p1, PDFPoint p2, PDFPoint p3);
	void Close();

	void Stroke();
	void Fill();
};

// ASCII 85 Encoder
class A85Encoder {
	GString*      mStream;
	unsigned char mInput[4];
	int           mLength;

	void Encode(unsigned char* output, bool* isNull);
	
public:
	A85Encoder(GString* stream);
	void Append(unsigned char byte);
	void Flush();
};

// Appearance Stream Creator
class AnnotAppearance : public AnnotVisitor {
public:
	AnnotAppearance();
	~AnnotAppearance();

	int      GetLength()   { return mAS.GetLength(); }
	GString* GetStream()   { return mAS.GetString(); }

	// visitor functionality
	void DoText(TextAnnot* a);
	void DoLink(LinkAnnot* a);
	void DoFreeText(FreeTextAnnot* a);
	void DoLine(LineAnnot* a);
	void DoSquare(SquareAnnot* a);
	void DoCircle(CircleAnnot* a);
	void DoHighlight(HighlightAnnot* a);
	void DoUnderline(UnderlineAnnot* a);
	void DoSquiggly(SquigglyAnnot* a);
	void DoStrikeOut(StrikeOutAnnot* a);
	void DoStamp(StampAnnot* a);
	void DoInk(InkAnnot* a);
	void DoPopup(PopupAnnot* a);
	void DoFileAttachment(FileAttachmentAnnot* a);
	void DoSound(SoundAnnot* a);
	void DoMovie(MovieAnnot* a);
	void DoWidget(WidgetAnnot* a);
	void DoPrinterMark(PrinterMarkAnnot* a);
	void DoTrapNet(TrapNetAnnot* a);

private:
	void Init(Annotation* annot);
	PDFPoint UserToForm(PDFPoint p);
	void DoStyledAnnot(StyledAnnot* a);
	PDFPoint PointBetween(PDFPoint p1, PDFPoint p2, float f);

	void Stroke(PDFQuadPoints &qp, float f);
	void DoMarkupAnnot(MarkupAnnot* a, float f);
	void Stroke(PDFQuadPoints &qp);
	void StrokeSquiggly(PDFPoint p1, PDFPoint p2, float height);

	GraphicsStream mAS;
	PDFRectangle   mBBox;
	PDFPoint       mDelta;
	A85Encoder     mEncoder;
};

#endif
