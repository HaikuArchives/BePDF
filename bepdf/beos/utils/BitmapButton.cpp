#include "BitmapButton.h"
#include "ResourceLoader.h"

//////////////////////////////////////////////////////////
// Resource Bitmap Button
ResourceBitmapButton::ResourceBitmapButton(BRect frame, const char *name,
		const char *off, const char *on,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BButton(frame, name, "", message, resizingMode, flags)
{
	SetIcon(LoadBitmap(on));
}

ResourceBitmapButton::ResourceBitmapButton(BRect frame, const char *name,
		const char *off, const char *on,
		const char *disabledOff, const char *disabledOn,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BButton(frame, name, "", message, resizingMode, flags)
{
	SetIcon(LoadBitmap(on));
}

ResourceBitmapButton::ResourceBitmapButton(BRect frame, const char *name,
		int32 offId, int32 onId,
		int32 disabledOffId, int32 disabledOnId,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BButton(frame, name, "", message, resizingMode, flags)
{
	SetIcon(LoadBitmap(onId));
}
