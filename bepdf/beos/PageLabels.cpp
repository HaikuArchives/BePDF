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

#include "PageLabels.h"
#include "LayoutUtils.h"
#include "TextConversion.h"

const char *PageLabel::d[3][10] = {
	{ "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" },
	{ "", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC" },
	{ "", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM" }
};

PageLabel::PageLabel(int page, Dict *leaf) {
	Object obj;
	this->page = page;
	// Style 
	if (leaf->lookup("S", &obj) && obj.isName()) {
		switch (*obj.getName()) {
		case 'D': style = DECIMAL;
			break;
		case 'R': style = UPPER_CASE_ROMAN;
			break;
		case 'r': style = LOWER_CASE_ROMAN;
			break;
		case 'A': style = UPPER_CASE_ALPHA;
			break;
		case 'a': style = LOWER_CASE_ALPHA;
			break;
		default:
			style = DECIMAL;
		}
	} else {
		style = DECIMAL;
	}
	obj.free();
	
	// Prefix
	if (leaf->lookup("P", &obj) && obj.isString()) {
		BString *s = TextToUtf8(obj.getString()->getCString(), obj.getString()->getLength());
		prefix = *s;
		delete s;
	}
	obj.free();
	
	// StartAt
	if (leaf->lookup("St", &obj) && obj.isInt()) {
		start = obj.getInt();
		if (start < 1) {
			start = 1;
		}
	} else {
		start = 1;
	}
	obj.free();
//	fprintf(stderr, "%d /P %s /St %d\n", page, prefix.String(), start);
}

BString &PageLabel::ToAlpha(BString &str, int num) {
	// num >= 1
	num--;
	int digits = (num+26) / 26;
	
	char *s = str.LockBuffer(digits+1);
	char c  = 'A' + (num % 26);
	for (int i = digits; i; i--, s++) *s = c;
	*s = '\0';
	str.UnlockBuffer(digits+1);
	return str;
}
	
BString &PageLabel::ToRoman(BString &s, int n) {
	int d0, d1, d2;
	d0 = n % 10;
	n /= 10;
	d1 = n % 10;
	n /= 10;
	d2 = n % 10;

	for (int i = n / 10; i > 0; i--) {
		s << "M";
	}
	s << d[2][d2] << d[1][d1] << d[0][d0];
	return s; 
}

const char *PageLabel::GetLabel(int num, int max, BString &label) {
	if ((page <= num) && (num <= max)) {
		BString s;
		label = prefix;
		num = num - page + start;
		switch (style) {
		case DECIMAL: label << num;
			break;
		case UPPER_CASE_ROMAN: label << ToRoman(s, num);
			break;
		case LOWER_CASE_ROMAN: ToRoman(s, num).ToLower(); label << s;
			break;
		case UPPER_CASE_ALPHA: label << ToAlpha(s, num);
			break;
		case LOWER_CASE_ALPHA: ToAlpha(s, num).ToLower(); label << s;
			break;
		}
		return label.String();
	} else {
		return NULL;
	}
}

/* PageLabels */
#include <be/interface/ListItem.h>

PageLabels::PageLabels(int numberOfPages)
	: numberOfPages(numberOfPages)
	, old(NULL) {	
}

PageLabels::~PageLabels() {
	delete old; 
	old = NULL;

	BListItem **items = (BListItem**)list.Items();
	for (int i = list.CountItems()-1; i >= 0; i--) {
		delete items[i];
	}
	list.MakeEmpty();
}

bool PageLabels::Parse(Object* pageLabels) {
	if (NumberTreeParser::Parse(pageLabels)) {
		return NewPageLabel(NULL, numberOfPages);
	}
	return false;
}

bool PageLabels::DoNumber(int page, Object* value) {
	if (!value->isDict()) {
		return false;
	}
	PageLabel *label = new PageLabel(page, value->getDict());
	return NewPageLabel(label, page-1);
}

bool PageLabels::NewPageLabel(PageLabel *label, int max) {
	if (old) {
		BString s;
		for (int i = old->GetPage(); i <= max; i++) {
			list.AddItem(new BStringItem(old->GetLabel(i, max, s)));
		}
		delete old;
	}
	old = label;
	return true;
}

void PageLabels::Replace(BListView *view) {
	MakeEmpty(view);
	view->AddList(&list);
	list.MakeEmpty();
}
