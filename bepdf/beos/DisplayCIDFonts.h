#ifndef _DISPLAY_CID_FONTS_H
#define _DISPLAY_CID_FONTS_H

#include <List.h>
#include <Message.h>

class DisplayCIDFont;

class DisplayCIDFonts {
public:
	DisplayCIDFonts(const BMessage& fonts);
	virtual ~DisplayCIDFonts();
	
	status_t Archive(BMessage& archive);

	enum Type {
		kType1,
		kTrueType,
		kUnknownType
	};
	
	bool Contains(const char* name) const;
	int32 GetSize() const;
	void Get(int32 index, BString& name, BString& file, Type& type) const;
	void Get(const char* name, BString& file, Type& type) const;
	void Set(const char* name, const char* file = "", Type type = kUnknownType);
	
private:
	void Get(DisplayCIDFont* displayName, BString& name, BString& file, Type& type) const;
	DisplayCIDFont* Get(int32 index) const;
	DisplayCIDFont* Find(const char* name) const;

	BList mFonts;
};

#endif
