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

#include "AnnotWriter.h"

#include <Debug.h>
#ifndef __POWERPC__
	#include <stdlib.h>
#endif
#include <ctype.h>
#include <time.h>
#include <stdio.h>


// Implementation of XRefTable

XRefTable::XRefTable(XRef* xref) 
	: mXRef(xref)
	, mLength(xref->getSize() + INITIAL_INCREMENT)
	, mSize(xref->getSize())
	, mEntries((XRefEntry*)malloc(sizeof(XRefEntry) * mLength))
{
	for (int i = 0; i < mSize; i++) {
		mEntries[i] = *xref->getEntry(i);
	}
}

XRefTable::~XRefTable() {
	free(mEntries); mEntries = NULL;
}

// increase mEntries if necessary only
void XRefTable::Resize(int l) {
	mSize = l;
	if (mLength < l) {
		mLength = l + INCREMENT;
		mEntries = (XRefEntry*)realloc(mEntries, sizeof(XRefEntry) * mLength);
		ASSERT(mEntries != NULL);
	}
}

bool XRefTable::InRange(int num) {
	return num >= 0 && num < mSize;
}

XRefEntry* XRefTable::GetUnusedHead() {
	XRefEntry* head = GetXRef(0);
	ASSERT(head->gen == DEAD_GEN && head->type == xrefEntryFree);
	return head;
}

// simply insert "e" after the head entry and its successor
void XRefTable::InsertInUnusedList(int num, XRefEntry* e) {
	XRefEntry* head = GetUnusedHead();
	e->offset = head->offset;
	head->offset = num;
}

// return (available) successor of head entry in unused list
Ref XRefTable::ActivateUnusedEntry(XRefEntryType type) {
	ASSERT(type != xrefEntryFree);
	XRefEntry* head = GetUnusedHead();
	XRefEntry* prev;
	XRefEntry* cur = head;
	do {
		prev = cur;
		cur = GetXRef(head->offset); // skip "dead" entries
	} while (cur != head && cur->gen == DEAD_GEN);

	if (cur != head) {
		// create a Ref for current entry
		Ref ref;
		ref.num = prev->offset; ref.gen = cur->gen;
		// unlink entry from list
		prev->offset = cur->offset;
		// mark entry as used
		cur->type = type; cur->offset = (Guint)-1;
		return ref;
	} else {
		return empty_ref;
	}
}

XRefEntry* XRefTable::GetXRef(int num) {
	ASSERT(InRange(num));
	return &mEntries[num];	
}

int XRefTable::GetSize() {
	return mSize;
}

bool XRefTable::HasChanged(int num) {
	ASSERT(InRange(num));
	if (num >= mXRef->getSize()) return true;
	if (num == 0) return true;
	XRefEntry* o = mXRef->getEntry(num);
	XRefEntry* n = GetXRef(num);
	return o->offset != n->offset ||
		o->gen != n->gen ||
		o->type != n->type;
}


void XRefTable::DeleteRef(Ref ref) {
	XRefEntry* e = GetXRef(ref.num);
	ASSERT(e->type != xrefEntryFree && e->gen == ref.gen && e->gen != DEAD_GEN);	
	e->gen ++; e->type = xrefEntryFree;
	InsertInUnusedList(ref.num, e);
}

Ref XRefTable::AppendNewRef(XRefEntryType type) {
	ASSERT(type != xrefEntryFree);
	Ref ref;
	ref.num = mSize; ref.gen = 0;
	Resize(mSize + 1);
	XRefEntry* e = GetXRef(ref.num);
	e->offset = (Guint)-1; e->gen = ref.gen; e->type = type;
	return ref;
}

Ref XRefTable::GetNewRef(XRefEntryType type) {
	ASSERT(type != xrefEntryFree);
	Ref ref = ActivateUnusedEntry(type);
	if (!is_empty_ref(ref)) return ref;
	return AppendNewRef(type);
}

void XRefTable::SetOffset(Ref ref, int offset) {
	XRefEntry* e = GetXRef(ref.num);
	e->offset = offset;
}

bool XRefTable::NextGroup(int first, int* num, int* nof) {
	int n = GetSize();
	while (first < n && !HasChanged(first)) first ++;
	bool found = first < n;
	if (found) {
		*num = first;
		do {
			first ++;
		} while (first < n && HasChanged(first));
		*nof = first - *num;
	}
	return found;
}

// Implementation of AnnotTester
AnnotTester::AnnotTester()
	: AnnotVisitor()
{
}

AnnotTester::~AnnotTester()
{
}

// global function
bool CanWrite(Annotation* annot) {
	if (annot == NULL) return false;
	AnnotTester tester;
	annot->Visit(&tester);
	return tester.CanWrite();
}

// Implementation of AnnotWriter
AnnotWriter::AnnotWriter(XRef* xref, PDFDoc* doc, AnnotsList* list, AcroForm* acroForm)
	: mDoc(doc)
	, mAnnots(list) // make a copy
	, mAcroForm(acroForm)
	, mXRef(xref)
	, mXRefTable(xref)
	, mASRef(empty_ref)
	, mInfoRef(empty_ref)
{
}

AnnotWriter::~AnnotWriter()
{
}

void AnnotWriter::Write(const char* s) {
	fprintf(mFile, "%s", s);
}

void AnnotWriter::Write(GString* s) {
	fwrite(s->getCString(), s->getLength(), 1, mFile);
}

void AnnotWriter::Write(Ref r) {
	fprintf(mFile, "%d %d", r.num, r.gen);
}

void AnnotWriter::WriteCr() {
	Write("\r");
}

void AnnotWriter::WriteCrLf() {
	Write("\r\n");
}

int AnnotWriter::Tell() {
	return ftell(mFile);
}

// Convert xpdf Object to PDF output

void AnnotWriter::InsertWhiteSpace(Object* obj) {
	bool startsWithDelimiter;
	switch (obj->getType()) {
		case objString:
		case objArray:
		case objDict:
			startsWithDelimiter = true;
			break;
		default:
			startsWithDelimiter = false;
	}
	if (!startsWithDelimiter) Write(" ");
}


void AnnotWriter::WriteObject(Object* obj) {
	ASSERT(mFile != NULL);
	int i;
	Object o;
	GString* s = NULL;
			
	switch (obj->getType()) {
		 // simple objects
		case objBool: 
			fprintf(mFile, "%s", obj->getBool() ? "true" : "false");
			break;
		case objInt:
			fprintf(mFile, "%d", obj->getInt());
			break;
		case objReal:
			fprintf(mFile, "%g", obj->getReal());
			break;
		case objString:
			if (AnnotUtils::InUCS2(obj->getString())) {
				s = AnnotUtils::EscapeString(obj->getString());
				Write("(");
				Write(s);
				Write(")");
			} else {
				s = AnnotUtils::EscapeString(obj->getString());
				fprintf(mFile, "(%s)", s->getCString());
			}
			break;
		case objName:
			s = AnnotUtils::EscapeName(obj->getName());
			fprintf(mFile, "/%s", s->getCString());
			break;
		case objNull:
			Write("null");
			break;

		// complex objects
		case objArray:
			Write("[");
			for (i = 0; i < obj->arrayGetLength(); i ++) {
				obj->arrayGetNF(i, &o);
				if (i > 0) InsertWhiteSpace(&o);
				WriteObject(&o);
			}
			Write("]");
			break;
		case objDict:
			Write("<<");
			for (i = 0; i < obj->dictGetLength(); i ++) {
				if (i > 0) WriteCr();
				fprintf(mFile, "/%s", obj->dictGetKey(i));
				obj->dictGetValNF(i, &o);
				InsertWhiteSpace(&o);
				WriteObject(&o);
			}
			Write(">>");
			break;
		case objStream:
			fflush(mFile);
			fprintf(stderr, "Error: <!!!stream!!!>\n");
			ASSERT(false);
			break;
		case objRef:
			Write(obj->getRef()); Write(" R");
			break;
		default:
			fflush(mFile);
			fprintf(stderr, "Error: WriteObj unknown type %d\n", obj->getType());
			obj->print(stderr);
			fprintf(stderr, "\n");
			ASSERT(false);
	}
	delete s;
}

void AnnotWriter::WriteObject(Ref ref, Object* obj, GString* stream) {
	mXRefTable.SetOffset(ref, Tell());
	Write(ref); Write(" obj"); WriteCr();
	WriteObject(obj); WriteCr();
	if (stream != NULL) {
		Write("stream"); WriteCr();
		Write(stream);
		Write("endstream"); WriteCr();
	}
	Write("endobj"); WriteCr();
}

bool AnnotWriter::WriteXRefTable() {
	WriteCr();
	mXRefOffset = ftell(mFile);
	Write("xref\r");
	int first = 0;
	int nof;
	while (mXRefTable.NextGroup(first, &first, &nof)) {
		// write start num and count
		fprintf(mFile, "%d %d\r", first, nof);
		for (int i = 0; i < nof; i ++) {
			XRefEntry* x = mXRefTable.GetXRef(i + first);
			ASSERT(x->offset >= 0);
			// write offset, gen and used or unused char
			fprintf(mFile, "%10.10d %5.5d %c\r\n", 
				x->offset, x->gen, x->type != xrefEntryFree ? 'n' : 'f'); 
			}
		first += nof;
	}
	return true;
}


// Copy a dictionary excluding specified keys

bool AnnotWriter::IsInList(char* s, char* list[]) {
	for (int i = 0; list[i] != NULL; i ++) {
		if (strcmp(list[i], s) == 0) return true;
	}
	return false;
}

void AnnotWriter::CopyDict(Object* in, Object* out, char* excludeKeys[]) {
	ASSERT(in->isDict());
	out->initDict(mXRef);
	int n = in->dictGetLength();
	for (int i = 0; i < n; i ++) {
		char* key = in->dictGetKey(i);
		if (excludeKeys == NULL || !IsInList(key, excludeKeys)) {
			Object val;
			in->dictGetValNF(i, &val);
			out->dictAdd(copyString(key), &val);
		}
	}
}

// Update modification date

Ref AnnotWriter::GetModDateRef(Ref infoDictRef) {
	Ref dateRef = empty_ref;
	if (!is_empty_ref(infoDictRef)) {
		Object ref, dict;
		ref.initRef(infoDictRef.num, infoDictRef.gen);
		ref.fetch(mXRef, &dict);
		ref.free();
		if (dict.isDict()) HasRef(&dict, "ModDate", dateRef);
		dict.free();
	}
	return dateRef;
}

Ref AnnotWriter::GetInfoDictRef() {
	Ref ref;
	HasRef(mXRef->getTrailerDict(), "Info", ref);
	return ref;
}

static char* infoDictExcludeKeys[] = { "ModDate", NULL };

void AnnotWriter::CopyInfoDict(Object* dict) {
	ASSERT(!is_empty_ref(mInfoRef));
	Object info;
	mXRef->getTrailerDict()->dictLookup("Info", &info);
	CopyDict(&info, dict, infoDictExcludeKeys);
	info.free();
}

void AnnotWriter::WriteModDate(Ref ref) {
	GString* date = new GString();
	AnnotUtils::CurrentDate(date);

	Object obj;
	obj.initString(date);
	WriteObject(ref, &obj);
	obj.free(); // frees date
}

void AnnotWriter::UpdateInfoDict() {
	mInfoRef = GetInfoDictRef();
	Ref modDate = GetModDateRef(mInfoRef);
	if (is_empty_ref(modDate)) {
		modDate = mXRefTable.GetNewRef(xrefEntryUncompressed);
		Object info, val;
		if (is_empty_ref(mInfoRef)) {
			mInfoRef = mXRefTable.GetNewRef(xrefEntryUncompressed);
			info.initDict(mXRef);
		} else {
			CopyInfoDict(&info);
		}
		info.dictAdd(copyString("ModDate"), val.initRef(modDate.num, modDate.gen));
		WriteObject(mInfoRef, &info);
		info.free();
	}
	WriteModDate(modDate);
}

static char* fileTrailerExcludeKeys[] = { "Size", "Prev", "Root", "Info", NULL };

bool AnnotWriter::WriteFileTrailer() {
	Write("trailer\r");
	Object trailer;
	Object val;
	CopyDict(mXRef->getTrailerDict(), &trailer, fileTrailerExcludeKeys);
	trailer.dictAdd(copyString("Size"), val.initInt(mXRefTable.GetSize()));
	trailer.dictAdd(copyString("Prev"), val.initInt(mXRef->getLastXRefPos()));
	trailer.dictAdd(copyString("Root"), val.initRef(mXRef->getRootNum(), mXRef->getRootGen()));
	trailer.dictAdd(copyString("Info"), val.initRef(mInfoRef.num, mInfoRef.gen));
	WriteObject(&trailer);
	Write("\rstartxref\r");
	fprintf(mFile, "%d\r", mXRefOffset);
	Write("%%EOF\r");
	trailer.free();
	return true;
}

bool AnnotWriter::CopyFile(const char* name) {
	GString n(name);
	return mDoc->saveAs(&n);
}

bool AnnotWriter::HasRef(Object* dict, const char* key, Ref &ref) {
	Object obj;
	bool ok = true;
	ASSERT(dict && dict->isDict());
	if (dict->dictLookupNF((char*)key, &obj) && obj.isRef()) {
		ref = obj.getRef();
	} else {
		ref = empty_ref;
		ok = false;
	}
	obj.free();
	return ok;
}

bool AnnotWriter::HasAnnotRef(Object* page, Ref &annotRef) {
	return HasRef(page, "Annots", annotRef);
}

bool AnnotWriter::HasEmbeddedContent(Object* page) {
	ASSERT(page && page->isDict());
	Object obj;
	bool embedded = !(page->dictLookupNF("Contents", &obj) && 
		(obj.isArray() || obj.isRef() || obj.isNull()));
	obj.free();
	return embedded;
}

bool AnnotWriter::CopyContentStream(Object* page) {
	// not implemented yet!!!
	return false;
}

static char* pageDictExcludeKeys[] = { "Annots", NULL };

bool AnnotWriter::CopyPage(Object* page, Ref pageRef, Ref arrayRef) {
	Object copy;
	Object ar;
	
	ar.initRef(arrayRef.num, arrayRef.gen);	
	CopyDict(page, &copy, pageDictExcludeKeys);		
	copy.dictAdd(copyString("Annots"), &ar);

	// write to file
	WriteObject(pageRef, &copy);	
	copy.free();
	return true;
}

bool AnnotWriter::UpdatePage(int pageNo, Annotations* annots, Ref& annotArray) {
	bool ok = false;
	Object page;
	Ref* pageRef = mDoc->getCatalog()->getPageRef(pageNo+1);
	if (!mXRef->fetch(pageRef->num, pageRef->gen, &page)->isNull()) {
		if (HasAnnotRef(&page, annotArray)) return true;
		annotArray = mXRefTable.GetNewRef(xrefEntryUncompressed);
		if (HasEmbeddedContent(&page)) {
			if (!CopyContentStream(&page)) {
				fprintf(stderr, "Error: Could not copy content stream!");
				goto error;
			}
		}
		if (!CopyPage(&page, *pageRef, annotArray)) {
			fprintf(stderr, "Error: Could not copy page!");
		} else {
			ok = true;
		}
	} else {
		fprintf(stderr, "Error: Could not get page dict for page %d\n", pageNo+1);
	}
error:
	page.free();
	return ok;
}


// Annotation:
//   Deleted -> don't add it to Annots array
//   Changed
//     non_empty_ref -> add to Annots array
//     empty_ref -> add to Annots array if CanWrite
//   Otherwise -> add to Annots array

void AnnotWriter::AddToAnnots(Object* array, Annotation* a) {
	Ref r = a->GetRef();
	if (is_empty_ref(r)) {
		if (CanWrite(a)) {
			a->SetRef(mXRefTable.GetNewRef(xrefEntryUncompressed));
			r = a->GetRef();
		} else {
			return;
		}
	}
	Object ref;
	ref.initRef(r.num, r.gen);
	array->arrayAdd(&ref);
}

bool AnnotWriter::UpdateAnnotArray(int pageNo, Annotations* annots, Ref annotArray) {
	ASSERT(annots->HasChanged());
	Object array;
	array.initArray(mXRef);
	for (int i = 0; i < annots->Length(); i++) {
		Annotation* a = annots->At(i);
		if (!a->IsDeleted()) {
			AddToAnnots(&array, a);
			if (a->GetPopup()) AddToAnnots(&array, a->GetPopup());
		}
	}
	// write to file
	WriteObject(annotArray, &array);
	array.free();
	return true;
}

bool AnnotWriter::WriteAS(Ref& ref, Annotation* a) {
	if (is_empty_ref(ref)) return true;
	
	Object xobj;
	xobj.initDict(mXRef);
	// setup XObject dictionary
	AddName(&xobj, "Type", "XObject");
	AddName(&xobj, "Subtype", "Form");
	AddInteger(&xobj, "FormType", 1);
	PDFRectangle r = *a->GetRect();
	r.x2 -= r.x1; r.y2 -= r.y1;
	r.x1 = r.y1 = 0;
	AddRect(&xobj, "BBox", &r);
	// setup resource dictionary	
	Object resources, array, name;	
	resources.initDict(mXRef);
	array.initArray(mXRef);
	name.initName("PDF");
	array.arrayAdd(&name);
	resources.dictAdd(copyString("ProcSet"), &array);
	xobj.dictAdd(copyString("Resources"), &resources);
	
	// create appearance stream
	AnnotAppearance as;
	a->Visit(&as);
	
	// set length 
	AddInteger(&xobj, "Length", as.GetLength());
	ASSERT(as.GetLength() > 0);

	// write form XObject
	WriteObject(ref, &xobj, as.GetStream());
	xobj.free();
	ref = empty_ref;
	return true;
}


bool AnnotWriter::UpdateAnnot(Annotation* annot) {
	if (annot->HasChanged()) {
		Ref ref = annot->GetRef();
		ASSERT(!is_empty_ref(ref));
		mASRef = empty_ref;
		mAnnot.initDict(mXRef);
		AddName(&mAnnot, "Type", "Annot");
		annot->Visit(this);
		DoAnnotation(annot);
		WriteObject(ref, &mAnnot);
		mAnnot.free();
		WriteAS(mASRef, annot);
	}
	if (annot->GetPopup() != NULL) {
		return UpdateAnnot(annot->GetPopup());
	}
	return true;
}

// Create new PDF file and append changed or new annotations
bool AnnotWriter::WriteTo(const char* name) {
	if (!CopyFile(name)) return false;
	if (!mAnnots.HasChanged()) return true;
	AssignShortFontNames();
	mFile = fopen(name, "a+b");
	bool ok = mFile != NULL;
	int numPages = mDoc->getNumPages();
	for (int i = 0; ok && i < numPages; i ++) {
		mPageRef = *mDoc->getCatalog()->getPageRef(i+1);
		Annotations* a = mAnnots.Get(i);
		if (a && a->HasChanged()) {
			Ref annotArray;
			ok = ok && UpdatePage(i, a, annotArray);
			ok = ok && UpdateAnnotArray(i, a, annotArray);
			for (int j = 0; ok && j < a->Length(); j ++) {
				Annotation* an = a->At(j);
				if (!an->IsDeleted()) {
					if (CanWrite(an)) {
						ok = UpdateAnnot(an);
					}
				} else {
					if (!is_empty_ref(an->GetRef())) {
						mXRefTable.DeleteRef(an->GetRef());
					}
				}
			}
		}
	}
	if (ok) {
		UpdateInfoDict();
		UpdateAcroForm();
		UpdateCatalog();
		ok = WriteXRefTable();
		ok = ok && WriteFileTrailer();
	} 
	if (mFile) {
		fclose(mFile); mFile = NULL;
	}
	if (!ok) {
		// delete file on error
		unlink(name);
	}
	UnassignShortFontNames();
	return ok;
}


void AnnotWriter::AddRef(Object* dict, char* key, Ref ref) {
	ASSERT(dict->isDict());
	Object n;
	n.initRef(ref.num, ref.gen);
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddBool(Object* dict, char* key, bool b) {
	ASSERT(dict->isDict());
	Object n;
	n.initBool(b);
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddName(Object* dict, char* key, char* name) {
	ASSERT(dict->isDict());
	Object n;
	n.initName(name);
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddString(Object* dict, char* key, GString* string) {
	ASSERT(dict->isDict());
	Object n;
	n.initString(new GString(string));
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddString(Object* dict, char* key, char* string) {
	ASSERT(dict->isDict());
	Object n;
	n.initString(new GString(string));
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddInteger(Object* dict, char* key, int i) {
	ASSERT(dict->isDict());
	Object n;
	n.initInt(i);
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddReal(Object* dict, char* key, double r) {
	ASSERT(dict->isDict());
	Object n;
	n.initReal(r);
	dict->dictAdd(copyString(key), &n);
}


void AnnotWriter::AddReal(Object* array, double r) {
	ASSERT(array->isArray());
	Object n;
	n.initReal(r);
	array->arrayAdd(&n);
}


void AnnotWriter::AddRect(Object* dict, char* key, PDFRectangle* rect) {
	ASSERT(dict->isDict());
	Object a;
	a.initArray(mXRef);
	AddReal(&a, rect->x1);
	AddReal(&a, rect->y1);
	AddReal(&a, rect->x2);
	AddReal(&a, rect->y2);
	dict->dictAdd(copyString(key), &a);
}


void AnnotWriter::AddColor(Object* dict, char* key, GfxRGB* c) {
	Object a;
	a.initArray(mXRef);
	AddReal(&a, colToDbl(c->r));
	AddReal(&a, colToDbl(c->g));
	AddReal(&a, colToDbl(c->b));
	dict->dictAdd(copyString(key), &a);
}


void AnnotWriter::AddDict(Object* dict, char* key, Object* d) {
	ASSERT(dict->isDict());
	dict->dictAdd(copyString(key), d);
}


void AnnotWriter::AddAnnotSubtype(char* type) {
	AddName(&mAnnot, "Subtype", type);
}

void AnnotWriter::AddAnnotContents(Annotation* a) {
	AddString(&mAnnot, "Contents", a->GetContents());
}

bool AnnotWriter::HasAppearanceStream(Annotation* a) {
	AnnotAppearance ap;
	a->Visit(&ap);
	return ap.GetLength() > 0;
}

void AnnotWriter::DoAnnotation(Annotation* a) {
	AddRect(&mAnnot, "Rect", a->GetRect());
	if (a->HasColor()) {
		AddColor(&mAnnot, "C", a->GetColor());
	}
	if (a->GetDate()[0] != 0) { 
		AddString(&mAnnot, "M", (char*)a->GetDate());
	}
	AddInteger(&mAnnot, "F", a->GetFlags()->Flags());
	if (a->GetTitle() != NULL) {
		AddString(&mAnnot, "T", a->GetTitle());
	}
	if (a->GetOpacity() != 1.0) {
		AddReal(&mAnnot, "CA", a->GetOpacity());
	}
	PopupAnnot* popup = a->GetPopup();
	if (popup != NULL) {
		popup->SetParentRef(a->GetRef());
		if (is_empty_ref(popup->GetRef())) {
			popup->SetRef(mXRefTable.GetNewRef(xrefEntryUncompressed));
		}
		AddRef(&mAnnot, "Popup", popup->GetRef());
	}
	if (HasAppearanceStream(a)) {
		mASRef = mXRefTable.GetNewRef(xrefEntryUncompressed);
		Object ap;
		ap.initDict(mXRef);
		AddRef(&ap, "N", mASRef);
		AddDict(&mAnnot, "AP", &ap);
	}
	if (dynamic_cast<PopupAnnot*>(a) == NULL) {
		AddRef(&mAnnot, "P", mPageRef);
	}
}

void AnnotWriter::DoStyledAnnot(StyledAnnot* s) {
	AddAnnotContents(s);
	// border style
	char* style = NULL;
	Object bs;
	bs.initDict(mXRef);
	AddName(&bs, "Type", "Border");
	AddInteger(&bs, "W", (float)s->GetBorderStyle()->GetWidth()); // width
	switch (s->GetBorderStyle()->GetStyle()) {
		case BorderStyle::solid_style: style = "S";
		case BorderStyle::dashed_style: style = "D";
		case BorderStyle::beveled_style: style = "B";
		case BorderStyle::inset_style: style = "I";
		case BorderStyle::underline_style: style = "U";
	}
	if (style != NULL) {
		AddName(&bs, "S", style); // border style	
	}
	AddDict(&mAnnot, "BS", &bs);
}

void AnnotWriter::DoMarkupAnnot(MarkupAnnot* m) {
	DoStyledAnnot(m);
	Object array;
	array.initArray(mXRef);
	for (int i = 0; i < m->QuadPointsLength(); i++) {
		PDFQuadPoints* q = m->QuadPointsAt(i);
		for (int j = 0; j < 4; j ++) {
			PDFPoint p = (*q)[j];
			Object val;
			array.arrayAdd(val.initReal(p.x));
			array.arrayAdd(val.initReal(p.y));
		}
	}
	mAnnot.dictAdd(copyString("QuadPoints"), &array);
}

// Annotation visitor implementation
void AnnotWriter::DoText(TextAnnot* a){
	AddAnnotSubtype("Text");
	AddAnnotContents(a);
	AddName(&mAnnot, "Name", (char*)a->GetName());
}


void AnnotWriter::DoLink(LinkAnnot* a){
	AddAnnotSubtype("Link");
}


void AnnotWriter::DoFreeText(FreeTextAnnot* a){
	GString appearance;
	char buf[250];
	PDFFont* font;
	GfxRGB* color;
	// build appearance string
	font = a->GetFont();
	color = a->GetFontColor();
	appearance.clear();
	// color
	sprintf(buf, "[%g %g %g] rg ", colToDbl(color->r), colToDbl(color->g), colToDbl(color->b)); 
	appearance.append(buf);
	
	// font and size
	sprintf(buf, "/%s %g Tf", font->GetShortName(), a->GetFontSize());
	appearance.append(buf);
	a->SetAppearance(&appearance);
	
	DoStyledAnnot(a);
	AddAnnotSubtype("FreeText");
	AddAnnotContents(a);
	AddString(&mAnnot, "DA", a->GetAppearance());
	if (a->GetJustification() != left_justify) {
		AddInteger(&mAnnot, "Q", a->GetJustification());
	}
	
	WriteFont(a->GetFont());
}


void AnnotWriter::DoLine(LineAnnot* a){
	AddAnnotSubtype("Line");
	DoStyledAnnot(a);
	Object array;
	array.initArray(mXRef);
	Object val;
	PDFPoint* line = a->GetLine();
	array.arrayAdd(val.initReal(line[0].x));
	array.arrayAdd(val.initReal(line[0].y));
	array.arrayAdd(val.initReal(line[1].x));
	array.arrayAdd(val.initReal(line[1].y));
	mAnnot.dictAdd(copyString("L"), &array);
}


void AnnotWriter::DoSquare(SquareAnnot* a){
	AddAnnotSubtype("Square");
	DoStyledAnnot(a);
}


void AnnotWriter::DoCircle(CircleAnnot* a){
	AddAnnotSubtype("Circle");
	DoStyledAnnot(a);
}


void AnnotWriter::DoHighlight(HighlightAnnot* a){
	AddAnnotSubtype("Highlight");
	DoMarkupAnnot(a);
}


void AnnotWriter::DoUnderline(UnderlineAnnot * a){
	AddAnnotSubtype("Underline");
	DoMarkupAnnot(a);
}


void AnnotWriter::DoSquiggly(SquigglyAnnot* a){
	AddAnnotSubtype("Squiggly");
	DoMarkupAnnot(a);
}


void AnnotWriter::DoStrikeOut(StrikeOutAnnot* a){
	AddAnnotSubtype("StrikeOut");
	DoMarkupAnnot(a);
}

void AnnotWriter::DoStamp(StampAnnot* a){
	AddAnnotSubtype("Stamp");
}


void AnnotWriter::DoInk(InkAnnot* a){
	AddAnnotSubtype("Ink");
}


void AnnotWriter::DoPopup(PopupAnnot* a){
	AddAnnotSubtype("Popup");
	if (!is_empty_ref(a->GetParentRef())) {
		AddRef(&mAnnot, "Parent", a->GetParentRef());
	}
}


void AnnotWriter::DoFileAttachment(FileAttachmentAnnot* a){
	AddAnnotSubtype("FileAttachment");
}


void AnnotWriter::DoSound(SoundAnnot* a){
	AddAnnotSubtype("Sound");
}


void AnnotWriter::DoMovie(MovieAnnot* a){
	AddAnnotSubtype("Movie");
}


void AnnotWriter::DoWidget(WidgetAnnot* a){
	AddAnnotSubtype("Widget");
}


void AnnotWriter::DoPrinterMark(PrinterMarkAnnot* a){
	AddAnnotSubtype("PrinterMark");
}


void AnnotWriter::DoTrapNet(TrapNetAnnot* a){
	AddAnnotSubtype("TrapNet");
}

// FreeTextAnnot
void AnnotWriter::AssignShortFontNames() {
	// scan all fonts
	std::list<int> fontIDs;
	{
		std::list<PDFFont*>* fonts = mAcroForm->GetFonts();
		std::list<PDFFont*>::iterator it;
		for (it = fonts->begin(); it != fonts->end(); it ++) {
			int d;
			PDFFont* font = *it;
			if (sscanf(font->GetShortName(), "F%d", &d) != 0 && d >= 0) {
				fontIDs.push_back(d);
			}
		}
	}
	
	// assign short names to standard fonts
	PDFStandardFonts* stdFonts = AcroForm::GetStandardFonts();
	int id = 0;
	std::list<int>::iterator it;
	fontIDs.sort();
	it = fontIDs.begin();
	for (int i = 0; i < stdFonts->CountFonts(); i ++) {
		PDFFont* font = stdFonts->FontAt(i);
		if (strcmp(font->GetShortName(), "") == 0) {
			GString shortName("F");
			char number[80];
			while (it != fontIDs.end() && id == *it) {
				id ++; it ++;
			}
			sprintf(number, "%d", id);
			shortName.append(number);
			font->SetShortName(shortName.getCString());
			mTemporaryFonts.push_back(font);	
			id ++;
		}
	}
}

void AnnotWriter::UnassignShortFontNames() {
	// reverse steps to have proper state in case file is saved again 
	std::list<PDFFont*>::iterator it;
	for (it = mTemporaryFonts.begin(); it != mTemporaryFonts.end(); it ++) {
		PDFFont* font = *it;
		font->SetRef(empty_ref);
		font->SetShortName("");
	}
}

void AnnotWriter::WriteFont(PDFFont* font) {
	if (!is_empty_ref(font->GetRef())) return; // already saved
	font->SetRef(mXRefTable.GetNewRef(xrefEntryUncompressed));
	mWrittenFonts.push_back(font);
	Object dict;
	dict.initDict(mXRef);
	AddName(&dict, "Type", "Font");
	AddName(&dict, "Subtype", "Type1");
	AddName(&dict, "BaseFont", (char*)font->GetName());
	AddName(&dict, "Encoding", "WinAnsiEncoding");
	WriteObject(font->GetRef(), &dict);
	dict.free();
}

void AnnotWriter::AddFonts(Object* dict, std::list<PDFFont*>* fonts) {
	std::list<PDFFont*>::iterator it;
	for (it = fonts->begin(); it != fonts->end(); it ++) {
		PDFFont* font = *it;
		ASSERT(!is_empty_ref(font->GetRef()));
		AddRef(dict, (char*)font->GetShortName(), font->GetRef());
	}
}

static char* acroFormExcludeKeys[] = {
	"DR", NULL
};

static char* drExcludeKeys[] = {
	"Font", NULL
};

void AnnotWriter::UpdateAcroForm() {
	if (mWrittenFonts.empty()) return;
	
	Object acroForm;
	Object oldDR;
	
	acroForm.initDict(mXRef);
	oldDR.initNull();
	
	if (is_empty_ref(mAcroForm->GetRef())) {
		Ref fieldsRef = mXRefTable.GetNewRef(xrefEntryUncompressed);
		// create empty array for fields
		Object fields;
		fields.initArray(mXRef);
		WriteObject(fieldsRef, &fields);
		fields.free();
		
		// create new AcroForm
		mAcroFormRef = mXRefTable.GetNewRef(xrefEntryUncompressed);
		AddName(&acroForm, "Type", "AcroForm");
		AddRef(&acroForm, "Fields", fieldsRef);
	} else {
		// copy existing AcroForm except DR
		mAcroFormRef = mAcroForm->GetRef();
		Object ref;
		Object oldForm;
		ref.initRef(mAcroFormRef.num, mAcroFormRef.gen);
		ref.fetch(mXRef, &oldForm);
		CopyDict(&oldForm, &acroForm, acroFormExcludeKeys);
		oldForm.dictLookup("DR", &oldDR);
		oldForm.free();
		ref.free();
	}
	// Add DR to AcroForm
	Object dr;
	dr.initDict(mXRef);
	if (oldDR.isDict()) {
		CopyDict(&oldDR, &dr, drExcludeKeys);
		oldDR.free();
	}
	// Add font dict
	Object font;
	font.initDict(mXRef);
	// add old fonts
	AddFonts(&font, mAcroForm->GetFonts());
	// add new fonts
	AddFonts(&font, &mWrittenFonts);
	AddDict(&dr, "Font", &font);
	AddDict(&acroForm, "DR", &dr);
	WriteObject(mAcroFormRef, &acroForm);
	acroForm.free();
}

void AnnotWriter::UpdateCatalog() {
	// Return if AcroForm has not been written or ref exists already in Catalog
	if (is_empty_ref(mAcroFormRef) || !is_empty_ref(mAcroForm->GetRef())) return;
	// Copy catalog and add ref to new AcroForm
	Ref root;
	Object oldCatalogRef;
	Object oldCatalog;
	Object catalog;
	root.num = mXRef->getRootNum();
	root.gen = mXRef->getRootGen();
	oldCatalogRef.initRef(root.num, root.gen);
	oldCatalogRef.fetch(mXRef, &oldCatalog);
	catalog.initDict(mXRef);	
	CopyDict(&oldCatalog, &catalog);
	AddRef(&catalog, "AcroForm", mAcroFormRef);
	WriteObject(root, &catalog);
	catalog.free();
	oldCatalog.free();
	oldCatalogRef.free();
}
