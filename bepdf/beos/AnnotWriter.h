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

#ifndef WRITE_ANNOTATION_H
#define WRITE_ANNOT_H

// xpdf
#include <Object.h>
#include <XRef.h>
#include <Catalog.h>
#include <PDFDoc.h>

#include "Annotation.h"
#include "AnnotAppearance.h"

struct XRefItem {
	int offset;  // the file offset
	int num;     // the reference number
	int gen;     // the reference generation
	bool used;   // is reference used?
};

class XRefTable {
	enum {
		INITIAL_INCREMENT = 30,
		INCREMENT         = 10,
		DEAD_GEN          = 65535
	};

	XRef*      mXRef;    // points to the files xref table
	int        mLength;  // allocated entries
	int        mSize;    // used entries
	XRefEntry* mEntries; // the updated xref table
	
	// resize mEntries to at least length elements
	void       Resize(int length); 
	bool       InRange(int num);
	XRefEntry* GetUnusedHead();
	void       InsertInUnusedList(int num, XRefEntry* e);
	Ref        ActivateUnusedEntry(XRefEntryType type);
	Ref        AppendNewRef(XRefEntryType type);
	
	XRefTable() { }	

public:
	XRefTable(XRef* xref);
	~XRefTable();
	
	XRefEntry* GetXRef(int num);
	int        GetSize();
	bool       HasChanged(int num);

	void DeleteRef(Ref ref);
	Ref  GetNewRef(XRefEntryType type);
	void SetOffset(Ref ref, int offset);

	bool NextGroup(int start, int* num, int* nof);
};

// We CAN write Line and Ink, but adding new annotations of this type is
// not implemented. 
class AnnotTester : public AnnotVisitor {
public:
	AnnotTester();
	~AnnotTester();

	void DoText(TextAnnot* a)                     { mResult = true; }
	void DoLink(LinkAnnot* a)                     { mResult = false; }
	void DoFreeText(FreeTextAnnot* a)             { mResult = true; }
	void DoLine(LineAnnot* a)                     { mResult = false; }
	void DoSquare(SquareAnnot* a)                 { mResult = true; }
	void DoCircle(CircleAnnot* a)                 { mResult = true; }
	void DoHighlight(HighlightAnnot* a)           { mResult = true; }
	void DoUnderline(UnderlineAnnot* a)           { mResult = true; }
	void DoSquiggly(SquigglyAnnot* a)             { mResult = true; }
	void DoStrikeOut(StrikeOutAnnot* a)           { mResult = true; }
	void DoStamp(StampAnnot* a)                   { mResult = false; }
	void DoInk(InkAnnot* a)                       { mResult = false; }
	void DoPopup(PopupAnnot* a)                   { mResult = true; }
	void DoFileAttachment(FileAttachmentAnnot* a) { mResult = false; }
	void DoSound(SoundAnnot* a)                   { mResult = false; }
	void DoMovie(MovieAnnot* a)                   { mResult = false; }
	void DoWidget(WidgetAnnot* a)                 { mResult = false; }
	void DoPrinterMark(PrinterMarkAnnot* a)       { mResult = false; }
	void DoTrapNet(TrapNetAnnot* a)               { mResult = false; }
	
	bool CanWrite() { return mResult; }	

private:
	bool mResult;
};

bool CanWrite(Annotation* annot);

class AnnotWriter : public AnnotVisitor {
	PDFDoc*         mDoc;
	AnnotsList      mAnnots;
	AcroForm*       mAcroForm;
	XRef*           mXRef;
	XRefTable       mXRefTable;
	// changed during pdf generation:
	Ref             mPageRef;
	Ref             mASRef;
	Ref             mInfoRef;
	Ref             mAcroFormRef;
	std::list<PDFFont*>  mTemporaryFonts; // not already stored in old PDF file
	std::list<PDFFont*>  mWrittenFonts;

	FILE* mFile;
	int   mXRefOffset;
	Object mAnnot; // used by UpdateAnnot & visitor
	bool   mAnnotValid;

	friend void test_annot_writer(PDFDoc* doc, int page, AnnotsList* list);

	void Write(const char* s);
	void Write(GString* s);
	void Write(Ref ref);
	void WriteCr();
	void WriteCrLf();
	int  Tell();
	void InsertWhiteSpace(Object* o);
	void WriteObject(Object* o);
	void WriteObject(Ref ref, Object* obj, GString* stream = NULL);
	bool IsInList(char* s, char* list[]);
	void CopyDict(Object* in, Object* out, char* excludeKeys[] = NULL);
	bool WriteXRefTable();

	Ref  GetModDateRef(Ref infoDictRef);
	Ref  GetInfoDictRef();
	void CopyInfoDict(Object* dict);
	void GetCurrentDate(GString* date);
	void WriteModDate(Ref ref);
	void UpdateInfoDict();
	bool WriteFileTrailer();

/* AcroFrom:
	1. Assign unique short names to fonts
	   1.1. Take names from existing AcroFrom DR assign ref to font (done in AcroForm constructor)
	   1.2. Assign to remainig fonts names in the form /F%d
	2. Write all new or changed FreeText annotations
	   2.1. Write annotation and assign ref if necessary 
	   2.2. if Font has no ref, write font and assign a new ref (record font)
	3. If fonts have been recorded, update AcroFrom
	   3.1. If form exists copy contents except DR
	   3.2. If form does not exist create new one
	   3.3. Copy DR except Font array
	   3.4. Copy Font array and add new fonts (use info from step 2.2.)
	4. If fonts have been recorded and AcroFrom does not exist in
	   Catalog copy Catalog and add ref to AcroFrom
*/
	void AssignShortFontNames();
	void UnassignShortFontNames();
	void WriteFont(PDFFont* font);
	void AddFonts(Object* dict, std::list<PDFFont*>* fonts);
	void UpdateAcroForm();
	void UpdateCatalog();
	
	bool CopyFile(const char* name);

	bool HasRef(Object* dict, const char* key, Ref& ref);
	bool HasAnnotRef(Object* page, Ref &annotRef);
	bool HasEmbeddedContent(Object* page);
	bool CopyContentStream(Object* page);
	bool CopyPage(Object* page, Ref pageRef, Ref annotsArray);
	bool UpdatePage(int pageNo, Annotations* annots, Ref& annotsArray);

	void AddToAnnots(Object* array, Annotation* a);
	bool UpdateAnnotArray(int pageNo, Annotations* annots, Ref annotsArray);
	
	void AddRef(Object* dict, char* key, Ref r);
	void AddBool(Object* dict, char* key, bool b);
	void AddName(Object* dict, char* key, char* name);
	void AddString(Object* dict, char* key, GString* string);
	void AddString(Object* dict, char* key, char* string);
	void AddInteger(Object* dict, char* key, int i);
	void AddReal(Object* dict, char* key, double r);
	void AddReal(Object* array, double r);
	void AddRect(Object* dict, char* key, PDFRectangle* rect);
	void AddColor(Object* dict, char* key, GfxRGB* color);
	void AddDict(Object* dict, char* key, Object* d); // does NOT copy d!!!
	void AddAnnotSubtype(char* type);
	void AddAnnotContents(Annotation* a);
	bool HasAppearanceStream(Annotation* a);
	void DoAnnotation(Annotation* a);
	void DoStyledAnnot(StyledAnnot* s);
	void DoMarkupAnnot(MarkupAnnot* m);
	bool WriteAS(Ref& ref, Annotation* annot);
	bool UpdateAnnot(Annotation* annot);

public:
	AnnotWriter(XRef* xref, PDFDoc* doc, AnnotsList* list, AcroForm* acroForm);
	~AnnotWriter();
	bool WriteTo(const char* name);

	// visitor functionality
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
