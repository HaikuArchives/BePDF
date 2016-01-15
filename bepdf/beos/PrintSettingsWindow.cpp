/*
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <locale/Catalog.h>
#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <PopUpMenu.h>
#include <StringView.h>

#include "BePDF.h"
#include "LayoutUtils.h"
#include "PDFWindow.h"
#include "PrintSettingsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrintSettingsWindow"

PrintSettingsWindow::PrintSettingsWindow(PDFDoc *doc, GlobalSettings *settings, BLooper *looper)
	: BWindow(BRect(0, 0, 100, 100), B_TRANSLATE("Print settings"),
		B_TITLED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_AUTO_UPDATE_SIZE_LIMITS),
	mDoc(doc), mLooper(looper), mSettings(settings), mZoomValue(settings->GetZoomPrinter()) {

	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));

	MoveTo(settings->GetPrinterWindowPosition());
	float w, h;
	settings->GetPrinterWindowSize(w, h);
	ResizeTo(w, h);

	mPrint = new BButton("mPrint", B_TRANSLATE("Print"), new BMessage(MSG_PRINT));

	BGridLayout *grid;
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGrid()
			.GetLayout(&grid)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(mPrint)
		.End();

	int32 row = 0;

	BPopUpMenu* resolution = MakePopup(MakeLabel(B_TRANSLATE("DPI")), grid, row);
	AddItem(resolution, B_TRANSLATE("Max. printer resolution"), MSG_DPI_CHANGED);
	resolution->AddSeparatorItem();
	AddItem(resolution, "72", MSG_DPI_CHANGED);
	AddItem(resolution, "300", MSG_DPI_CHANGED);

	// Page selection
	BPopUpMenu* pages = MakePopup(MakeLabel(B_TRANSLATE("Pages")), grid, row);
	AddItem(pages, B_TRANSLATE("All pages"), MSG_SELECTION_CHANGED);
	AddItem(pages, B_TRANSLATE("Odd pages"), MSG_SELECTION_CHANGED);
	AddItem(pages, B_TRANSLATE("Even pages"), MSG_SELECTION_CHANGED);

	// Print order
	BPopUpMenu* order = MakePopup(MakeLabel(B_TRANSLATE("Order")), grid, row);
	AddItem(order, B_TRANSLATE("Normal"), MSG_ORDER_CHANGED);
	AddItem(order, B_TRANSLATE("Reverse"), MSG_ORDER_CHANGED);

	// Rotation
	BPopUpMenu* rotation = MakePopup(MakeLabel(B_TRANSLATE("Rotation")), grid,
		row);
	AddItem(rotation, "0°", MSG_ROTATION_CHANGED);
	AddItem(rotation, "90°", MSG_ROTATION_CHANGED);
	AddItem(rotation, "180°", MSG_ROTATION_CHANGED);
	AddItem(rotation, "270°", MSG_ROTATION_CHANGED);

	// Color mode
	BPopUpMenu* mode = MakePopup(MakeLabel(B_TRANSLATE("Mode")), grid, row);
	AddItem(mode, B_TRANSLATE("Color"), MSG_COLOR_MODE_CHANGED);
	AddItem(mode, B_TRANSLATE("Gray scale"), MSG_COLOR_MODE_CHANGED);

	char zoomStr[10];
	sprintf(zoomStr, "%d", (int)mZoomValue);

	mZoom = new BTextControl("mZoom", MakeLabel(B_TRANSLATE("Zoom (%)")),
		zoomStr, new BMessage(MSG_ZOOM_CHANGED));
	grid->AddItem(mZoom->CreateLabelLayoutItem(), 0, row);
	grid->AddItem(mZoom->CreateTextViewLayoutItem(), 1, row);
	row++;

	mPage = new BTextControl("mPage", MakeLabel(B_TRANSLATE("Page")), "1",
		new BMessage(MSG_PAGE_CHANGED));
	grid->AddItem(mPage->CreateLabelLayoutItem(), 0, row);
	grid->AddItem(mPage->CreateTextViewLayoutItem(), 1, row);
	row++;

	BStringView *width = new BStringView("width", B_TRANSLATE("Width: "));
	mWidth = new BStringView("mWidth", "");
	grid->AddView(width, 0, row);
	grid->AddView(mWidth, 1, row);
	row++;

	BStringView *height = new BStringView("height", B_TRANSLATE("Height: "));
	mHeight = new BStringView("mHeight", "");
	grid->AddView(height, 0, row);
	grid->AddView(mHeight, 1, row);

	mPrint->SetEnabled(mDoc->okToPrint());

	SetDefaultButton(mPrint);

	// Print settings
	int i;

	// Resolution
	i = 0;
	switch(settings->GetDPI()) {
		case 0:   i = 0; break;
		case 72:  i = 2; break;
		case 300: i = 3; break;
		case 600: i = 4; break;
		case 720: i = 5; break;
//		case 1440: i = 5; break;
	}
	resolution->ItemAt(i)->SetMarked(true);

	// Page selection
	if ((settings->GetPrintSelection() >= 0) && (settings->GetPrintSelection() < 3)) {
		pages->ItemAt(settings->GetPrintSelection())->SetMarked(true);
	}

	// Print order
	if (settings->GetPrintOrder() == 0) {
		i = 0;
	} else {
		i = 1;
	}
	order->ItemAt(i)->SetMarked(true);

	// Rotation
	i = int(settings->GetRotationPrinter() / 90) % 4;
	rotation->ItemAt(i)->SetMarked(true);

	// Color mode
	if (settings->GetPrintColorMode() == GlobalSettings::PRINT_COLOR_MODE) {
		i = 0;
	} else {
		i = 1;
	}
	mode->ItemAt(i)->SetMarked(true);

	// Zoom
	mZoom->SetModificationMessage(new BMessage(MSG_ZOOM_CHANGED));

	// Page size info
	mPage->SetModificationMessage(new BMessage(MSG_PAGE_CHANGED));
	GetPageSize(1);

	Show();
}

void PrintSettingsWindow::Refresh(PDFDoc *doc) {
	mDoc = doc;
	mPrint->SetEnabled(mDoc->okToPrint());
	GetPageSize(1);
}

void PrintSettingsWindow::MessageReceived(BMessage *msg) {
int32 index;
	if (B_OK != msg->FindInt32("index", &index)) index = -1;

	switch (msg->what) {
		case MSG_DPI_CHANGED:
			switch (index) {
				case 0: mSettings->SetDPI(0);
					break;
				case 2: mSettings->SetDPI(72);
					break;
				case 3: mSettings->SetDPI(300);
					break;
				case 4: mSettings->SetDPI(600);
					break;
				case 5: mSettings->SetDPI(720);
					break;
				case 6: mSettings->SetDPI(1440);
					break;
			}
			break;
		case MSG_ROTATION_CHANGED:
			if (index != -1) {
				mSettings->SetRotationPrinter(90 * index);
			}
			break;
		case MSG_SELECTION_CHANGED:
			if (index != -1) {
				mSettings->SetPrintSelection(index);
			}
			break;
		case MSG_ORDER_CHANGED:
			if (index != -1) {
				mSettings->SetPrintOrder(index);
			}
			break;
		case MSG_ZOOM_CHANGED: {
				int32 z = mZoomValue = atoi(mZoom->Text());

				if (z < 25) {
					z = 25;
				} else if (z > 400) {
					z = 400;
				}
				mSettings->SetZoomPrinter(z);
			}
			break;
		case MSG_COLOR_MODE_CHANGED:
			if (index == 0) {
				mSettings->SetPrintColorMode(GlobalSettings::PRINT_COLOR_MODE);
			} else if (index == 1) {
				mSettings->SetPrintColorMode(GlobalSettings::PRINT_MONOCHROME_MODE);
			}
			break;
		case MSG_PRINT: {
				BMessage msg(PRINT_NOTIFY);
				mLooper->PostMessage(&msg);
			}
			break;
		case MSG_PAGE_CHANGED: {
				int32 z = atoi(mPage->Text());
				if (z < 1) {
					z = 1;
				} else if (z > mDoc->getNumPages()) {
					z = mDoc->getNumPages();
				}
				GetPageSize(z);
			}
			break;
	}

	BWindow::MessageReceived(msg);
}

bool PrintSettingsWindow::QuitRequested() {
	bool quit = (mZoomValue >= 25) && (mZoomValue <= 400);
	if (!quit) {
		BAlert *warning = new BAlert(B_TRANSLATE("Wrong parameter!"),
			B_TRANSLATE("Zoom must be between 25 and 400!"), B_TRANSLATE("OK"),
			NULL, NULL,
			B_WIDTH_AS_USUAL, B_STOP_ALERT);
		warning->Go();
	} else if (mLooper) {
		BMessage msg(QUIT_NOTIFY);
		mLooper->PostMessage(&msg);
	}
	return quit;
}

void PrintSettingsWindow::FrameMoved(BPoint p) {
	mSettings->SetPrinterWindowPosition(p);
	BWindow::FrameMoved(p);
}

void PrintSettingsWindow::FrameResized(float w, float h) {
	mSettings->SetPrinterWindowSize(w, h);
	BWindow::FrameResized(w, h);
}

void PrintSettingsWindow::GetPageSize(uint32 page) {
	PDFLock lock;
	BString s; char size[40];
	sprintf(size, "%4.2f", mDoc->getPageCropWidth(page) / 72.0);
	s << size << B_TRANSLATE(" in");
	mWidth->SetText(s.String());

	sprintf(size, "%2.2f", mDoc->getPageCropHeight(page) / 72.0);
	s = "";
	s << size << B_TRANSLATE(" in");
	mHeight->SetText(s.String());
}

// Appends ":" to text and returns it.
const char* PrintSettingsWindow::MakeLabel(const char* text) {
	static BString tmp;
	tmp = "";
	tmp << text << ":";
	return tmp.String();
}

void PrintSettingsWindow::AddItem(BPopUpMenu* popup, const char* label, uint32 what) {
	popup->AddItem(new BMenuItem(label, new BMessage(what)));
}

BPopUpMenu* PrintSettingsWindow::MakePopup(const char *label,
	BGridLayout *layout, int32 &row)
{
	BPopUpMenu *menu = new BPopUpMenu("menu");
	BMenuField *mf = new BMenuField("mf", label, menu);
	layout->AddItem(mf->CreateLabelLayoutItem(), 0, row);
	layout->AddItem(mf->CreateMenuBarLayoutItem(), 1, row);
	row++;
	return menu;
}


