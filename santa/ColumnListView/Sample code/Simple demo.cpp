//Generic test app framework
#include <Application.h>
#include <Window.h>
#include <Region.h>
#include <string.h>

#include "ColumnListView.h"
#include "CLVColumn.h"
#include "CLVListItem.h"
#include "NewStrings.h"

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

class DemoItem : public CLVListItem
{
	public:
		DemoItem(uint32 level, bool superitem, bool expanded, BBitmap* icon, char* text0, char* text1, char* text2, char* text3, char* text4, char* text5, char* text6, char* text7);
		~DemoItem();
		void DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
		void Update(BView *owner, const BFont *font);
		static int MyCompare(const CLVListItem* Item1, const CLVListItem* Item2, int32 KeyColumn);

	private:
		BBitmap* fIcon;
		char* fText[8];
		float fTextOffset;
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
	CLVContainerView* ContainerView;
	MyColumnListView = new ColumnListView(bounds,&ContainerView,NULL,B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE,B_SINGLE_SELECTION_LIST,false,false,false,false,B_NO_BORDER);
	MyColumnListView->AddColumn(new CLVColumn(NULL,20.0,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	MyColumnListView->AddColumn(new CLVColumn("Name",108.0,CLV_SORT_KEYABLE,50.0));
	MyColumnListView->AddColumn(new CLVColumn("Size",70.0,CLV_SORT_KEYABLE));
	MyColumnListView->AddColumn(new CLVColumn("Date",131.0,CLV_SORT_KEYABLE));
	MyColumnListView->AddColumn(new CLVColumn("Label",180.0,CLV_SORT_KEYABLE));
	MyColumnListView->AddColumn(new CLVColumn("Locked to right",161.0,CLV_LOCK_WITH_RIGHT |
		CLV_SORT_KEYABLE));
	MyColumnListView->AddColumn(new CLVColumn("Boolean",55.0,CLV_SORT_KEYABLE));
	MyColumnListView->AddColumn(new CLVColumn("Another bool",80.0,CLV_SORT_KEYABLE));
	MyColumnListView->AddColumn(new CLVColumn("Locked at end",237.0,CLV_LOCK_AT_END|CLV_NOT_MOVABLE|
		CLV_SORT_KEYABLE));
	MyColumnListView->SetSortFunction(DemoItem::MyCompare);
	AddCLVItems(MyColumnListView);
	AddChild(ContainerView);
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
: CLVListItem(level, superitem, expanded, 20.0)
{
	fIcon = icon;
	fText[0] = new char[strlen(text0)+1];
	strcpy(fText[0],text0);
	fText[1] = new char[strlen(text1)+1];
	strcpy(fText[1],text1);
	fText[2] = new char[strlen(text2)+1];
	strcpy(fText[2],text2);
	fText[3] = new char[strlen(text3)+1];
	strcpy(fText[3],text3);
	fText[4] = new char[strlen(text4)+1];
	strcpy(fText[4],text4);
	fText[5] = new char[strlen(text5)+1];
	strcpy(fText[5],text5);
	fText[6] = new char[strlen(text6)+1];
	strcpy(fText[6],text6);
	fText[7] = new char[strlen(text7)+1];
	strcpy(fText[7],text7);
}


DemoItem::~DemoItem()
{
	for(int Counter = 0; Counter <= 7; Counter++)
		delete[] fText[Counter];
}


void DemoItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
{
	rgb_color color;
	bool selected = IsSelected();
	if(selected)
		color = BeListSelectGrey;
	else
		color = White;
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{
		owner->SetHighColor(color);
		owner->FillRect(item_column_rect);
	}
	BRegion Region;
	Region.Include(item_column_rect);
	owner->ConstrainClippingRegion(&Region);
	owner->SetHighColor(Black);
	if(column_index == 0)
	{
		item_column_rect.left += 2.0;
		item_column_rect.right = item_column_rect.left + 15.0;
		item_column_rect.top += 2.0;
		if(Height() > 20.0)
			item_column_rect.top += ceil((Height()-20.0)/2.0);
		item_column_rect.bottom = item_column_rect.top + 15.0;
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(fIcon, BRect(0.0,0.0,15.0,15.0),item_column_rect);
		owner->SetDrawingMode(B_OP_COPY);
	}
	else if(column_index >= 1)
		owner->DrawString(fText[column_index-1],
			BPoint(item_column_rect.left+2.0,item_column_rect.top+fTextOffset));
	owner->ConstrainClippingRegion(NULL);
}


void DemoItem::Update(BView *owner, const BFont *font)
{
	CLVListItem::Update(owner,font);
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
	fTextOffset = ceil(FontAttributes.ascent) + (Height()-FontHeight)/2.0;
}


int DemoItem::MyCompare(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn)
{
	char* Text1 = ((DemoItem*)a_Item1)->fText[KeyColumn-1];
	char* Text2 = ((DemoItem*)a_Item2)->fText[KeyColumn-1];
	return strcasecmp(Text1,Text2);
}

