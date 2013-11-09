/*  
	Copyright (C) 2005 Michael Pfeiffer
	
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
#ifndef TREE_PARSER_H
#define TREE_PARSER_H

// xpdf
#include <XRef.h>
#include <PDFDoc.h>
#include <Object.h>

class TreeParser {
	bool ParseEntries(Array *nums);
	bool ParseKids(Array *kids);
	
	GString entryKey;
	char* GetEntryKey() { return entryKey.getCString(); };
	
public:
	TreeParser(const char* entryKey) { this->entryKey.append(entryKey); };
	virtual ~TreeParser() {};
	
	bool Parse(Object *tree);
	virtual bool DoEntry(Object* key, Object* value) = 0;
};

class NameTreeParser : public TreeParser {
public:
	NameTreeParser() : TreeParser("Names") {};
	bool DoEntry(Object* key, Object* value);
	virtual bool DoName(const char *name, Object* value) = 0;
};

class NumberTreeParser : public TreeParser {
public:
	NumberTreeParser() : TreeParser("Nums") {};
	bool DoEntry(Object* key, Object* value);
	virtual bool DoNumber(int number, Object* value) = 0;
};

#endif
