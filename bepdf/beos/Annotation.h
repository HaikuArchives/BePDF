//========================================================================
//
// Annotation.h
//
// Copyright 2002, 2003 Michael Pfeiffer
//
//========================================================================

#ifndef ANNOTATION_H
#define ANNOTATION_H

// xpdf
#include <Object.h>
#include <Link.h>
#include <Page.h>  // for PDFRectangle
#include <GfxState.h> // for GfxRGB
#include <list>

// bepdf
#include "FileSpec.h"

class TextAnnot;
class LinkAnnot;
class StyledAnnot;
class   FreeTextAnnot;
class   LineAnnot;
class   SquareAnnot;
class   CircleAnnot;
class   MarkupAnnot;
class     HighlightAnnot;
class     UnderlineAnnot;
class     SquigglyAnnot;
class     StrikeOutAnnot;
class   InkAnnot;
class StampAnnot;
class PopupAnnot;
class FileAttachmentAnnot;
class SoundAnnot;
class MovieAnnot;
class WidgetAnnot;
class PrinterMarkAnnot;
class TrapNetAnnot;

class AcroForm;

extern Ref empty_ref;

bool is_empty_ref(Ref ref);

const char* to_date(const char* date, char* buffer); // min. buffer length == 80

class PDFFont {
private:
	Ref     mRef;
	GString mName;
	GString mShortName;
	
public:
	PDFFont();
	
	Ref GetRef() const;
	void SetRef(Ref ref);
	const char* GetName();
	void SetName(const char* name);
	const char* GetShortName();	
	void SetShortName(const char* name);
	
	void Print();
};

class PDFStandardFonts {
private:
	enum {
		num_of_standard_fonts = 12
	};
	static const char* mNames[num_of_standard_fonts];
	PDFFont mFonts[num_of_standard_fonts];
	
public:
	PDFStandardFonts();
	void Reset(); // refs and short name

	int CountFonts() const;
	PDFFont* FontAt(int i);
	PDFFont* FindByName(const char* name);
	PDFFont* FindByShortName(const char* name);
};

class PDFPoint {
public:
	double x, y;
	PDFPoint() { }
	PDFPoint(double x, double y) : x(x), y(y) { }
	
	PDFPoint operator+(const PDFPoint& p) const;
	PDFPoint operator-(const PDFPoint& p) const;
	bool operator==(const PDFPoint& p) const;
	bool operator!=(const PDFPoint& p) const;
	PDFPoint& operator+=(const PDFPoint& p);
	PDFPoint& operator-=(const PDFPoint& p);
};

class PDFPoints {
private:
	int       mLength;
	PDFPoint *mPoints;
	
public:
	PDFPoints() : mLength(0), mPoints(NULL) { }
	PDFPoints(PDFPoints* copy);
	~PDFPoints()     { delete mPoints; }

	void SetLength(int l) { delete []mPoints; mLength = l; mPoints = new PDFPoint[l]; }
	int GetLength()  { return mLength; }
	PDFPoint* PointAt(int i) { return &mPoints[i]; }
	PDFPoint* Points()       { return mPoints; }
	
	PDFPoints& operator=(PDFPoints& p);
};

class PDFQuadPoints {
public:
	PDFPoint q[4];
	PDFPoint& operator[](int i) { return q[i]; }
};

class AnnotVisitor {
public:
	AnnotVisitor();
	virtual ~AnnotVisitor();

	virtual void DoText(TextAnnot* a) = 0;
	virtual void DoLink(LinkAnnot* a) = 0;
	virtual void DoFreeText(FreeTextAnnot* a) = 0;
	virtual void DoLine(LineAnnot* a) = 0;
	virtual void DoSquare(SquareAnnot* a) = 0;
	virtual void DoCircle(CircleAnnot* a) = 0;
	virtual void DoHighlight(HighlightAnnot* a) = 0;
	virtual void DoUnderline(UnderlineAnnot* a) = 0;
	virtual void DoSquiggly(SquigglyAnnot* a) = 0;
	virtual void DoStrikeOut(StrikeOutAnnot* a) = 0;
	virtual void DoStamp(StampAnnot* a) = 0;
	virtual void DoInk(InkAnnot* a) = 0;
	virtual void DoPopup(PopupAnnot* a) = 0;
	virtual void DoFileAttachment(FileAttachmentAnnot* a) = 0;
	virtual void DoSound(SoundAnnot* a) = 0;
	virtual void DoMovie(MovieAnnot* a) = 0;
	virtual void DoWidget(WidgetAnnot* a) = 0;
	virtual void DoPrinterMark(PrinterMarkAnnot* a) = 0;
	virtual void DoTrapNet(TrapNetAnnot* a) = 0;
};

class BorderStyle {
public:
	enum border_style_type {
		solid_style,
		dashed_style,
		beveled_style,
		inset_style,
		underline_style
	};

private:
	int               mWidth;
	border_style_type mStyle;
	// dash array not supported yet!
	
public:
	BorderStyle();
	void Set(Dict* d, bool check_type = true);
	
	int GetWidth()               { return mWidth; }
	border_style_type GetStyle() { return mStyle; }
	
	void SetWidth(int w) { mWidth = w; }
	void SetStyle(border_style_type style) { mStyle = style; }
};


enum {
	invisible_flag = 1 << 0,
	hidden_flag    = 1 << 1,
	print_flag     = 1 << 2,
	no_zoom_flag   = 1 << 3,
	no_rotate_flag = 1 << 4,
	no_view_flag   = 1 << 5,
	read_only_flag = 1 << 6
};
	
class AnnotFlags {
private:
	unsigned int mFlags;
	
public:
	AnnotFlags() : mFlags(0) { }

	void Set(int f) { mFlags = f; }
	unsigned int Flags() { return mFlags; }

	bool IsSet(unsigned int mask) const { return (mFlags & mask) == mask; }
	void ClearMask(unsigned int mask)   { mFlags = mFlags & ~mask; }
	void SetMask(unsigned int mask)     { mFlags |= mask; }

	bool Invisible() const { return IsSet(invisible_flag); }
	bool Hidden() const    { return IsSet(hidden_flag); }
	bool Print() const     { return IsSet(print_flag); }
	bool NoZoom() const    { return IsSet(no_zoom_flag); }
	bool NoRotate() const  { return IsSet(no_rotate_flag); }
	bool NoView() const    { return IsSet(no_view_flag); }
	bool ReadOnly() const  { return IsSet(read_only_flag); }
};

class Annotation {
private:
	Ref             mRef;
	GString			mContents; // in PDFDocEncoding or UCS2
	PDFRectangle	mRect;	
	GString*        mDate; // optional (can be NULL)
	AnnotFlags      mFlags;
	// border style and border -> StyledAnnot
	// appearence stream
	bool            mHasAppearanceStream;
	// appearence state
	GfxRGB			mColor;
	bool            mHasColor;
	double          mOpacity;
	GString*        mTitle; // optional
	// popup
	PopupAnnot*     mPopup; // optional
	// action
	// additional action
	// structural parent tree
	
	bool			mValid;

	// editing
	bool            mDeleted;
	bool            mChanged;
	bool            mSelected;
	
protected:
	// This method should be the first statement in the constructor
	// of a child class.
	// Returns false if constructor in parent class has failed.
	bool CheckInvalid() { if (mValid) { mValid = false; return false; } else return true; }
	// This method should be called as the last statement 
	// in the constructor if there were no errors.
	void SetValid()     { mValid = true; }
	bool ReadNum(Array* a, int i, double& d);
	bool ReadPoint(Array* a, int i, PDFPoint* p); 
	void SetString(GString* &s, const char* t);
	
public:
	Annotation(PDFRectangle rect);
	Annotation(Annotation* copy);
	Annotation(Dict* annot);
	virtual ~Annotation();
	
	virtual Annotation* Clone() = 0;

	bool IsValid() const { return mValid; }

	Ref           GetRef()           { return mRef; }
	GString*      GetContents()      { return &mContents; }
	PDFRectangle* GetRect()          { return &mRect; }
	const char*   GetDate()          { return mDate ? mDate->getCString() : ""; }
	AnnotFlags*   GetFlags()         { return &mFlags; }
	bool          HasAppearanceStream() { return mHasAppearanceStream; }
	GfxRGB*       GetColor()         { return &mColor; }
	bool          HasColor()         { return mHasColor; }
	double        GetOpacity()       { return mOpacity; }
	GString*      GetTitle()         { return mTitle; }
	PopupAnnot*   GetPopup()         { return mPopup; }

	virtual void Visit(AnnotVisitor* v) = 0;
	virtual void Print();
	
	PDFPoint     LeftTop()          { return PDFPoint(mRect.x1, mRect.y2); }
	virtual void MoveTo(PDFPoint p);
	virtual void ResizeTo(double w, double h);

	// Editing
	void SetRef(const Ref ref);
	void SetContents(GString* contents);
	void SetDate(const char* date);
	void SetHasColor(bool color)   { mHasColor = color; }
	void SetTitle(GString* title);
	void SetPopup(PopupAnnot* a)   { mPopup = a; }
	void SetDeleted(bool d)        { mDeleted = d; if (d) SetChanged(); }
	bool IsDeleted() const         { return mDeleted; }
	void SetChanged(bool c = true) { mChanged = c; }
	bool HasChanged() const        { return mChanged; }
	void SetSelected(bool s)       { mSelected = s; }	
	bool IsSelected() const        { return mSelected; }
};


class TextAnnot : public Annotation {
public:
	enum text_annot_type {
		comment_type,
		help_type,
		insert_type,
		key_type,
		new_paragraph_type,
		note_type,
		paragraph_type,
		unknown_type,
		no_of_types
	};

private:
	bool		         mOpen;
	GString		         mName;  // default Note
	enum text_annot_type mType;
	
	static const char *mTypeNames[no_of_types-1];
	
public:
	TextAnnot(PDFRectangle rect, text_annot_type type);
	TextAnnot(TextAnnot* copy);
	TextAnnot(Dict* annot);
	
	Annotation* Clone() { return new TextAnnot(this); }

	bool IsOpen()                  { return mOpen; }
	const char* GetName()          { return mName.getCString(); }
	enum text_annot_type GetType() { return mType; }

	virtual void Visit(AnnotVisitor* v) { v->DoText(this); }
	virtual void Print();
};

class StyledAnnot : public Annotation {
private:
	BorderStyle mStyle;
	
public:
	StyledAnnot(PDFRectangle rect, float r, float g, float b);
	StyledAnnot(StyledAnnot* copy);
	StyledAnnot(Dict* d);

	BorderStyle* GetBorderStyle() { return &mStyle; }
	virtual void Print();
};

enum free_text_justification {
	left_justify  = 0,
	centered      = 1,
	right_justify = 2
};

free_text_justification ToFreeTextJustification(const char* name);
const char* ToString(free_text_justification j);

class FreeTextAnnot : public StyledAnnot {
private:
	GString		            mAppearance;  
	free_text_justification mJustification;
	PDFFont*                mFont; 
	GfxRGB                  mFontColor;
	float                   mFontSize;            

	void Init();
			
public:
	FreeTextAnnot(PDFRectangle rect, PDFFont* font);
	FreeTextAnnot(FreeTextAnnot* copy);
	FreeTextAnnot(Dict* annot, AcroForm* acroForm);
	
	Annotation* Clone() { return new FreeTextAnnot(this); }

	GString* GetAppearance() { return &mAppearance; }
	void SetAppearance(GString* ap);
	free_text_justification GetJustification() { return mJustification; }
	void SetJustification(free_text_justification j);
	PDFFont* GetFont() const { return mFont; }
	void SetFont(PDFFont* font);
	GfxRGB* GetFontColor() { return &mFontColor; }
	float GetFontSize() const { return mFontSize; }
	void SetFontSize(float s) { mFontSize = s; }

	virtual void Visit(AnnotVisitor* v) { v->DoFreeText(this); }
	virtual void Print();
};

class LineAnnot : public StyledAnnot {
private:
	PDFPoint    mLine[2];

public:
	LineAnnot(PDFRectangle rect, PDFPoint* line);
	LineAnnot(LineAnnot* copy);
	LineAnnot(Dict* annot);
	Annotation* Clone() { return new LineAnnot(this); }
	
	PDFPoint* GetLine()     { return mLine; }	

	virtual void Visit(AnnotVisitor* v) { v->DoLine(this); }
	virtual void Print();
};

class SquareAnnot : public StyledAnnot {
public:
	SquareAnnot(PDFRectangle rect) : StyledAnnot(rect, 0.0, 0.6, 1.0) { }
	SquareAnnot(SquareAnnot* copy) : StyledAnnot(copy) { }
	SquareAnnot(Dict* d) : StyledAnnot(d) { }
	Annotation* Clone() { return new SquareAnnot(this); }
	virtual void Visit(AnnotVisitor* v) { v->DoSquare(this); }
	virtual void Print();
};

class CircleAnnot : public StyledAnnot {
public:
	CircleAnnot(PDFRectangle rect) : StyledAnnot(rect, 0.0, 0.6, 1.0) { }
	CircleAnnot(CircleAnnot* copy) : StyledAnnot(copy) { }
	CircleAnnot(Dict* d) : StyledAnnot(d) { }
	Annotation* Clone() { return new CircleAnnot(this); }
	virtual void Visit(AnnotVisitor* v) { v->DoCircle(this); }
	virtual void Print();
};

class MarkupAnnot : public StyledAnnot {
private:
	int            mLength;
	PDFQuadPoints *mQuadPoints;
	
public:
	MarkupAnnot(PDFRectangle rect, float r, float g, float b, PDFQuadPoints* p, int l);
	MarkupAnnot(MarkupAnnot* copy);
	MarkupAnnot(Dict* annot);
	~MarkupAnnot();
	
	int QuadPointsLength()             { return mLength; }
	PDFQuadPoints *QuadPointsAt(int i) { return &mQuadPoints[i]; }
	
	virtual void Visit(AnnotVisitor* v) = 0;
	virtual void Print();

	virtual void MoveTo(PDFPoint p);
	virtual void ResizeTo(double w, double h);
};

class HighlightAnnot : public MarkupAnnot {
public:
	HighlightAnnot(PDFRectangle rect);
	HighlightAnnot(HighlightAnnot* copy) : MarkupAnnot(copy) { }
	HighlightAnnot(Dict* annot) : MarkupAnnot(annot) { }
	Annotation* Clone() { return new HighlightAnnot(this); }
	virtual void Visit(AnnotVisitor* v) { v->DoHighlight(this); }
};

class UnderlineAnnot : public MarkupAnnot {
public:
	UnderlineAnnot(PDFRectangle rect);
	UnderlineAnnot(UnderlineAnnot* copy) : MarkupAnnot(copy) { }
	UnderlineAnnot(Dict* annot) : MarkupAnnot(annot) { }
	Annotation* Clone() { return new UnderlineAnnot(this); }
	virtual void Visit(AnnotVisitor* v) { v->DoUnderline(this); }
};

class SquigglyAnnot : public MarkupAnnot {
public:
	SquigglyAnnot(PDFRectangle rect);
	SquigglyAnnot(SquigglyAnnot* copy) : MarkupAnnot(copy) { }
	SquigglyAnnot(Dict* annot) : MarkupAnnot(annot) { }
	Annotation* Clone() { return new SquigglyAnnot(this); }
	virtual void Visit(AnnotVisitor* v) { v->DoSquiggly(this); }
};

class StrikeOutAnnot : public MarkupAnnot {
public:
	StrikeOutAnnot(PDFRectangle rect);
	StrikeOutAnnot(StrikeOutAnnot* copy) : MarkupAnnot(copy) { }
	StrikeOutAnnot(Dict* annot) : MarkupAnnot(annot) { }
	Annotation* Clone() { return new StrikeOutAnnot(this); }
	virtual void Visit(AnnotVisitor* v) { v->DoStrikeOut(this); }
};

class StampAnnot : public Annotation {
	GString		mName;  // default Draft

public:
	StampAnnot(StampAnnot* copy);
	StampAnnot(Dict* annot);
	Annotation* Clone() { return new StampAnnot(this); }

	const char* GetName()       { return mName.getCString(); }

	virtual void Visit(AnnotVisitor* v) { v->DoStamp(this); }
	virtual void Print();
};

class InkAnnot : public StyledAnnot {
private:
	int        mLength;
	PDFPoints* mInkList;		

public:
	InkAnnot(InkAnnot* copy);
	InkAnnot(Dict* annot);
	~InkAnnot();
	Annotation* Clone() { return new InkAnnot(this); }
	
	int GetLength() { return mLength; }
	PDFPoints* PathAt(int i) { return &mInkList[i]; }
	virtual void Visit(AnnotVisitor* v) { v->DoInk(this); }
	virtual void Print();
};

class PopupAnnot : public Annotation {
	Ref mParentRef;
	
public:
	PopupAnnot(PDFRectangle rect);
	PopupAnnot(PopupAnnot* copy);
	PopupAnnot(Dict* annot);
	Annotation* Clone() { return new PopupAnnot(this); }

	void SetParentRef(Ref ref) { mParentRef = ref; }
	Ref  GetParentRef()        { return mParentRef; }

	virtual void Visit(AnnotVisitor* v) { v->DoPopup(this); }	
	virtual void Print();
};

class FileAttachmentAnnot : public Annotation {
public:
	enum attachment_type {
		graphic_type,
		paper_clip_type,
		push_pin_type,
		tag_type,
		unknown_type,
		no_of_types
	};
	
private:
	GString              mName;
	GString              mFileName;
	enum attachment_type mType;
	static const char* mTypeNames[no_of_types-1];
	FileSpec             mFileSpec;
	
public:
	FileAttachmentAnnot(FileAttachmentAnnot* copy);
	FileAttachmentAnnot(Dict* annot);
	Annotation* Clone() { return new FileAttachmentAnnot(this); }

	const char* GetName() { return mName.getCString(); }
	enum attachment_type GetType() { return mType; }
	// Return file name
	const char* GetFileName() { return mFileSpec.GetFileName()->getCString(); }
	// Save file
	bool Save(XRef *xref, const char* file);
	
	virtual void Visit(AnnotVisitor* v) { v->DoFileAttachment(this); }
	virtual void Print();
};

class AnnotSorter : public AnnotVisitor {
public:
	AnnotSorter();
	~AnnotSorter();
	
	virtual void DoText(TextAnnot* a) { mResult = 1; }
	virtual void DoLink(LinkAnnot* a) { mResult = 2; }
	virtual void DoFreeText(FreeTextAnnot* a) { mResult = 3; }
	virtual void DoLine(LineAnnot* a) { mResult = 4; }
	virtual void DoSquare(SquareAnnot* a) { mResult = 5; }
	virtual void DoCircle(CircleAnnot* a) { mResult = 6; }
	virtual void DoHighlight(HighlightAnnot* a) { mResult = 7; }
	virtual void DoUnderline(UnderlineAnnot* a) { mResult = 8; }
	virtual void DoSquiggly(SquigglyAnnot* a) { mResult = 9; }
	virtual void DoStrikeOut(StrikeOutAnnot* a) { mResult = 10; }
	virtual void DoStamp(StampAnnot* a) { mResult = 11; }
	virtual void DoInk(InkAnnot* a) { mResult = 12; }
	virtual void DoPopup(PopupAnnot* a) { mResult = 13; }
	virtual void DoFileAttachment(FileAttachmentAnnot* a) { mResult = 14; }
	virtual void DoSound(SoundAnnot* a) { mResult = 15; }
	virtual void DoMovie(MovieAnnot* a) { mResult = 16; }
	virtual void DoWidget(WidgetAnnot* a) { mResult = 17; }
	virtual void DoPrinterMark(PrinterMarkAnnot* a) { mResult = 18; }
	virtual void DoTrapNet(TrapNetAnnot* a) { mResult = 19; }
	
	int GetResult() { return mResult; }	

private:
	int mResult;
};

class AnnotName : public AnnotVisitor {
public:
	AnnotName();
	~AnnotName();

	virtual void DoText(TextAnnot* a) { mResult = a->GetName(); }
	virtual void DoLink(LinkAnnot* a) { mResult = "Link"; }
	virtual void DoFreeText(FreeTextAnnot* a) { mResult = "FreeText"; }
	virtual void DoLine(LineAnnot* a) { mResult = "Line"; }
	virtual void DoSquare(SquareAnnot* a) { mResult = "Square"; }
	virtual void DoCircle(CircleAnnot* a) { mResult = "Circle"; }
	virtual void DoHighlight(HighlightAnnot* a) { mResult = "Highlight"; }
	virtual void DoUnderline(UnderlineAnnot* a) { mResult = "Underline"; }
	virtual void DoSquiggly(SquigglyAnnot* a) { mResult = "Squiggly"; }
	virtual void DoStrikeOut(StrikeOutAnnot* a) { mResult = "StrikeOut"; }
	virtual void DoStamp(StampAnnot* a) { mResult = "Stamp"; }
	virtual void DoInk(InkAnnot* a) { mResult = "Ink"; }
	virtual void DoPopup(PopupAnnot* a) { mResult = "Popup"; }
	virtual void DoFileAttachment(FileAttachmentAnnot* a) { mResult = "FileAttachment"; }
	virtual void DoSound(SoundAnnot* a) { mResult = "Sound"; }
	virtual void DoMovie(MovieAnnot* a) { mResult = "Movie"; }
	virtual void DoWidget(WidgetAnnot* a) { mResult = "Widget"; }
	virtual void DoPrinterMark(PrinterMarkAnnot* a) { mResult = "PrinterMark"; }
	virtual void DoTrapNet(TrapNetAnnot* a) { mResult = "TrapNet"; }
	
	const char* GetResult() { return mResult; }	

private:
	const char* mResult;
};

class AcroForm {
private:
	static PDFStandardFonts* mStandardFonts;
	Ref                      mRef;
	GString                  mAppearance;
	free_text_justification  mJustification;
	std::list<PDFFont*>           mFonts;
	
	void ParseFont(const char* shortName, Ref ref, Dict* font);
	
public:
	AcroForm(XRef* xref, Object* acroForm);
	~AcroForm();
	
	static PDFStandardFonts* GetStandardFonts();
	
	Ref GetRef() { return mRef; }
	void SetRef(Ref ref) { mRef = ref; }
	
	const char* GetAppearance() { return mAppearance.getCString(); }
	void SetAppearance(const char* a) { mAppearance.clear()->append(a); }
	
	free_text_justification GetJustification() const { return mJustification; }
	void SetJustification(free_text_justification j) { mJustification = j; }
	
	std::list<PDFFont*>* GetFonts() { return &mFonts; }
	PDFFont* FindFontByShortName(const char* name);
};

typedef Annotation *AnnotationPtr;

class Annotations {
private:	
	int               mMax;
	int               mLength;
	AnnotationPtr*    mAnnots;
	
	void Resize(int max);
	
public:
	Annotations(Annotations* copy);
	Annotations(Object* annots, AcroForm* acroForm);
	~Annotations();
	
	int Length() const    { return mLength; }
	Annotation* At(int i) const { return mAnnots[i]; }
	void Append(Annotation* a);
	Annotation* Remove(int i);
	
	void Iterate(AnnotVisitor* visitor);
	Annotation* OverAnnotation(double x, double y, bool edit = false);
	void Sort(AnnotSorter* sort); // not thread safe!!!
	bool HasChanged() const;
};

typedef Annotations *AnnotationsPtr;


class AnnotsList {
	AnnotationsPtr* mAnnots;
	int             mLength;

	void MakeEmpty();

public:
	AnnotsList(AnnotsList* copy);
	AnnotsList();
	~AnnotsList();
	void SetSize(int size);
	Annotations* Get(int i) const;	
	void Set(int i, Annotations* a);
	bool HasChanged() const;
};

// we are interested in font name and size only
class AppearanceStringParser {
public:
	AppearanceStringParser(const char* as);
	
	bool IsOK() const { return mOK; }
	const char* GetFontName() { return mFontName.getCString(); }
	float GetFontSize() const { return mFontSize; }
	GfxRGB* GetColor() { return &mColor; }
	
private:
	bool    mOK;
	GString mFontName;
	float   mFontSize;
	GfxRGB  mColor;
};

class AnnotUtils {
public:
	// helper functions
	static bool InUCS2(GString* s);
	static GString* EscapeString(GString* text);
	static GString* EscapeName(const char* text);
	static void CurrentDate(GString* date);
};

#endif
