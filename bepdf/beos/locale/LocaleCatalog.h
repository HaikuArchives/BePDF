/***********************************************************************
 * 48hrs - Coding Event - BeGeistert, Düsseldorf April, 15.-16.2000
 *
 * Locale Kit
 *		Shared-Library		(liblocale.so)
 *		Preference Panel	(Locale)
 *
 * Copyright © 2000, 48hrs participants & contributors, All rights
 * reserved.
 *
 ***********************************************************************
 *
 * Date		04/18/2000
 * Revision	0.1
 *
 ***********************************************************************
 * 48hrs participants:
 * 		Lukas Hartmann, Marcus Jacob, Cedric Neve,
 *		Hans Speijer, Jens Tinz, Ingo Weinhold
 ***********************************************************************
 *
 * 48HRS PUBLIC LICENSE
 * Version 1, April 2000
 * 
 * Copyright © 2000, 48hrs participants & contributors, All rights
 * reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice applies to all
 * licensees and shall be included in all copies or substantial
 * portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE 48HRS PARTICIPANTS &
 * CONTRIBUTORS (THE "DEVELOPER") BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **********************************************************************/
 
#ifndef _CATALOG_H
#define _CATALOG_H

// #include "LocaleBuild.h"

#define IMPEXP_LOCALE

class BString;
class IDTransTable;

class IMPEXP_LOCALE BCatalog;
class BCatalog {			// abstract base class
 public:
							// returns B_OK if the appropriate translation file was found
	virtual	status_t		InitCheck() const= 0;

							// translation is deleted by BCatalog
	virtual	status_t		GetString(const BString *id, const BString **translation) const= 0;
 protected:
							BCatalog(const BString *name, uint32 priority);
	virtual					~BCatalog();		
};

class IMPEXP_LOCALE BTextFileCatalog;
class BTextFileCatalog : public BCatalog {
 public:
							BTextFileCatalog(const BString *name, uint32 priority);
	virtual					~BTextFileCatalog();		

							// returns B_OK if the translation data was found
	virtual	status_t		InitCheck() const;

							// translation is deleted by BCatalog
	virtual	status_t		GetString(const BString *id, const BString **translation) const;

 private:
			status_t		vInitState;
			IDTransTable	*vTransTable;
};

#endif // _CATALOG_H
