/*  
	BeOS Front-end du PDF file reader xpdf.
	Copyright (C) 2000-2002 Michael Pfeiffer

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

#include <Alert.h>

#include <layout-all.h>

#include <MDividable.h>

#include "BePDF.h"
#include "LayoutUtils.h"
#include "PDFWindow.h"
#include "PrintSettingsWindow.h"
#include "StringLocalization.h"

PrintSettingsWindow::PrintSettingsWindow(PDFDoc *doc, GlobalSettings *settings, BLooper *looper) 
	: MWindow(BRect(0, 0, 100, 100), TRANSLATE("Print Settings"), 
		B_TITLED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, 0), 
	mDoc(doc), mLooper(looper), mSettings(settings), mZoomValue(settings->GetZoomPrinter()) {
	
	AddCommonFilter(new EscapeMessageFilter(this, B_QUIT_REQUESTED));

	MoveTo(settings->GetPrinterWindowPosition());
	float w, h;
	settings->GetPrinterWindowSize(w, h);
	ResizeTo(w, h);

	char zoomStr[10];
	sprintf(zoomStr, "%d", (int)mZoomValue);
	
	// Resolution
	BString label;
	MPopup* resolution = new MPopup(MakeLabel(label, TRANSLATE("DPI")), NULL);
	AddItem(resolution, TRANSLATE("Max. Printer Resolution"), MSG_DPI_CHANGED);
	resolution->Menu()->AddSeparatorItem();
	AddItem(resolution, "72", MSG_DPI_CHANGED);
	AddItem(resolution, "300", MSG_DPI_CHANGED);
//	AddItem(resolution, "600", MSG_DPI_CHANGED);
//	AddItem(resolution, "720", MSG_DPI_CHANGED);
//	AddItem(resolution, "1440", MSG_DPI_CHANGED);

	// Page selection
	MPopup* pages  = new MPopup(MakeLabel(label, TRANSLATE("Pages")), NULL);
	AddItem(pages, TRANSLATE("All Pages"), MSG_SELECTION_CHANGED);
	AddItem(pages, TRANSLATE("Odd Pages"), MSG_SELECTION_CHANGED);
	AddItem(pages, TRANSLATE("Even Pages"), MSG_SELECTION_CHANGED);

	// Print order
	MPopup* order = new MPopup(MakeLabel(label, TRANSLATE("Order")), NULL);
	AddItem(order, TRANSLATE("Normal"), MSG_ORDER_CHANGED);
	AddItem(order, TRANSLATE("Reverse"), MSG_ORDER_CHANGED);
	
	// Rotation
	MPopup* rotation = new MPopup(MakeLabel(label, TRANSLATE("Rotation")), NULL);
	AddItem(rotation, "0°", MSG_ROTATION_CHANGED);
	AddItem(rotation, "90°", MSG_ROTATION_CHANGED);
	AddItem(rotation, "180°", MSG_ROTATION_CHANGED);
	AddItem(rotation, "270°", MSG_ROTATION_CHANGED);

	// Color mode
	MPopup* mode  = new MPopup(MakeLabel(label, TRANSLATE("Mode")), NULL);
	AddItem(mode, TRANSLATE("Color"), MSG_COLOR_MODE_CHANGED);
	AddItem(mode, TRANSLATE("Gray scale"), MSG_COLOR_MODE_CHANGED);
	
	MView *view = new VGroup(
		new Space(GetBorder()),

		resolution,
		pages,
		order,			
		rotation,
		mode,		
		mZoom = new MTextControl(MakeLabel(label, TRANSLATE("Zoom (%)")), 
				"", 
				new BMessage(MSG_ZOOM_CHANGED)), 				
		new VGroup(
			mPage = new MTextControl( MakeLabel(label, TRANSLATE("Page")), "1", new BMessage(MSG_PAGE_CHANGED)), 
			mWidth = new MStringView(""),
			mHeight = new MStringView(""),
			new Space(GetFiller()),
		0),

		new Space(GetSeparator()),

		mPrint = new MButton(TRANSLATE("Print"), new BMessage(MSG_PRINT), NULL, minimax(-1, -1, -1, -1)),

		new Space(GetBorder()),
	0);

	view = new HGroup(
		new Space(GetBorder()),
		view,
		new Space(GetBorder()),
		0
	);

	mPrint->SetEnabled(mDoc->okToPrint());

	AddChild(dynamic_cast<BView*>(view));
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
	resolution->Menu()->ItemAt(i)->SetMarked(true);

	// Page selection
	if ((settings->GetPrintSelection() >= 0) && (settings->GetPrintSelection() < 3)) {
		pages->Menu()->ItemAt(settings->GetPrintSelection())->SetMarked(true);
	}
	
	// Print order
	if (settings->GetPrintOrder() == 0) {
		i = 0;
	} else {
		i = 1;
	}
	order->Menu()->ItemAt(i)->SetMarked(true);

	// Rotation
	i = int(settings->GetRotationPrinter() / 90) % 4;
	rotation->Menu()->ItemAt(i)->SetMarked(true);

	// Color mode
	if (settings->GetPrintColorMode() == GlobalSettings::PRINT_COLOR_MODE) {
		i = 0;
	} else {
		i = 1;
	}
	mode->Menu()->ItemAt(i)->SetMarked(true);

	// Zoom
	mZoom->SetModificationMessage(new BMessage(MSG_ZOOM_CHANGED));
	mZoom->SetText(zoomStr);
		
	// Page size info
	mPage->SetModificationMessage(new BMessage(MSG_PAGE_CHANGED));
	GetPageSize(1);

	DivideSame(view, resolution, pages, order, rotation, mode, mZoom, mPage, NULL);

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
	
	MWindow::MessageReceived(msg);
}

bool PrintSettingsWindow::QuitRequested() {
	bool quit = (mZoomValue >= 25) && (mZoomValue <= 400);
	if (!quit) {
		BAlert *warning = new BAlert(TRANSLATE("Wrong Parameter!"), 
			TRANSLATE("Zoom must be between 25 and 400!"), TRANSLATE("OK"),
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
	MWindow::FrameMoved(p);
}

void PrintSettingsWindow::FrameResized(float w, float h) {
	mSettings->SetPrinterWindowSize(w, h);
	MWindow::FrameResized(w, h);
}

void PrintSettingsWindow::GetPageSize(uint32 page) {
	PDFLock lock;
	BString s; char size[40];
	sprintf(size, "%4.2f", mDoc->getPageCropWidth(page) / 72.0);
	s = TRANSLATE("Width: "); s << size << TRANSLATE(" in");
	mWidth->SetText(s.String());
	
	sprintf(size, "%2.2f", mDoc->getPageCropHeight(page) / 72.0);
	s = TRANSLATE("Height: "); s << size << TRANSLATE(" in");
	mHeight->SetText(s.String());
}

// Appends ":" to text and returns it.
char* PrintSettingsWindow::MakeLabel(BString& string, const char* text) {
	string = text;
	string << ":  ";
	return const_cast<char*>(string.String());
}

void PrintSettingsWindow::AddItem(MPopup* popup, const char* label, uint32 what) {
	popup->Menu()->AddItem(new BMenuItem(label, new BMessage(what)));	
}
