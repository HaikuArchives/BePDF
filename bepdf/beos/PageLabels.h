/*  
	Copyright (C) 2000 Michael Pfeiffer
	
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
#ifndef PAGE_LABELS_H
#define PAGE_LABELS_H

// BeOS
#include <be/support/String.h>
#include <be/support/List.h>
#include <be/interface/ListView.h>

#include "TreeParser.h"

/* PageLabels */
class PageLabel {
	BString prefix;
	enum {
		DECIMAL,
		UPPER_CASE_ROMAN,
		LOWER_CASE_ROMAN,
		UPPER_CASE_ALPHA,
		LOWER_CASE_ALPHA
	} style;
	int page, start;

	static const char *d[3][10];
public:
	static BString &ToAlpha(BString &s, int n);
	static BString &ToRoman(BString &s, int n);
	
	int GetPage() { return page; };
	PageLabel(int page, Dict *leaf);
	const char *GetLabel(int page, int max, BString &label);
};

class PageLabels : public NumberTreeParser {
	int numberOfPages;
	PageLabel *old;
	BList list;
public:
	PageLabels(int numberOfPages);
	~PageLabels();

	bool Parse(Object* pageLabels);
	bool DoNumber(int page, Object* object);
	virtual bool NewPageLabel(PageLabel *label, int max);

	void Replace(BListView *view);
};

#endif