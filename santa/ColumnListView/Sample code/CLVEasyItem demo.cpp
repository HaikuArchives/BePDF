//Generic test app framework
//The "dirt" left by the BScrollView when resizing the window smaller (as of PR2) is Be's bug, not mine,
//but it is marked in the Be bugs database as "fixed."
#include <Application.h>
#include <Window.h>

#include "ColumnListView.h"
#include "CLVColumn.h"
#include "CLVEasyItem.h"

//******************************************************************************************************
//**** Classes used
//******************************************************************************************************
class GenericApp : public BApplication
{
	public:
		GenericApp();
		~GenericApp();
};

class GenericWindow : public BWindow
{
	public:
		GenericWindow();
		~GenericWindow();
		virtual	void Quit();
		void AddCLVItems(ColumnListView* MyColumnListView);
		ColumnListView* MyColumnListView;
};

class DemoItem : public CLVEasyItem
{
	public:
		DemoItem(uint32 level, bool superitem, bool expanded, BBitmap* icon, char* text0, char* text1, char* text2, char* text3, char* text4, char* text5, char* text6, char* text7);
		~DemoItem();
};


//******************************************************************************************************
//**** Application
//******************************************************************************************************

int main()
{
	new GenericApp;
	be_app->Run();
	delete be_app;
}

#include "Paper.h"
#include "CDPlayer.h"
#include "Folder.h"
#include "BeIDE.h"
#include "MidiIcon.h"

GenericApp::GenericApp()
: BApplication("application/x-vnd.BT-Generic")
{
	BeIDE_MICN = new PrefilledBitmap(BRect(0.0,0.0,15.0,15.0),B_COLOR_8_BIT,BeIDE_MData,false,false);
	CDPlayer_MICN = new PrefilledBitmap(BRect(0.0,0.0,15.0,15.0),B_COLOR_8_BIT,CDPlayer_MData,false,false);
	Folder_MICN = new PrefilledBitmap(BRect(0.0,0.0,15.0,15.0),B_COLOR_8_BIT,Folder_MData,false,false);
	Midi_MICN = new PrefilledBitmap(BRect(0.0,0.0,15.0,15.0),B_COLOR_8_BIT,Midi_MData,false,false);
	Paper_MICN = new PrefilledBitmap(BRect(0.0,0.0,15.0,15.0),B_COLOR_8_BIT,Paper_MData,false,false);
	new GenericWindow;
}

GenericApp::~GenericApp()
{ }


//******************************************************************************************************
//**** Window
//******************************************************************************************************

GenericWindow::~GenericWindow()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
}


void GenericWindow::Quit()
{
	DemoItem* TheItem;
	do
	{
		TheItem = (DemoItem*)MyColumnListView->RemoveItem(int32(0));
		if(TheItem)
			delete TheItem;
	}while(TheItem);
	BWindow::Quit();
}

GenericWindow::GenericWindow()
: BWindow(BRect(50,50,450,450),"Generic test window",B_TITLED_WINDOW,0)
{
	BRect bounds = Bounds();
	BView* BGView = new BView(bounds,NULL,B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	BGView->SetViewColor(BeBackgroundGrey);
	AddChild(BGView);

	bounds.left += 10.0;
	bounds.right -= 10.0 + B_V_SCROLL_BAR_WIDTH;
	bounds.top += 10.0;
	bounds.bottom -= 10.0 + B_H_SCROLL_BAR_HEIGHT;
	CLVContainerView* ContainerView;
	MyColumnListView = new ColumnListView(bounds,&ContainerView,NULL,B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE,B_SINGLE_SELECTION_LIST,true,true,true,true,B_FANCY_BORDER);
	MyColumnListView->AddColumn(new CLVColumn(NULL,20.0,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	MyColumnListView->AddColumn(new CLVColumn(NULL,20.0,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	MyColumnListView->AddColumn(new CLVColumn("Name",108.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH,50.0));
	MyColumnListView->AddColumn(new CLVColumn("Size",70.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Date",131.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Label",180.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Locked to right",161.0,CLV_LOCK_WITH_RIGHT|
		CLV_MERGE_WITH_RIGHT|CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Boolean",55.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Another bool",80.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Locked at end",237.0,CLV_LOCK_AT_END|CLV_NOT_MOVABLE|
		CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH|CLV_RIGHT_JUSTIFIED));
	MyColumnListView->SetSortFunction(CLVEasyItem::CompareItems);
	AddCLVItems(MyColumnListView);
	BGView->AddChild(ContainerView);
	Show();
}


void GenericWindow::AddCLVItems(ColumnListView* MyColumnListView)
{
	MyColumnListView->AddItem(new DemoItem(0,false,false,Paper_MICN,	".bash_history",	"11 K",			"Today",					"ColumnListView is pretty cool, huh?",	"Something",						"true", "false","Something else"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,Paper_MICN,	".profile",			"236 bytes",	"Friday, October 31, 1997",	"I thought you would think so",			"Something entirely new",			"true", "true", "Something useful"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,CDPlayer_MICN,	"CDPlayer",			"<link>",		"Last full moon",			"Donuts...",							"Is there anything they can't do?",	"false","false","Homer"));
	MyColumnListView->AddItem(new DemoItem(0,true, true, Folder_MICN,	"config",			"<folder>",		"January 1, 1998",			"Wait a minute...",						"That's in the future!",			"false","false","Time travel is fun."));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"add-ons",			"<folder>",		"December 24, 1997",		"Ne noo ne noo",						"Getting really bored now...",		"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"be",				"<folder>",		"December 25, 1997",		"Blah",									"Blah",								"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(2,false,false,BeIDE_MICN,	"BeIDE",			"<link>",		"Last thursday",			"Hi ho, Hi ho",							"It's off to work we go",			"false","true", "Something fun"));
	MyColumnListView->AddItem(new DemoItem(2,true, false,Folder_MICN,	"apps",				"<link>",		"Friday",					"Blah",									"Blah",								"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(3,true, false,Folder_MICN,	"Metrowerks",		"<folder>",		"Saturday",					"Codewarrior rulez!",					"Metrowerks rulez!",				"false","false","When I grow up I want to be just like Brian Stern"));
	MyColumnListView->AddItem(new DemoItem(4,false,false,BeIDE_MICN,	"BeIDE",			"1001 K",		"Sunday",					"Hi ho, Hi ho",							"It's off to work we go",			"false","true", "When I grow up I want to be just like Jon Watte"));
	MyColumnListView->AddItem(new DemoItem(2,true, false,Folder_MICN,	"preferences",		"<link>",		"Monday",					"Blah",									"Blah",								"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"bin",				"<folder>",		"Tuesday",					"Blah",									"Blah",								"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"boot",				"<folder>",		"Wednesday",				"Blah",									"Blah",								"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(0,true, false,Folder_MICN,	"Desktop",			"<folder>",		"Thursday",					"Blah",									"Blah",								"false","false","Blah"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,Paper_MICN,	"Doggie.jpg",		"15 MB",		"Friday",					"Bad dog!  Bad noise, bad smell!",		"Got away with it...",				"false","false","Blamed it on the dog!"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,Midi_MICN,		"Midi",				"<link>",		"April 15, 1973",			"Blah",									"Blah",								"false","true", "I quit!"));
}



//******************************************************************************************************
//**** CLVListItem
//******************************************************************************************************

DemoItem::DemoItem(uint32 level, bool superitem, bool expanded, BBitmap* icon, char* text0, char* text1, char* text2, char* text3, char* text4, char* text5, char* text6, char* text7)
: CLVEasyItem(level, superitem, expanded, 20.0)
{
	SetColumnContent(1,icon,2.0,false);
	SetColumnContent(2,text0);
	SetColumnContent(3,text1);
	SetColumnContent(4,text2);
	SetColumnContent(5,text3);
	SetColumnContent(6,text4);
	SetColumnContent(7,text5);
	SetColumnContent(8,text6);
	SetColumnContent(9,text7,true,true);
}


DemoItem::~DemoItem()
{ }
