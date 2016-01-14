#include "BitmapButton.h"
#include "ResourceLoader.h"

BPicture *BitmapButton::BitmapToPicture(BBitmap *bitmap) {
	if (bitmap) {
		BeginPicture(new BPicture());
		DrawBitmap(bitmap);
		return EndPicture();
	}
	return NULL;
}

BPicture *BitmapButton::BitmapToGrayedPicture(BBitmap *bitmap) {
	if (bitmap) {
		BRect rect(bitmap->Bounds());
		BeginPicture(new BPicture());
		DrawBitmap(bitmap);
		SetHighColor(255, 255, 255, 128);
		SetDrawingMode(B_OP_ALPHA);
		FillRect(rect);
		return EndPicture();
	}
	return NULL;
}

BitmapButton::BitmapButton(BRect frame, const char *name,
		BBitmap *off, BBitmap *on,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BPictureButton(frame, name,
			&BPicture(), &BPicture(),
			message,
			behavior, resizingMode, flags),
		off(off), on(on), disabledOff(NULL), disabledOn(NULL),
		toolTipItem(NULL),
		mouseEnteredView(false) {
	// resize button to include both bitmaps
	float width = 0, height = 0;

	if (off) {
		BRect rect = off->Bounds();
		width = rect.Width(); height = rect.Height();
	}

	if (on) {
		BRect rect = on->Bounds();
		if (width < rect.Width()) width = rect.Width();
		if (height < rect.Height()) height = rect.Height();
	}

	if ((width != 0) && (height != 0)) ResizeTo(width, height);
}

BitmapButton::~BitmapButton() {
	delete toolTipItem; toolTipItem = NULL;
	delete on; on = NULL;
	delete off; off = NULL;
	delete disabledOn; disabledOn = NULL;
	delete disabledOff; disabledOff = NULL;
}

void BitmapButton::AttachedToWindow() {
	if (off == NULL) return; // already initialized
	// generate pictures from buttons
	BPicture *p = BitmapToPicture(off);
	SetEnabledOff(p); delete p;

	p = BitmapToPicture(on);
	SetEnabledOn(p); delete p;

	// disabled images: gray enabled image if disabled image not available
	if (disabledOff) {
		p = BitmapToPicture(disabledOff);
		delete disabledOff; disabledOff = NULL;
	} else {
		p = BitmapToGrayedPicture(off);
	}
	SetDisabledOff(p); delete p;

	if (disabledOn) {
		p = BitmapToPicture(disabledOn);
		delete disabledOn; disabledOn = NULL;
	} else {
		p = BitmapToGrayedPicture(on);
	}
	SetDisabledOn(p); delete p;

	delete off; delete on; off = NULL; on = NULL;

	BPictureButton::AttachedToWindow();
}

void BitmapButton::DetachedFromWindow() {
	if (toolTipItem) toolTipItem->Hide();
}

void BitmapButton::SetEnabledOn(BBitmap *on) {
	delete this->on; this->on = NULL;
	// attached to window?
	if (Window()) { // set picture
		BPicture *p = BitmapToPicture(on);
		SetEnabledOn(p);
		delete p; delete on;
	} else { // store bitmap; set picture later
		this->on = on;
	}
}

void BitmapButton::SetEnabledOff(BBitmap *off) {
	delete this->off; this->off = NULL;
	// attached to window?
	if (Window()) { // set picture
		BPicture *p = BitmapToPicture(off);
		SetEnabledOff(p);
		delete p; delete off;
	} else { // store bitmap; set picture later
		this->off = off;
	}
}

void BitmapButton::SetDisabledOn(BBitmap *on) {
	delete disabledOn; disabledOn = NULL;
	// attached to window?
	if (Window()) { // set picture
		BPicture *p = BitmapToPicture(on);
		SetDisabledOn(p);
		delete p; delete on;
	} else { // store bitmap; set picture later
		disabledOn = on;
	}
}

void BitmapButton::SetDisabledOff(BBitmap *off) {
	delete disabledOff; disabledOff = NULL;
	// attached to window?
	if (Window()) { // set picture
		BPicture *p = BitmapToPicture(off);
		SetDisabledOn(p);
		delete p; delete off;
	} else { // store bitmap; set picture later
		disabledOff = off;
	}
}

void BitmapButton::Draw(BRect rect) {
	BPoint p; uint32 buttons;
	GetMouse(&p, &buttons, false);
	bool mouseInsideView = Bounds().Contains(p);

	BPictureButton::Draw(rect);

	if (mouseEnteredView && mouseInsideView && IsEnabled()) {
		rgb_color white = {255, 255, 255, 255}, black = {0, 0, 0, 255};
		BRect rect(Bounds());
		BPoint p0(rect.left, rect.top),
			p1(rect.right, rect.top),
			p2(rect.right, rect.bottom),
			p3(rect.left, rect.bottom);
		BeginLineArray(4);
		if (Value() == B_CONTROL_OFF) {
			AddLine(p1, p2, black);
			AddLine(p2, p3, black);
			AddLine(p0, p1, white);
			AddLine(p3, p0, white);
		} else {
			AddLine(p3, p0, black);
			AddLine(p0, p1, black);
			AddLine(p1, p2, white);
			AddLine(p2, p3, white);
		}
		EndLineArray();
	}
}

void BitmapButton::MouseDown(BPoint point) {
	if (toolTipItem) toolTipItem->Hide();
	BPictureButton::MouseDown(point); // returns when button is released
	if (toolTipItem) {
		BPoint p; uint32 buttons;
		GetMouse(&p, &buttons, false);
		bool mouseInsideView = Bounds().Contains(p);
		if (mouseInsideView) toolTipItem->Show();
	}
}

void BitmapButton::MouseMoved(BPoint p, uint32 transit, const BMessage *msg) {
	if (toolTipItem) toolTipItem->MouseMoved(p, transit, msg);
	if (transit == B_ENTERED_VIEW) {
		mouseEnteredView = true;
		Draw(Bounds());
	} else if ((transit == B_EXITED_VIEW) || (transit == B_OUTSIDE_VIEW)) {
		mouseEnteredView = false;
		Draw(Bounds());
	}
	BPictureButton::MouseMoved(p, transit, msg);
}

void BitmapButton::SetToolTip(::ToolTip *toolTip, const char *label) {
	toolTip->AddItem(toolTipItem = new ToolTipItem(this, label));
}
//////////////////////////////////////////////////////////
// Resource Bitmap Button
ResourceBitmapButton::ResourceBitmapButton(BRect frame, const char *name,
		const char *off, const char *on,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BitmapButton(frame, name,
			LoadBitmap(off), LoadBitmap(on),
			message,
			behavior, resizingMode, flags) {
}

ResourceBitmapButton::ResourceBitmapButton(BRect frame, const char *name,
		const char *off, const char *on,
		const char *disabledOff, const char *disabledOn,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BitmapButton(frame, name,
			LoadBitmap(off), LoadBitmap(on),
			message,
			behavior, resizingMode, flags) {

	if (disabledOff) SetDisabledOff(LoadBitmap(disabledOff));
	if (disabledOn) SetDisabledOn(LoadBitmap(disabledOn));
}

ResourceBitmapButton::ResourceBitmapButton(BRect frame, const char *name,
		int32 offId, int32 onId,
		int32 disabledOffId, int32 disabledOnId,
		BMessage *message,
		uint32 behavior, uint32 resizingMode, uint32 flags) :
		BitmapButton(frame, name,
			LoadBitmap(offId), LoadBitmap(onId),
			message,
			behavior, resizingMode, flags) {

	if (disabledOffId != -1) SetDisabledOff(LoadBitmap(disabledOffId));
	if (disabledOnId != -1) SetDisabledOn(LoadBitmap(disabledOnId));
}
