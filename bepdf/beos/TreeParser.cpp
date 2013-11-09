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

#include "TreeParser.h"

#include "TextConversion.h"

/* TreeParser */
bool TreeParser::ParseEntries(Array *entries) {
	int len = entries->getLength();	
	bool ok = true;
	for (int i = 0; ok && i < len; i++) {
		Object key;
		if (entries->get(i, &key)) {
			i ++;
			Object value;
			if (i < len && entries->get(i, &value)) {
				ok = DoEntry(&key, &value);
			} else {
				// missing value
				ok = false;
			}
			value.free();
		}
		key.free();
	}
	return ok;
}

bool TreeParser::ParseKids(Array *kids) {
	int len = kids->getLength();
	Object kid;
	Object sub;
	bool ok = true;
	for (int i = 0; ok && i < len; i++) {
		if (kids->get(i, &kid) && kid.isDict()) {
			if (kid.dictLookup("Kids", &sub) && sub.isArray()) {
				ok = ParseKids(sub.getArray());
			}
			sub.free();
			
			if (ok && kid.dictLookup(GetEntryKey(), &sub) && sub.isArray()) {
				ok = ParseEntries(sub.getArray());
			}
			sub.free();
		}
		kid.free();
	}
	return ok;
}

bool TreeParser::Parse(Object *tree) {
	if (tree->isDict()) {
		Object o; 
		// nodes or leafs
		if (tree->dictLookup("Kids", &o) && o.isArray()) {
			bool ok = ParseKids(o.getArray());
			o.free();
			return ok;
		} else { 
			o.free();
		
			// leafs
			if (tree->dictLookup(GetEntryKey(), &o) && o.isArray()) {
				bool ok = ParseEntries(o.getArray());
				o.free();
				return ok;
			}
			o.free();
		}
	} 
	return false;
}

/* NameTreeParser */
bool NameTreeParser::DoEntry(Object* key, Object* value) {
	if (key->isString() && key->getString() != NULL) {
		GString* string = key->getString();
		BString* utf8 = TextToUtf8(string->getCString(), string->getLength());
		bool ok = utf8 != NULL;
		if (ok) {
			ok = DoName(utf8->String(), value);
		}
		delete utf8;
		return ok;
	} 
	// not in PDF spec. but does not hurt either
	if (key->isName()) {
		return DoName(key->getName(), value);
	}
	return false;
}

/* NumberTreeParser */
bool NumberTreeParser::DoEntry(Object* key, Object* value) {
	if (key->isInt()) {
		return DoNumber(key->getInt(), value);
	}
	return false;
}
