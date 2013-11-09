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



#include <be/storage/Entry.h>
#include <be/storage/File.h>
#include "LocaleCatalog.h"

#ifndef ASSERT
#define ASSERT(a)
#endif

#include "OpenHashTable.h"

#include <stdio.h>
#include <String.h>

// IDTransElement

class IDTransElement
{
	public:
		IDTransElement();
		IDTransElement(const BString &id, const BString &translation);
		BString	id;				// id string
		BString	translation;	// translated string

		inline uint32 Hash() const { return Hash(id.String()); }
//		inline bool operator==(const IDTransElement &element) const
//		{
//			return (id == element.id && translation == element.translation);
//		}
		static uint32 Hash(const char *string, uint32 seed = 0);
};

// constructor
IDTransElement::IDTransElement()
{
}

// constructor
IDTransElement::IDTransElement(const BString &cid, const BString &ctranslation)
			  : id(cid),
				translation(ctranslation)
{
}

// Hash
uint32 IDTransElement::Hash(const char *string, uint32 seed)
{
	char ch;
	uint32 result = seed;
	
	while((ch = *string++) != 0) {
		result = (result << 7) ^ (result >> 24);
		result ^= ch;
	}

	result ^= result << 12;
	return result;
}

// IDTransElementHandle
class IDTransElementHandle
{
	public:
		IDTransElementHandle();
		IDTransElementHandle(IDTransElement *element);
		void SetElement(IDTransElement *element);

		inline uint32 Hash() const { return element->Hash(); }
		inline bool operator==(const IDTransElementHandle &handle) const
		{
			return (element == handle.element);	// compare the pointers
		}
		inline void Adopt(IDTransElementHandle &handle)
		{
			element = handle.element;
		}

		inline IDTransElement *Element() { return element; }

		IDTransElement	*element;
		int32 fNext;
};

// constructor
IDTransElementHandle::IDTransElementHandle()
					: element(0),
					  fNext(-1)
{
}

// constructor
IDTransElementHandle::IDTransElementHandle(IDTransElement *celement)
					: element(celement),
					  fNext(-1)
{
}

// SetElement
void IDTransElementHandle::SetElement(IDTransElement *celement)
{
	element = celement;
}


// IDTransElementVector

class IDTransElementHandleVector
	: public OpenHashElementArray<IDTransElementHandle>
{
	public:
		~IDTransElementHandleVector();
		IDTransElementHandleVector(int32 initialSize);
		IDTransElementHandle *Add();
};

// constructor
IDTransElementHandleVector::IDTransElementHandleVector(int32 initialSize)
	:	OpenHashElementArray<IDTransElementHandle>(initialSize)
{
}

// destructor
IDTransElementHandleVector::~IDTransElementHandleVector()
{
}


// Add
IDTransElementHandle *IDTransElementHandleVector::Add()
{
	return &At(OpenHashElementArray<IDTransElementHandle>::Add());
}


// IDTransTable

class IDTransTable : public OpenHashTable<IDTransElementHandle,
										  IDTransElementHandleVector>
{
	public:
		IDTransTable();
		~IDTransTable();
		IDTransElement *FindElement(const BString &key);
		void AddElement(IDTransElement *element);
};

// constructor
IDTransTable::IDTransTable()
			: OpenHashTable<IDTransElementHandle,
							IDTransElementHandleVector>(10,
					new IDTransElementHandleVector(10))
{
}

// destructor
IDTransTable::~IDTransTable()
{
	// delete the elements
	for (int32 i = 0; i < fArraySize; i++)
	{
		int32 index = fHashArray[i];
		while (index >= 0)
		{
			IDTransElementHandle *handle = ElementAt(index);
			index = handle->fNext;
			delete handle->Element();
		}
	}
	delete fElementVector;
}

// FindElement
IDTransElement *IDTransTable::FindElement(const BString &key)
{
	IDTransElementHandle *handle =
		FindFirst(IDTransElement::Hash(key.String()));
	while (handle && handle->Element())
	{
		if (handle->Element()->id == key)
			return handle->Element();
		if (handle->fNext >= 0)
			handle = ElementAt(handle->fNext);
		else
			handle = 0;
	}
	return 0;
}

// AddElement
void IDTransTable::AddElement(IDTransElement *element)
{
	Add(element->Hash()).SetElement(element);
}


static int32 find_delimiter(BString &str, int32 pos)
{
	for (;;)
	{
		pos = str.FindFirst('\n', pos);
		if (pos == B_ERROR)
			return B_ERROR;
		// check if it is not quoted (even number of '\'s)
		int32 count = 0;
		while (pos - count - 1 >= 0 && str[pos - count - 1] == '\\')
			count++;
		if (!(count & 1))
			return pos;
		pos++;
	}
}

//----------------------------------------------------------//

BCatalog::BCatalog(const BString *name, uint32 priority) {
}

BCatalog::~BCatalog() {
}

//----------------------------------------------------------//

// BTextFileCatalog

// constructor
BTextFileCatalog::BTextFileCatalog(const BString *name, uint32 priority)	//catalogFile
		: BCatalog(name, priority)
		, vInitState(B_ERROR)
		, vTransTable(new IDTransTable())
{
	// get a file object
	BFile file(name->String(), B_READ_ONLY);
	vInitState = file.InitCheck();
	if (vInitState != B_OK)
		return;

	// read in hash table
	// we read in the whole file
	off_t size;
	vInitState = file.GetSize(&size);
	if (vInitState != B_OK)
		return;

	BString contents;
	if (char *buffer = contents.LockBuffer(size))
	{
		if (file.Read(buffer, size) != (ssize_t)size)
			vInitState = B_ERROR;
		contents.UnlockBuffer(size);
		if (vInitState == B_OK)
		{
			int32 start = 0;
			// read the lines: two per hash table element
			while (vInitState == B_OK && start < size)
			{
				int32 second = find_delimiter(contents, start);
				if (second != B_ERROR)
				{
					int32 end = find_delimiter(contents, second + 1);
					if (end != B_ERROR)
					{
						// id: from start to inclusively second - 1
						BString id(contents.String() + start, second - start);
						// translation: from second + 1 to incl. end - 1
						BString translation(contents.String() + second + 1,
											end - second - 1);
						id.CharacterDeescape('\\');
						translation.CharacterDeescape('\\');
						if (id.Length() > 0 && translation.Length())
						{
							vTransTable->AddElement(new IDTransElement(id,
														translation));
						}
						start = end + 1;
					}
					else
						vInitState = B_ERROR;
				}
				else
					vInitState = B_ERROR;
			}
		}
	}
	else
		vInitState = B_ERROR;

	// if everything went ok set vInitState to ok
	vInitState = B_OK;
}

// destructor
BTextFileCatalog::~BTextFileCatalog()
{
	delete vTransTable;
}

// InitCheck
status_t BTextFileCatalog::InitCheck() const
{
	return vInitState;
}

// GetString
status_t
BTextFileCatalog::GetString(const BString *id, const BString **translation) const
{
	status_t result = B_ERROR;
	if (IDTransElement *element = vTransTable->FindElement(*id))
	{
		*translation = &element->translation;
		result = B_OK;
	}
	return result;
}

#if 0
int main()
{
	entry_ref ref;
	if (get_ref_for_path("test.catalog", &ref))
	{
		BString name("test.catalog");
		BTextFileCatalog catalog(&name, 0);
		if (catalog.InitCheck() == B_OK)
		{
//	IDTransTable table;
//	table.AddElement(new IDTransElement("key1", "value1"));
//	table.AddElement(new IDTransElement("key2", "value2"));
//	table.AddElement(new IDTransElement("key3", "value3"));
//	table.AddElement(new IDTransElement("key4", "value4"));
//	table.AddElement(new IDTransElement("key5", "value5"));
/*			if (IDTransElement *element = table.FindElement("key2"))
				printf("%s\n", element->translation.String());
			if (IDTransElement *element = table.FindElement("key1"))
				printf("%s\n", element->translation.String());
			if (IDTransElement *element = table.FindElement("key5"))
				printf("%s\n", element->translation.String());
			if (IDTransElement *element = table.FindElement("key7"))
				printf("%s\n", element->translation.String());
			if (IDTransElement *element = table.FindElement("key2"))
				printf("%s\n", element->translation.String());
*/
			const BString *translation;
			BString keystr("key1");
			if (catalog.GetString(&keystr, &translation) == B_OK)
			{
				printf("%s\n", translation->String());
			}
		}
	}
	return 0;
}
#endif


