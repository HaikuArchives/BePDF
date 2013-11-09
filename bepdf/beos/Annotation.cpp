//========================================================================
//
// Annotation.cpp
//
// Copyright 2002 Michael Pfeiffer
//
//========================================================================

#include <stdlib.h>
#include <stdio.h>
#include <Rect.h>
#include <ctype.h>
#include <Debug.h>
#include <stack>

#include "Annotation.h"

#if 0
#define LOG(text) fprintf(stderr, text)
#else
#define LOG(text)
#endif
#define CR LOG("\n")


#define COPYN(N) for (n = N; n && date[i]; n--) s[j++] = date[i++];

// to_date() is used in PDFView.cpp
const char *to_date(const char *date, char* s) {
	if ((date[0] == 'D') && (date[1] == ':')) {
		int i = 2;
		// skip spaces
		while (date[i] == ' ') i++;
		int from = i;
		while (date[i] && isdigit(date[i])) i++;
		int to = i;
		int j = 0, n;
		i = from;
		if (to - from > 12) 
			COPYN(to - from - 10)
		else
			COPYN(to - from - 4);
		s[j++] = '/';
		COPYN(2)
		s[j++] = '/';
		COPYN(2)
		s[j++] = ' ';
		if (date[i]) {
			COPYN(2);
			s[j++] = ':';
			COPYN(2);
			s[j++] = ':';
			COPYN(2);
			if (date[i]) {
				s[j++] = ' ';
				s[j++] = date[i++];
				COPYN(2); i++; // skip '
				s[j++] = ':';
				COPYN(2);
			}
		}
		s[j] = 0;
		return s;
	}
	else
		return date;
}

Ref empty_ref = { 0, 65535 };

bool is_empty_ref(Ref ref) {
	return ref.num == empty_ref.num && ref.gen == empty_ref.gen;
}

// Implementation of PDFFont
PDFFont::PDFFont() 
	: mRef(empty_ref)
{
}

Ref PDFFont::GetRef() const {
	return mRef;
}

const char* PDFFont::GetName() {
	return mName.getCString();
}

const char* PDFFont::GetShortName() {
	return mShortName.getCString();
}

void PDFFont::SetRef(Ref ref) {
	mRef = ref;
}

void PDFFont::SetName(const char* name) {
	mName.clear()->append(name);
}

void PDFFont::SetShortName(const char* name) {
	mShortName.clear()->append(name);
}

void PDFFont::Print() {
	fprintf(stderr, "Name: %s\n", GetName());
	fprintf(stderr, "Short name: %s\n", GetShortName());
}

// Implementation of PDFStandardFonts
const char* PDFStandardFonts::mNames[] = {
	"Times-Roman", "Times-Bold", "Times-Italic", "Times-BoldItalic",
	"Helvetica", "Helvetica-Bold", "Helvetica-Oblique", "Helvetica-BoldOblique",
	"Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique",
//	"Symbol", 
//	"ZapfDingbats",
};

PDFStandardFonts::PDFStandardFonts() {
	ASSERT(sizeof(mNames)/sizeof(const char*) == num_of_standard_fonts);	
	int i;
	for (i = 0; i < num_of_standard_fonts; i ++) {
		mFonts[i].SetName(mNames[i]);
	}
}

void PDFStandardFonts::Reset() {
	int i;
	for (i = 0; i < num_of_standard_fonts; i ++) {
		PDFFont* font = FontAt(i);
		font->SetRef(empty_ref);
		font->SetShortName("");
	}
}

int PDFStandardFonts::CountFonts() const {
	return num_of_standard_fonts;
}

PDFFont* PDFStandardFonts::FontAt(int i) {
	ASSERT(i >= 0 && i < num_of_standard_fonts);
	return &mFonts[i];
}

PDFFont* PDFStandardFonts::FindByName(const char* name) {
	for (int i = 0; i < num_of_standard_fonts; i ++) {
		if (strcmp(mFonts[i].GetName(), name) == 0) {
			return &mFonts[i];
		}	
	}
	return NULL;
}

PDFFont* PDFStandardFonts::FindByShortName(const char* name) {
	for (int i = 0; i < num_of_standard_fonts; i ++) {
		if (strcmp(mFonts[i].GetShortName(), name) == 0) {
			return &mFonts[i];
		}	
	}
	return NULL;
}

// Implementation of PDFPoint
PDFPoint PDFPoint::operator+(const PDFPoint& p) const {
	return PDFPoint(this->x + p.x, this->y + p.y);
}

PDFPoint PDFPoint::operator-(const PDFPoint& p) const {
	return PDFPoint(this->x - p.x, this->y - p.y);
}

bool PDFPoint::operator==(const PDFPoint& p) const {
	return this->x == p.x && this->y == p.y;
}

bool PDFPoint::operator!=(const PDFPoint& p) const {
	return !(*this == p);
}

PDFPoint& PDFPoint::operator+=(const PDFPoint& p) {
	*this = *this + p;
	return *this;
}

PDFPoint& PDFPoint::operator-=(const PDFPoint& p) {
	*this = *this - p;
	return *this;
}

// Implementation of PDFPoints
PDFPoints::PDFPoints(PDFPoints* copy) 
	: mLength(copy->mLength)
	, mPoints(NULL)
{
	*this = *copy;
}

PDFPoints& PDFPoints::operator=(PDFPoints& p) {
	if (this != &p) {
		delete []mPoints;
		mLength = p.mLength;
		if (mLength > 0) {
			mPoints = new PDFPoint[mLength];
			for (int i = 0; i < mLength; i++) mPoints[i] = p.mPoints[i];
		} else {
			mPoints = NULL;
		}
	}
	return *this;
}

// Implementation of BorderStyle
BorderStyle::BorderStyle()
	: mWidth(1)
	, mStyle(solid_style) 
{
	// intentionally empty
}

void BorderStyle::Set(Dict* d, bool check_type) {
	Object obj;

	// set default values
	mWidth = 1;
	mStyle = solid_style;

	// sanity check
	if (check_type && d->lookup("Type", &obj) && !obj.isNull()) {
		if (!obj.isName() || strcmp(obj.getName(), "Border") != 0) {
			LOG("Dict is not of type Border!\n");
			obj.free();
			return;
		}
	}
	obj.free();

	// border width in points
	if (d->lookup("W", &obj) && obj.isNum()) {
		mWidth = (int)obj.getNum();
	} else {
	}
	obj.free();	

	// border style
	if (d->lookup("S", &obj) && obj.isName()) {
		const char* n = obj.getName();
		switch (n[0]) {
			case 'S': mStyle = solid_style; break;
			case 'D': mStyle = dashed_style; break;
			case 'B': mStyle = beveled_style; break;
			case 'I': mStyle = inset_style; break;
			case 'U': mStyle = underline_style; break;
		}
	}
	obj.free();
	
	// dash array not supported yet!
}

template<class C>
static C* Copy(C* s) {
	if (s) {
		return new C(s);
	} else {
		return NULL;
	}
}

// Implementation of AnnotVisitor
AnnotVisitor::AnnotVisitor() 
{
	// no op
}

AnnotVisitor::~AnnotVisitor() 
{
	// no op
}

// Implementation of Annotation
Annotation::Annotation(PDFRectangle r)
	: mRef(empty_ref)
	, mContents("")
	, mRect(r)
	, mDate(NULL)
	, mHasAppearanceStream(false)
	, mHasColor(true)
	, mOpacity(1)
	, mTitle(NULL)
	, mPopup(NULL)
	, mValid(true)
	, mDeleted(false)
	, mChanged(true)
	, mSelected(false)
{
	mColor.r = mColor.g = mColor.b = 0;
	mFlags.Set(print_flag);
}

Annotation::Annotation(Annotation* copy)
	: mRef(copy->mRef)
	, mContents(copy->mContents)
	, mRect(copy->mRect)
	, mDate(Copy(copy->mDate))
	, mFlags(copy->mFlags)
	, mHasAppearanceStream(copy->mHasAppearanceStream)
	, mColor(copy->mColor)
	, mHasColor(copy->mHasColor)
	, mOpacity(copy->mOpacity)
	, mTitle(Copy(copy->mTitle))
	, mPopup(Copy(copy->mPopup))
	, mValid(copy->mValid)
	, mDeleted(copy->mDeleted)
	, mChanged(copy->mChanged)
	, mSelected(copy->mSelected)
{
}

Annotation::Annotation(Dict* d) 
	: mRef(empty_ref)
	, mDate(NULL)
	, mHasAppearanceStream(false)
	, mHasColor(true)
	, mOpacity(1)
	, mTitle(NULL)

	, mPopup(NULL)
	, mValid(false)
	, mDeleted(false)
	, mChanged(false)
	, mSelected(false)
{
	mRect.x1 = mRect.x2 = mRect.y1 = mRect.y2 = 0;
	mOpacity = 1;
	mColor.r = mColor.g = mColor.b = 0;
	
	Object obj;
	if (d->lookup("Contents", &obj) && obj.isString()) {
		mContents.append(obj.getString());
	}
	obj.free();

	if (d->lookup("M", &obj) && obj.isString()) {
		mDate = new GString(obj.getString()->getCString());
	}
	obj.free();
	
	if (d->lookup("Rect", &obj) && obj.isArray() && obj.arrayGetLength() == 4) {
		Array* a = obj.getArray();
		
		if (ReadNum(a, 0, mRect.x1) &&
			ReadNum(a, 1, mRect.y1) &&
			ReadNum(a, 2, mRect.x2) &&
			ReadNum(a, 3, mRect.y2)) {
		} else goto error;
	} else goto error;
	obj.free();

	if (d->lookup("F", &obj) && obj.isInt()) {
		mFlags.Set(obj.getInt());
	}
	obj.free();	

	mHasAppearanceStream = d->lookup("AP", &obj) && !obj.isNull();
	obj.free();

	if (d->lookup("C", &obj) && obj.isArray() && obj.arrayGetLength() == 3) {
		Array* a = obj.getArray();
		double r, g, b;
		if (ReadNum(a, 0, r) &&
			ReadNum(a, 1, g) &&
			ReadNum(a, 2, b)) {
			mColor.r = dblToCol(r);
			mColor.g = dblToCol(g);
			mColor.b = dblToCol(b);
		} else {
			goto error;
		}
	}
	obj.free();
	
	if (d->lookup("CA", &obj) && obj.isNum()) {
		mOpacity = obj.getNum();
	} 
	obj.free();
	
	if (d->lookup("T", &obj) && obj.isString()) {
		mTitle = new GString(obj.getString());
	}
	obj.free();

	if (d->lookup("Popup", &obj) && obj.isDict()) {
		PopupAnnot* popup = new PopupAnnot(obj.getDict());
		if (popup->IsValid()) LOG("Popup is valid\n");
		else LOG("Popup is invalid\n");
		if (popup->IsValid()) {
			mPopup = popup;
			Object ref;
			if (d->lookupNF("Popup", &ref) && obj.isRef()) {
				Ref r = ref.getRef();
				popup->SetRef(r);
			}
			ref.free();
		} else delete popup;
	}
	obj.free();
			
	SetValid();
	return;

error:
	obj.free();
}

Annotation::~Annotation() {
	delete mDate; 
	delete mTitle; 
	delete mPopup;
}

bool Annotation::ReadNum(Array* a, int i, double &d) {
	Object n;
	bool ok = a->get(i, &n) && n.isNum();
	if (ok) {
		d = n.getNum();
	} 
	n.free();
	return ok;
}

bool Annotation::ReadPoint(Array* a, int i, PDFPoint* p) {
	return ReadNum(a, i, p->x) && ReadNum(a, i + 1, p->y);
}

void Annotation::Print() {
	if (is_empty_ref(mRef)) fprintf(stderr, "Empty Ref\n");
	else fprintf(stderr, "Ref num %d gen %d\n", mRef.num, mRef.gen);
	fprintf(stderr, "Contents:\n%s\n", GetContents()->getCString());
	fprintf(stderr, "Date: %s\n", GetDate());
	fprintf(stderr, "Rect: %f %f %f %f\n", 
		GetRect()->x1, GetRect()->y1, GetRect()->x2, GetRect()->y2);
	fprintf(stderr, "Flags: ");
	if (GetFlags()->Invisible()) fprintf(stderr, "Invisible, ");
	if (GetFlags()->Hidden())    fprintf(stderr, "Hidden, ");
	if (GetFlags()->Print())     fprintf(stderr, "Print, ");
	if (GetFlags()->NoZoom())    fprintf(stderr, "NoZoom, ");
	if (GetFlags()->NoRotate())  fprintf(stderr, "NoRotate, ");
	if (GetFlags()->NoView())    fprintf(stderr, "NoView, ");
	if (GetFlags()->ReadOnly())  fprintf(stderr, "ReadOnly, ");
	fprintf(stderr, "\n");
	fprintf(stderr, "HasAppearanceStream: %s\n", HasAppearanceStream() ? "true" : "false");
	fprintf(stderr, "Color: %d %d %d\n",
		int(GetColor()->r * 255),
		int(GetColor()->g * 255),
		int(GetColor()->b * 255));
	fprintf(stderr, "Color: %f %f %f\n",
		float(GetColor()->r),
		float(GetColor()->g),
		float(GetColor()->b));
	fprintf(stderr, "Opacity: %d\n", int(GetOpacity() * 255));
	fprintf(stderr, "\n");
	fprintf(stderr, "Title: %s\n", GetTitle() ? GetTitle()->getCString() : "NULL");
	if (mPopup) {
		fprintf(stderr, "---Popup:\n");
		mPopup->Print();
	}
}

void Annotation::SetRef(const Ref ref) {
	mRef = ref;
}

void Annotation::SetContents(GString* c) {
	mContents.clear()->append(c);
}

void Annotation::SetString(GString* &s, const char* t) {
	if (t) {
		if (s) {
			s->clear()->append(t);
		} else {
			s = new GString(t);
		}
	} else {
		delete s; s = NULL;
	}
}

void Annotation::SetDate(const char* date) {
	SetString(mDate, date);
}

void Annotation::SetTitle(GString* title) {
	if (mTitle) {
		mTitle->clear()->append(title);
	} else {
		mTitle = new GString(title);
	}
}

void Annotation::MoveTo(PDFPoint p) {
	mRect.x2 = p.x + mRect.x2 - mRect.x1;
	mRect.x1 = p.x;
	mRect.y1 = p.y - mRect.y2 + mRect.y1;
	mRect.y2 = p.y;
}

void Annotation::ResizeTo(double w, double h) {
	mRect.x2 = mRect.x1 + w;
	mRect.y1 = mRect.y2 - h;
}


// Implementation of TextAnnot
TextAnnot::TextAnnot(PDFRectangle rect, text_annot_type type)
	: Annotation(rect)
	, mOpen(true)
	, mName(mTypeNames[type])
	, mType(type)
{
	GfxRGB* c = GetColor();
	c->r = dblToCol(1.0);
	c->g = dblToCol(1.0);
	c->b = dblToCol(0.0);
	GetFlags()->Set(print_flag | no_zoom_flag | no_rotate_flag);
}

TextAnnot::TextAnnot(TextAnnot* copy)
	: Annotation(copy)
	, mOpen(copy->mOpen)
	, mName(copy->mName)
	, mType(copy->mType)
{
	
}

const char* TextAnnot::mTypeNames[TextAnnot::no_of_types-1] = {
	"Comment",
	"Help",
	"Insert",
	"Key",
	"NewParagraph",
	"Note",
	"Paragraph"
};

TextAnnot::TextAnnot(Dict* d) 
	: Annotation(d)
	, mOpen(false)
	, mName("Note")
	, mType(note_type) 
{
	Object obj;
	if (d->lookup("Open", &obj) && obj.isBool()) {
		mOpen = obj.getBool();
	}
	obj.free();
	
	if (d->lookup("Name", &obj) && obj.isName()) {
		mName.clear()->append(obj.getName());
		mType = unknown_type;
		for (int i = 0; i < no_of_types - 1; i++) {
			if (strcmp(obj.getName(), mTypeNames[i]) == 0) {
				mType = (enum text_annot_type)i;
				break;
			}
		}	
	}
	obj.free();
}

void TextAnnot::Print() {
	fprintf(stderr, "Text\n");
	Annotation::Print();
	fprintf(stderr, "Name: %s\n", GetName());
	fprintf(stderr, "Open: %s\n", IsOpen() ? "true" : "false");
}

free_text_justification ToFreeTextJustification(const char* name) {
	if (strcmp(name, "left") == 0) return left_justify;
	if (strcmp(name, "centered") == 0) return centered;
	if (strcmp(name, "right") == 0) return right_justify;
	return left_justify; // default
}

const char* ToString(free_text_justification j) {
	switch (j) {
		case left_justify: return "left";
		case centered: return "centered";
		case right_justify: return "right";
		default: return "left";
	}
}

// Implementation of FreeTextAnnot

void FreeTextAnnot::Init() {
	SetHasColor(false);	
	GfxRGB* c = GetColor();
	c->r = dblToCol(1.0); 
	c->g = dblToCol(1.0); 
	c->b = dblToCol(1.0);
	mFontColor.r = dblToCol(0.0);
	mFontColor.g = dblToCol(0.0);
	mFontColor.b = dblToCol(0.0);
}

FreeTextAnnot::FreeTextAnnot(PDFRectangle rect, PDFFont* font)
	: StyledAnnot(rect, 1.0, 1.0, 1.0)
	, mJustification(left_justify)
	, mFont(font)
	, mFontSize(12)
{
	Init();
	GetBorderStyle()->SetWidth(0.0);
}

FreeTextAnnot::FreeTextAnnot(FreeTextAnnot* copy)
	: StyledAnnot(copy)
	, mAppearance(copy->mAppearance)
	, mJustification(copy->mJustification)
	, mFont(copy->mFont)
	, mFontColor(copy->mFontColor)
	, mFontSize(copy->mFontSize)
{
	
}

FreeTextAnnot::FreeTextAnnot(Dict* d, AcroForm* acroForm) 
	: StyledAnnot(d)
	, mAppearance("")
	, mJustification(left_justify)
	, mFont(NULL)
	, mFontSize(12)
{
	Init();

	Object obj;
	if (d->lookup("DA", &obj) && obj.isString()) {
		mAppearance.append(obj.getString());
	}
	obj.free();

	mJustification = acroForm->GetJustification(); // inherit property from AcroForm
	if (d->lookup("Q", &obj) && obj.isInt()) {
		int j = obj.getInt();
		if (j >= left_justify && j <= right_justify) 
			mJustification = (free_text_justification)j;
	}	
	obj.free();
	
	// use appearance string to extract font short name and size	
	const char* appearance;
	if (mAppearance.cmp("") != 0) {
		appearance = mAppearance.getCString();
	} else {
		// inherit it from AcroForm if it is not defined in annotation
		appearance = acroForm->GetAppearance();
	}
	
	AppearanceStringParser asp(appearance);
	if (asp.IsOK()) {
		mFontSize = asp.GetFontSize();
		mFontColor = *asp.GetColor();
		mFont = acroForm->GetStandardFonts()->FindByShortName(asp.GetFontName());
		if (mFont == NULL) {
			// try to find a matching font
			PDFFont* font = acroForm->FindFontByShortName(asp.GetFontName());
			if (font) {
				mFont = acroForm->GetStandardFonts()->FindByName(font->GetName());
			}
		}
	}
	
	// assign default font
	if (mFont == NULL) {
		mFont = acroForm->GetStandardFonts()->FindByName("Helvetica");
		ASSERT(mFont != NULL);
	}
}

void FreeTextAnnot::Print() {
	fprintf(stderr, "FreeText\n");
	Annotation::Print();
	fprintf(stderr, "Appearance: %s\n", GetAppearance()->getCString());
	fprintf(stderr, "Justification: %s", ToString(mJustification));
	fprintf(stderr, "Font: ");
	if (mFont) {
		fprintf(stderr, "\n");
		mFont->Print();
	} else {
		fprintf(stderr, "<null>\n");
	}
}

void FreeTextAnnot::SetAppearance(GString* ap) {
	mAppearance.clear()->append(ap);
}

void FreeTextAnnot::SetJustification(free_text_justification j) {
	mJustification = j;
}

void FreeTextAnnot::SetFont(PDFFont* font) {
	mFont = font;
}

// Implementation of StyledAnnot
StyledAnnot::StyledAnnot(PDFRectangle rect, float r, float g, float b)
	: Annotation(rect)
{
	GfxRGB* c = GetColor();
	c->r = dblToCol(r); 
	c->g = dblToCol(g); 
	c->b = dblToCol(b);
}

StyledAnnot::StyledAnnot(StyledAnnot* copy) 
	: Annotation(copy)
	, mStyle(copy->mStyle)
{
}

StyledAnnot::StyledAnnot(Dict* d) 
	: Annotation(d)
{
	Object obj;
	if (d->lookup("BS", &obj) && obj.isDict()) {
		mStyle.Set(obj.getDict());
	}
	obj.free();
}

void StyledAnnot::Print() {
	Annotation::Print();
	fprintf(stderr, "BorderStyle:\n");
	fprintf(stderr, "  Width: %d\n", GetBorderStyle()->GetWidth());
	fprintf(stderr, "  Style: ");
	switch (GetBorderStyle()->GetStyle()) {
		case BorderStyle::solid_style: fprintf(stderr, "solid"); break;
		case BorderStyle::dashed_style: fprintf(stderr, "dashed"); break;
		case BorderStyle::beveled_style: fprintf(stderr, "beveled"); break;
		case BorderStyle::inset_style: fprintf(stderr, "inset"); break;
		case BorderStyle::underline_style: fprintf(stderr, "underline"); break;
		default: fprintf(stderr, "unknown"); break;
	};
	fprintf(stderr, "\n");
}

// Implementation of LineAnnot
LineAnnot::LineAnnot(PDFRectangle rect, PDFPoint* line)
	: StyledAnnot(rect, 0, 0, 0)
{
	mLine[0] = line[0];
	mLine[1] = line[1];
}

LineAnnot::LineAnnot(LineAnnot* copy)
	: StyledAnnot(copy)
{
	mLine[0] = copy->mLine[0];
	mLine[1] = copy->mLine[1];
} 

LineAnnot::LineAnnot(Dict* d) 
	: StyledAnnot(d)
{
	if (CheckInvalid()) return;
	
	Object obj;
	if (d->lookup("L", &obj) && obj.isArray() && obj.arrayGetLength() == 4) {
		Array* a = obj.getArray();
		if (ReadPoint(a, 0, &mLine[0]) &&
			ReadPoint(a, 2, &mLine[1])) {
		} else goto error;		
	} else {
		goto error;
	}
	obj.free();
	SetValid();
	return;
error:
	obj.free();
}

void LineAnnot::Print() {
	fprintf(stderr, "Line\n");
	StyledAnnot::Print();
}


void SquareAnnot::Print() {
	fprintf(stderr, "Square\n");
	StyledAnnot::Print();
}

void CircleAnnot::Print() {
	fprintf(stderr, "Circle\n");
	StyledAnnot::Print();
}

// Implementation of MarkupAnnot
MarkupAnnot::MarkupAnnot(PDFRectangle rect, float r, float g, float b, PDFQuadPoints* p, int l)
	: StyledAnnot(rect, r, g, b)
	, mLength(l)
	, mQuadPoints(p)
{
}

MarkupAnnot::MarkupAnnot(MarkupAnnot* copy)
	: StyledAnnot(copy)
	, mLength(copy->mLength)
	, mQuadPoints(NULL)
{
	if (mLength > 0) {
		mQuadPoints = new PDFQuadPoints[mLength];
		for (int i = 0; i < mLength; i++) {
			mQuadPoints[i] = copy->mQuadPoints[i];
		}
	}
}
 

MarkupAnnot::MarkupAnnot(Dict* d) 
	: StyledAnnot(d)
	, mLength(0)
	, mQuadPoints(NULL)
{
	if (CheckInvalid()) return;
	
	Object qp;
	if (d->lookup("QuadPoints", &qp) && qp.isArray() && qp.arrayGetLength() % 8 == 0) {
		mLength = qp.arrayGetLength() / 8;
		if (mLength == 0) goto error;
		
		mQuadPoints = new PDFQuadPoints[mLength];
		int index = 0;
		for (int j = 0; j < mLength; j++) {
			PDFQuadPoints* quad = &mQuadPoints[j];
			for (int i = 0; i < 4; i++) {
				if (!ReadPoint(qp.getArray(), index, &quad->q[i])) goto error;
				index += 2;
			}
		}
	} else goto error;
	qp.free();
	
	
	SetValid();	
	return;
			
error:
	qp.free();
	delete []mQuadPoints;
}

MarkupAnnot::~MarkupAnnot() {
	delete []mQuadPoints;
}

void MarkupAnnot::Print() {
	fprintf(stderr, "Markup\n");
	StyledAnnot::Print();
}

void MarkupAnnot::MoveTo(PDFPoint p) {
	PDFPoint d(p.x - GetRect()->x1, p.y - GetRect()->y2);
	for (int i = 0; i < mLength; i++) {
		PDFQuadPoints& p = mQuadPoints[i];
		p.q[0] += d; p.q[1] += d; p.q[2] += d; p.q[3] += d;
	}
	StyledAnnot::MoveTo(p);
}

void MarkupAnnot::ResizeTo(double w, double h) {
	double oldW = GetRect()->x2 - GetRect()->x1;
	double oldH = GetRect()->y2 - GetRect()->y1;
	PDFPoint origin(GetRect()->x1, GetRect()->y2);
	for (int i = 0; i < mLength; i++) {
		PDFQuadPoints& q = mQuadPoints[i];
		for (int j = 0; j < 4; j ++) {
			PDFPoint& p = q[j];
			p -= origin;
			p.x *= w/oldW;
			p.y *= h/oldH;
			p += origin;
		}
	}
	StyledAnnot::ResizeTo(w, h);
}

// Implementation of HighlightAnnot

static PDFQuadPoints* QuadPoints(PDFRectangle rect) {
	PDFQuadPoints* ps = new PDFQuadPoints[1];
	PDFQuadPoints& p = *ps;
	p[2] = PDFPoint(rect.x1, rect.y1);
	p[3] = PDFPoint(rect.x2, rect.y1);
	p[0] = PDFPoint(rect.x1, rect.y2);
	p[1] = PDFPoint(rect.x2, rect.y2);
	return ps;
}

static int NofQuadPoints() {
	return 1;
}

HighlightAnnot::HighlightAnnot(PDFRectangle rect)
	: MarkupAnnot(rect, 1.0, 0.992, 0.18, QuadPoints(rect), NofQuadPoints())
{
}

// Implementation of UnderlineAnnot

UnderlineAnnot::UnderlineAnnot(PDFRectangle rect)
	: MarkupAnnot(rect, 0.0, 0.5, 0.0, QuadPoints(rect), NofQuadPoints())
{
}

// Implementation of SquigglyAnnot

SquigglyAnnot::SquigglyAnnot(PDFRectangle rect)
	: MarkupAnnot(rect, 0.0, 0.5, 0.0, QuadPoints(rect), NofQuadPoints())
{
}

// Implementation of StrikeOutAnnot
StrikeOutAnnot::StrikeOutAnnot(PDFRectangle rect)
	: MarkupAnnot(rect, 0.702, 0.0015, 0.0, QuadPoints(rect), NofQuadPoints())
{
}

// Implementation of StampAnnot
StampAnnot::StampAnnot(StampAnnot* copy) 
	: Annotation(copy)
	, mName(copy->mName) 
{
}

StampAnnot::StampAnnot(Dict* d) 
	: Annotation(d)
	, mName("Draft") 
{
	Object obj;
	if (d->lookup("Name", &obj) && obj.isName()) {
		mName.clear()->append(obj.getName());	
	}
	obj.free();
}

void StampAnnot::Print() {
	fprintf(stderr, "StampAnnot\n");
	Annotation::Print();
	fprintf(stderr, "Name: %s\n", GetName());
}

// Implementation of InkAnnot
InkAnnot::InkAnnot(InkAnnot* copy) 
	: StyledAnnot(copy)
	, mLength(copy->mLength)
	, mInkList(NULL) 
{
	if (mLength > 0) {
		mInkList = new PDFPoints[mLength];
		for (int i = 0; i < mLength; i ++) 
			mInkList[i] = copy->mInkList[i];
	}
}

InkAnnot::InkAnnot(Dict* d) 
	: StyledAnnot(d)
	, mLength(0)
	, mInkList(NULL) 
{
	if (CheckInvalid()) return;
	Object obj;
	if (d->lookup("InkList", &obj) && obj.isArray()) {
		Array* a = obj.getArray();
		mLength = a->getLength();
		mInkList = new PDFPoints[mLength];
		for (int i = 0; i < mLength; i++) {
			PDFPoints* p = PathAt(i);
			Object obj2;
			if (a->get(i, &obj2) && obj2.isArray() && obj2.arrayGetLength() % 2 == 0) {
				const int n = obj2.arrayGetLength() / 2;
				p->SetLength(n);
				for (int j = 0; j < n; j ++) {
					if (!ReadPoint(obj2.getArray(), 2*j, p->PointAt(j))) {
						obj2.free(); goto error;
					}
				}
			} else {
				obj2.free(); goto error;
			}
			obj2.free();
		}
	} else goto error;
	obj.free();
	SetValid();
	return;
	
error:
	obj.free();
}

InkAnnot::~InkAnnot() {
	if (mInkList) {		
		delete []mInkList;
	}
}

void InkAnnot::Print() {
	fprintf(stderr, "Ink\n");
	StyledAnnot::Print();
}

// Implementation of PopupAnnot
PopupAnnot::PopupAnnot(PDFRectangle rect)
	: Annotation(rect)
	, mParentRef(empty_ref)
{
	GetFlags()->Set(print_flag | no_zoom_flag | no_rotate_flag);
} 

PopupAnnot::PopupAnnot(PopupAnnot* copy)
	: Annotation(copy)
	, mParentRef(empty_ref)
{
}

PopupAnnot::PopupAnnot(Dict* annot)
	: Annotation(annot)
	, mParentRef(empty_ref)
{
	if (CheckInvalid()) return;
	
	SetValid();
}

void PopupAnnot::Print() {
	fprintf(stderr, "Popup\n");
	Annotation::Print();
}

// Implementation of FileAttachmentAnnot
const char* FileAttachmentAnnot::mTypeNames[FileAttachmentAnnot::no_of_types-1] = {
	"Graphic",
	"PaperClip",
	"PushPin",
	"Tag"
};

FileAttachmentAnnot::FileAttachmentAnnot(FileAttachmentAnnot* copy)
	: Annotation(copy)
	, mName(copy->mName)
	, mFileName(copy->mFileName)
	, mType(copy->mType)
{
}

FileAttachmentAnnot::FileAttachmentAnnot(Dict* annot)
	: Annotation(annot)
	, mName("PushPin")
	, mType(push_pin_type)
{
	if (CheckInvalid()) return;

	Object obj;
	if (annot->lookup("Name", &obj) && obj.isName()) {
		mName.clear()->append(obj.getName());
		mType = unknown_type;
		for (int i = 0; i < no_of_types-1; i++) {
			if (strcmp(obj.getName(), mTypeNames[i]) == 0) {
				mType = (enum attachment_type)i;
				break;
			}
		}
	}
	obj.free();
	
	// File specification
	if (annot->lookup("FS", &obj) && obj.isDict() && obj.dictIs("Filespec")) {
		mFileSpec.SetTo(obj.getDict());
	}
	obj.free();
	
	SetValid();
}

bool FileAttachmentAnnot::Save(XRef *xref, const char *file) {
	return mFileSpec.Save(xref, file);
}

void FileAttachmentAnnot::Print() {
	fprintf(stderr, "FileAttachment\n");
	Annotation::Print();
	fprintf(stderr, "Name: %s\n", GetName());
}


// Implementation of AnnotSorter

AnnotSorter::AnnotSorter()
{
}

AnnotSorter::~AnnotSorter()
{
}


// Implementation of AnnotName

AnnotName::AnnotName()
{
}

AnnotName::~AnnotName()
{
}


// Implementation of AcroForm

PDFStandardFonts* AcroForm::mStandardFonts = NULL;

PDFStandardFonts* AcroForm::GetStandardFonts() {
	if (mStandardFonts == NULL) {
		mStandardFonts = new PDFStandardFonts();
	}
	return mStandardFonts;
}


AcroForm::AcroForm(XRef* xref, Object* acroFormRef) 
	: mRef(empty_ref)
	, mJustification(left_justify)
{
	GetStandardFonts()->Reset();
	if (!acroFormRef->isRef()) return;
	
	Object acroForm;
	if (acroFormRef->fetch(xref, &acroForm) == NULL) return;
	

	if (!acroForm.isDict()) {
		acroForm.free();
		return;	
	}

	mRef = acroFormRef->getRef();

	Object obj;
	Dict* d = acroForm.getDict();
	
	// default appearance string
	if (d->lookup("DA", &obj) && obj.isString()) {
		mAppearance.append(obj.getString());
	}
	obj.free();

	// default quadding
	if (d->lookup("Q", &obj) && obj.isInt()) {
		int j = obj.getInt();
		if (j >= left_justify && j <= right_justify) 
			mJustification = (free_text_justification)j;
	}	
	obj.free();

	if (d->lookup("DR", &obj) && obj.isDict()) {
		Dict* resources = obj.getDict();
		Object obj2;
		if (resources->lookup("Font", &obj2) && obj2.isDict()) {
			Dict* font = obj2.getDict();
			for (int i = 0; i < font->getLength(); i ++) {
				Object obj3, obj4;
				if (font->getVal(i, &obj3) && obj3.isDict() &&
					font->getValNF(i, &obj4) && obj4.isRef()) {
					ParseFont(font->getKey(i), obj4.getRef(), obj3.getDict());
				}
				obj3.free(); obj4.free();
			}	
		}
		obj2.free();
	}
	obj.free();
}

AcroForm::~AcroForm() {
	std::list<PDFFont*>::iterator it;
	for (it = mFonts.begin(); it != mFonts.end(); it ++) {
		PDFFont* font = *it;
		delete font;
	}
}

void AcroForm::ParseFont(const char* shortName, Ref ref, Dict* dict) {
	bool isType1;
	GString baseFont;
	GString encoding;
	bool hasSupportedEncoding = false;
	PDFFont* font;
	int firstChar = 0;
	int lastChar = 255;
	
	Object obj;
	if (dict->lookup("Type", &obj) == NULL || !obj.isName() || strcmp(obj.getName(), "Font") != 0) {
		goto error;
	}
	obj.free();

	isType1 = dict->lookup("Subtype", &obj) && obj.isName() && strcmp(obj.getName(), "Type1") == 0;
	obj.free();

	if (dict->lookup("FirstChar", &obj) && obj.isNum()) {
		firstChar = (int)obj.getNum();
	}
	obj.free();
	
	if (dict->lookup("LastChar", &obj) && obj.isNum()) {
		lastChar = (int)obj.getNum();
	}
	obj.free();
	
	if (dict->lookupNF("Encoding", &obj) && obj.isName()) {
		encoding.append(obj.getName());
		if (encoding.cmp("WinAnsiEncoding") == 0) {
			hasSupportedEncoding = true;
		}
	}
	obj.free();

	if (dict->lookup("BaseFont", &obj) && obj.isName()) {
		baseFont.append(obj.getName());
	} else {
		goto error;
	}
	obj.free();
	
	// add font to list
	font = new PDFFont();
	font->SetRef(ref);
	font->SetName(baseFont.getCString());
	font->SetShortName(shortName);
	mFonts.push_back(font);
		
	// If it is a standard font set reference to it and its short name
	// Note: AnnotWriter must not write a referenced standard font. 
	if (isType1 && lastChar-firstChar == 255 && hasSupportedEncoding) {
		font = GetStandardFonts()->FindByName(baseFont.getCString());
		if (font != NULL) {
			if (is_empty_ref(font->GetRef())) {
				font->SetRef(ref);
				font->SetShortName(shortName);
			}
		}
	}
	
	return;
		
error:
	obj.free();
}

PDFFont* AcroForm::FindFontByShortName(const char* name) {
	std::list<PDFFont*>::iterator it;
	for (it = mFonts.begin(); it != mFonts.end(); it ++) {
		PDFFont* font = *it;
		if (strcmp(font->GetShortName(), name) == 0) {
			return font;
		}
	}
	return NULL;
}

// Implementation of Annotations
Annotations::Annotations(Annotations* copy)
	: mMax(copy->mLength)
	, mLength(mMax)
	, mAnnots(NULL)
{
	if (mLength > 0) {
		mAnnots = new AnnotationPtr[mMax];
		for (int i = 0; i < mLength; i ++) {
			mAnnots[i] = copy->At(i)->Clone();
		}
	}		
}

Annotations::Annotations(Object* annots, AcroForm* acroForm) 
	: mMax(0)
	, mLength(0)
	, mAnnots(NULL)
{	
	if (!annots->isArray() || annots->arrayGetLength() <= 0) return;
	
	mMax = annots->arrayGetLength();
	mAnnots = new AnnotationPtr[mMax];

	for (int i = 0; i < mMax; i ++) mAnnots[i] = NULL;

	for (int i = 0; i < annots->arrayGetLength(); i++) {
		Ref ref = empty_ref;
		Object r;
		if (annots->arrayGetNF(i, &r) && r.isRef()) {
			ref = r.getRef();
		}
		Object annot;
		if (annots->arrayGet(i, &annot) && annot.isDict()) {
			Object type; // optional, "Annot" if present
			if (annot.dictLookup("Type", &type) && (type.isNull() || 
				type.isName() && strcmp(type.getName(), "Annot") == 0)) {

				Object subType;
				if (annot.dictLookup("Subtype", &subType) && subType.isName()) {
					const char* s = subType.getName();
					Annotation* a = NULL;
					LOG(s); CR;
					if (strcmp(s, "Text") == 0) {
						a = new TextAnnot(annot.getDict());
					} else if (strcmp(s, "FreeText") == 0) {
						a = new FreeTextAnnot(annot.getDict(), acroForm); 
					} else if (strcmp(s, "Highlight") == 0) {
						a = new HighlightAnnot(annot.getDict());
					} else if (strcmp(s, "Underline") == 0) {
						a = new UnderlineAnnot(annot.getDict());
					} else if (strcmp(s, "Squiggly") == 0) {
						a = new SquigglyAnnot(annot.getDict());
					} else if (strcmp(s, "StrikeOut") == 0) {
						a = new StrikeOutAnnot(annot.getDict());
					} else if (strcmp(s, "Line") == 0) {
						a = new LineAnnot(annot.getDict());
					} else if (strcmp(s, "Square") == 0) {
						a = new SquareAnnot(annot.getDict());
					} else if (strcmp(s, "Circle") == 0) {
						a = new CircleAnnot(annot.getDict());
					} else if (strcmp(s, "Stamp") == 0) {
						a = new StampAnnot(annot.getDict());
					} else if (strcmp(s, "Ink") == 0) {
						a = new InkAnnot(annot.getDict());
					} else if (strcmp(s, "FileAttachment") == 0){
						a = new FileAttachmentAnnot(annot.getDict());
					}
						 
					if (a && a->IsValid()) {
						a->SetRef(ref);
						mAnnots[mLength++] = a; 
					} else {
						#ifdef DEBUG
						fprintf(stderr, "Warning: Could not parse Annotation %s\n", s);
						#endif
						delete a;
					}
				} 
				subType.free();
			}
			type.free();
		} // else skip it 
		r.free();
		annot.free();
	}
}


Annotations::~Annotations() {
	if (mAnnots) {
		for (int i = 0; i < mLength; i++) {
			delete mAnnots[i];
		}
		delete []mAnnots;
		mAnnots = NULL;
	}
}

Annotation* Annotations::OverAnnotation(double x, double y, bool edit) {
	for (int i = 0; i < mLength; i++) {
		Annotation* a = mAnnots[i];
		if (a->IsDeleted()) continue;
		PDFRectangle* r = a->GetRect();
		if (r->x1 <= x && x <= r->x2 && r->y1 <= y && y <= r->y2) {
			AnnotFlags* f = a->GetFlags();
			if (edit || !f->Invisible() && !f->Hidden() && !f->NoView())
				return a;
		}
	}
	return NULL;
}

void Annotations::Iterate(AnnotVisitor* v) {
	for (int i = 0; i < mLength; i++) {
		mAnnots[i]->Visit(v);
	}
}

static AnnotSorter* sorter = NULL;

int cmp_func(const void* x, const void* y) {
	AnnotationPtr* a = (AnnotationPtr*)x;
	AnnotationPtr* b = (AnnotationPtr*)y;
	(*a)->Visit(sorter);
	int i = sorter->GetResult();
	(*b)->Visit(sorter);
	if (i < sorter->GetResult()) return -1;
	if (i == sorter->GetResult()) return 0;
	return 1;	
}

void Annotations::Sort(AnnotSorter* s) {
	sorter = s;
	qsort(mAnnots, mLength, sizeof(AnnotationPtr), cmp_func);
}

void Annotations::Resize(int m) {
	if (mMax < m) {
		if (mMax + 10 > m) m = mMax + 10;
		mMax = m;
		AnnotationPtr* a = new AnnotationPtr[mMax];
		int i = 0;
		for (; i < mLength; i ++) a[i] = mAnnots[i];
		for (; i < mMax; i ++)    a[i] = NULL;
		delete []mAnnots;
		mAnnots = a;
	}
}

void Annotations::Append(Annotation* a) {
	Resize(mLength + 1);
	mAnnots[mLength] = a;
	mLength ++;
}

Annotation* Annotations::Remove(int i) {
	Annotation* a = NULL;
	if (i >= 0 && i < mLength) {
		a = At(i);
		const int n = mLength - 2;
		for (; i < n; i ++) mAnnots[i] = mAnnots[i+1];
		mAnnots[i] = NULL;
		mLength --;
	}	
	return a;	
}

bool Annotations::HasChanged() const {
	for (int i = 0; i < Length(); i ++) {
		Annotation* a = At(i);
		if (a->HasChanged()) return true;
	}
	return false;
}

// Implementation of AnnotsList
AnnotsList::AnnotsList(AnnotsList* copy) 
	: mAnnots(NULL)
	, mLength(copy->mLength)
{
	if (mLength > 0) {
		mAnnots = new AnnotationsPtr[mLength];
		for (int i = 0; i < mLength; i ++) {
			Annotations* a = copy->Get(i);
			mAnnots[i] = a ? new Annotations(a) : NULL;
		}		
	}
}
AnnotsList::AnnotsList()
	: mAnnots(NULL)
	, mLength(0)
{	
}

AnnotsList::~AnnotsList()
{
	MakeEmpty();
}

void AnnotsList::MakeEmpty() {
	if (mAnnots) {
		for (int i = 0; i < mLength; i++) delete mAnnots[i];
		delete []mAnnots;
		mAnnots = NULL; mLength = 0;
	}
}

void AnnotsList::SetSize(int size) {
	MakeEmpty();
	ASSERT(size >= 1);
	mLength = size;
	mAnnots = new AnnotationsPtr[size];
	for (int i = 0; i < mLength; i++) mAnnots[i] = NULL;
}

Annotations* AnnotsList::Get(int i) const {
	ASSERT(0 <= i && i < mLength);
	ASSERT(mAnnots);
	return mAnnots[i];
}
	
void AnnotsList::Set(int i, Annotations* a) {
	ASSERT(0 <= i && i < mLength);
	ASSERT(mAnnots);
	mAnnots[i] = a;
}

bool AnnotsList::HasChanged() const {
	for (int i = 0; i < mLength; i ++) {
		Annotations* a = Get(i);
		if (a && a->HasChanged()) return true;
	}
	return false;
}

// Implementation of AppearanceStringParser
AppearanceStringParser::AppearanceStringParser(const char* as) 
{
	char* text = strdup(as); // copy string, because it will be changed 
	std::stack<const char*> operands;
	int32 length;
	char* s;
	char* end;
	mColor.r = mColor.g = mColor.b = dblToCol(0.0);
	
	mOK = false;
	length = strlen(text);
	s = text;
	end = s + length;
	for (; s < end && (s = strtok(s, " []\r\n\t")) != NULL; s += strlen(s) + 1) {
		if (isdigit(*s)) {
			operands.push(s);
		} else if (*s == '/') {
			operands.push(s+1);
		} else {
			if (strcmp(s, "Tf") == 0 && operands.size() >= 2) {
				mFontSize = atof(operands.top()); operands.pop();
				mFontName.clear()->append(operands.top()); operands.pop();
				mOK = true; 
			} else if (strcmp(s, "rg") == 0 && operands.size() >= 3) {
				mColor.b = dblToCol(atof(operands.top())); operands.pop();
				mColor.g = dblToCol(atof(operands.top())); operands.pop();
				mColor.r = dblToCol(atof(operands.top())); operands.pop();
			} else {
#if DEBUG
				fprintf(stderr, "AppearanceStringParser: Unsupported operand '%s'\n", s);
#endif
			}
			if (!operands.empty()) {
#if DEBUG
				fprintf(stderr, "AppearanceStringParser: Not all operands consumed in operation '%s'!\n", s);
#endif
			}
		}
	}
	free(text);
}

// Implementation of AnnotUtils

bool AnnotUtils::InUCS2(GString* s) {
	if (s->getLength() < 2) {
		return false;
	}
	const unsigned char* t = (const unsigned char*)s->getCString();
	return t[0] == 0xfe && t[1] == 0xff;
}

GString* AnnotUtils::EscapeString(GString* text) {
	const char* t = text->getCString();
	int len = text->getLength();
	GString* s = new GString();
	while (len > 0) {
		len --;

		switch (*t) {
			case '\t':
				s->append("\\t");
				break;
			case '\n':
				s->append("\\n");
				break;
			case '\f':
				s->append("\\f");
				break;
			case '\r':
				s->append("\\r");
				break;
			case '\b': // backspace
				s->append("\\b");
				break;
			case '(':
			case ')':
			case '\\':
				s->append('\\'); s->append(*t);
				break;
			default:
				s->append(*t);
		}
		t ++;
	}
	return s;
}

GString* AnnotUtils::EscapeName(const char* t) {
	GString* s = new GString();
	char b[80];
	if (t != NULL) {
		while (*t != 0) {
			if (!isprint(*t) || isspace(*t) || *t == '/') {
				int i = *t && 0xff;
				sprintf(b, "#%2.2x", i);
			} else {
				s->append(*t);
			}
			t ++;
		}
	}
	return s;
}

void AnnotUtils::CurrentDate(GString* date) {
	time_t tme;
	struct tm tm;
	char   s[80];
	time(&tme);
	tm = *localtime(&tme);
	char o = tm.tm_gmtoff >= 0 ? '+' : '-';
	if (tm.tm_gmtoff < 0) tm.tm_gmtoff = -tm.tm_gmtoff;
	int min = tm.tm_gmtoff / 3600;
	int sec = (tm.tm_gmtoff / 60) % 60;
	sprintf(s, "D:%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%c%2.2d'%2.2d'",
		1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec,
		o, min, sec);
	date->append(s);
}

