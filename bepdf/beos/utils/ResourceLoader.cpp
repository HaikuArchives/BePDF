#include <Application.h>
#include <Resources.h>
#include <Mime.h>
#include <TranslationUtils.h>
#include "ResourceLoader.h"

static const type_code
	icon = 'ICON',
	miniIcon = 'MICN';

BBitmap *LoadLargeIcon(const char *name) {
	BResources *res = BApplication::AppResources();
	if (res != NULL) {
		size_t length;
		const void *bits = res->LoadResource(icon, name, &length);
		if ((bits != NULL) && (length == B_LARGE_ICON * B_LARGE_ICON)) {
			BRect rect(0, 0, B_LARGE_ICON-1, B_LARGE_ICON-1);
			BBitmap *bitmap = new BBitmap(rect, B_CMAP8);
			bitmap->SetBits(bits, B_LARGE_ICON * B_LARGE_ICON, 0, B_CMAP8);
			return bitmap;
		}
	}
	return NULL;
}

BBitmap *LoadLargeIcon(int32 id) {
	BResources *res = BApplication::AppResources();
	if (res != NULL) {
		size_t length;
		const void *bits = res->LoadResource(icon, id, &length);
		if ((bits != NULL) && (length == B_LARGE_ICON * B_LARGE_ICON)) {
			BRect rect(0, 0, B_LARGE_ICON-1, B_LARGE_ICON-1);
			BBitmap *bitmap = new BBitmap(rect, B_CMAP8);
			bitmap->SetBits(bits, B_LARGE_ICON * B_LARGE_ICON, 0, B_CMAP8);
			return bitmap;
		}
	}
	return NULL;
}

BBitmap *LoadMiniIcon(const char *name) {
	BResources *res = BApplication::AppResources();
	if (res != NULL) {
		size_t length;
		const void *bits = res->LoadResource(miniIcon, name, &length);
		if ((bits != NULL) && (length == B_MINI_ICON * B_MINI_ICON)) {
			BRect rect(0, 0, B_MINI_ICON-1, B_MINI_ICON-1);
			BBitmap *bitmap = new BBitmap(rect, B_CMAP8);
			bitmap->SetBits(bits, B_MINI_ICON * B_MINI_ICON, 0, B_CMAP8);
			return bitmap;
		}
	}
	return NULL;
}

BBitmap *LoadMiniIcon(int32 id) {
	BResources *res = BApplication::AppResources();
	if (res != NULL) {
		size_t length;
		const void *bits = res->LoadResource(miniIcon, id, &length);
		if ((bits != NULL) && (length == B_MINI_ICON * B_MINI_ICON)) {
			BRect rect(0, 0, B_MINI_ICON-1, B_MINI_ICON-1);
			BBitmap *bitmap = new BBitmap(rect, B_CMAP8);
			bitmap->SetBits(bits, B_MINI_ICON * B_MINI_ICON, 0, B_CMAP8);
			return bitmap;
		}
	}
	return NULL;
}

BBitmap *LoadBitmap(const char *name, uint32 type_code) {
	if (type_code == B_TRANSLATOR_BITMAP) {
		return BTranslationUtils::GetBitmap(type_code, name);
	} else {
		BResources *res = BApplication::AppResources();
		if (res != NULL) {
			size_t length;
			const void *bits = res->LoadResource(type_code, name, &length);
			BMessage m;
			if (bits && B_OK == m.Unflatten((char*)bits)) {
				BBitmap* bitmap = (BBitmap*)BBitmap::Instantiate(&m);
				return bitmap;
			}
		}
		return NULL;
	}
}

BBitmap *LoadBitmap(int32 id, uint32 type_code) {
	return BTranslationUtils::GetBitmap(type_code, id);
}
