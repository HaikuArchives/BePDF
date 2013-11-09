#include "Test.h"
#include "BitmapButton.h"
#include "MultiButton.h"
#include "ResourceLoader.h"

// Creation of bitmaps for a toolbar icon from bitmap in off state
// A bitmap button can have the following states:
// off or on and enabled or disabled
// off enabled: button must be given (background must be transparent)
// on enabled:  adds a shadow
// off disabled: grayed version of off enabled bitmap
// on disabled:  grayed version of on enabled bitmap

// add a shadow
// source image    destination image
//   01234567        01234567
// 0 ........        ........
// 1 .XXX....        .XXX....
// 2 ........        ..***...
// 3 ........        ...+++..
// 4 ..XXX...        ..XXX...
// 5 ..X.X...        ..X*X*..
// 6 ...X....        ...X.*+.
// 7 ........        ....*...


AppWindow::AppWindow(BRect aRect) 
	: BWindow(aRect, APPLICATION, B_TITLED_WINDOW, 0) {
	// add menu bar
	BRect rect = BRect(0,0,aRect.Width(), aRect.Height());
	menubar = new BMenuBar(rect, "menu_bar");
	BMenu *menu; 
	BMenuItem *item; 

	menu = new BMenu("Test");
	menu->AddItem(new BMenuItem("About ...", new BMessage(B_ABOUT_REQUESTED)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q')); 
	menubar->AddItem(menu);

	AddChild(menubar);
	// add view
	aRect.Set(0 + 2, menubar->Bounds().Height()+1 + 2, aRect.Width(), aRect.Height());
	BView *view = new BView(aRect, "", B_FOLLOW_ALL, 0);
	rgb_color gray = {222, 219, 222, 255};
	view->SetViewColor(gray);
	
	mToolTip = new ToolTip();
	
	BitmapButton * button = new ResourceBitmapButton (aRect, 
	                                "first_btn", "FIRST_OFF", "FIRST_ON", 
	                                new BMessage('firs'));
	button->SetToolTip(mToolTip, "First");
	mTipItem = button->GetToolTipItem();
	
	view->AddChild (button);

	aRect.OffsetBy(3 + button->Bounds().Width(), 0); 
	button = new ResourceBitmapButton (aRect, 
	                                "last_btn", "LAST_OFF", "LAST_ON", 
	                                new BMessage('last'));
	button->SetToolTip(mToolTip, "Last");
	view->AddChild (button);

	aRect.OffsetBy(3 + button->Bounds().Width(), 0); 
	button = new ResourceBitmapButton (aRect, 
	                                "last_btn", "LAST_OFF", "LAST_ON", 
	                                new BMessage('las1'));
	button->SetToolTip(mToolTip, "Last");
	view->AddChild (button);
	button->SetEnabled(false);
	
	aRect.OffsetBy(3 + button->Bounds().Width(), 0); 
	button = new ResourceBitmapButton (aRect, 
	                                "last_btn", "LAST_OFF", "LAST_ON", 
	                                "LAST_DISABLED_OFF", "LAST_DISABLED_ON", 
	                                new BMessage('las2'),
	                                B_TWO_STATE_BUTTON);
	button->SetValue(B_CONTROL_ON);
	button->SetToolTip(mToolTip, "Last");
	button->SetEnabled(false);
	view->AddChild (button);

	aRect.OffsetBy(3 + button->Bounds().Width(), 0); 
	button = new ResourceBitmapButton (aRect, 
	                                "last_btn", "LAST_OFF", "LAST_ON", 
	                                "LAST_DISABLED_OFF", NULL,
	                                new BMessage('las2'));
	button->SetToolTip(mToolTip, "Last");
	button->SetEnabled(false);
	view->AddChild (button);

	aRect.OffsetBy(3 + button->Bounds().Width(), 0); 
	const uint32 bitmap_code = 'BBMP';
	button = new BitmapButton (aRect, 
	                                "last_btn", LoadBitmap("PREV_OFF", bitmap_code), 
	                                LoadBitmap("PREV_OFF", bitmap_code), 
	                                new BMessage('las2'));
	button->SetToolTip(mToolTip, "Prev");
	button->SetEnabled(false);
	view->AddChild (button);

	MultiButton *mb = new MultiButton(BRect(10, 100, 50, 140), "mb", B_FOLLOW_NONE, 0);
	AddChild(mb);
	aRect.Set(0, 0, 10, 10);
	button = new ResourceBitmapButton (aRect, 
	                                "last_btn", "B1_OFF", "B1_ON", 
	                                new BMessage('xxxx'));
	button->SetToolTip(mToolTip, "1/2");
	mb->AddButton(button);

	button = new ResourceBitmapButton (aRect, 
	                                "last_btn", "B2_OFF", "B2_ON", 
	                                new BMessage('yyyy'));
	button->SetToolTip(mToolTip, "3/4");
	mb->AddButton(button);
	
	AddChild(view);
	// make window visible
	Show();
	// new BMessageRunner(BMessenger(NULL, this), new BMessage('runn'), 100000);
}

void AppWindow::MessageReceived(BMessage *message) {
static int32 i = 0;
char string[80];
	// message->PrintToStream(); fflush(stdout);
	fprintf(stderr, "AppWindow %4.4c\n", message->what);
	switch(message->what) {
	case MENU_APP_NEW: 
		break; 
	case 'runn':
		sprintf(string, "Test %d", i++);
		mTipItem->SetLabel(string);
		break;
	case B_ABOUT_REQUESTED:
		AboutRequested();
		break;
	default:
		BWindow::MessageReceived(message);
	}
}


bool AppWindow::QuitRequested() {
	mToolTip->PostMessage(B_QUIT_REQUESTED);
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}

void AppWindow::AboutRequested() {
	BAlert *about = new BAlert(APPLICATION, 
		APPLICATION " " VERSION "\nThis program is freeware under GPL.\n\n"
		"Written 2000.\n\n"
		"By Michael Pfeiffer.\n\n"
		"EMail: michael.pfeiffer@utanet.at.","Close");
		about->Go();
}

App::App() : BApplication("application/x-vnd.test") {
	BRect aRect;
	// set up a rectangle and instantiate a new window
	aRect.Set(100, 80, 410, 380);
	window = NULL;
	window = new AppWindow(aRect);		
}

int main(int argc, char *argv[]) { 
	be_app = NULL;
	App app;
	be_app->Run();
	return 0;
}