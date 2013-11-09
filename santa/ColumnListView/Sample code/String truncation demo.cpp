//Generic test app framework
#include <Application.h>
#include <Window.h>
#include <Region.h>
#include <string.h>

#include "ColumnListView.h"
#include "CLVColumn.h"
#include "CLVListItem.h"

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
		class ColumnListView* MyColumnListView;
};


class DemoItem : public CLVListItem
{
	public:
		DemoItem(uint32 level, bool superitem, bool expanded, BBitmap* icon, const char* text0, 
			const char* text1);
		~DemoItem();
		void DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
		void Update(BView *owner, const BFont *font);
		static int MyCompare(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn);
		BRect TruncateText(int32 column_index, float column_width);
			//Returns the area that needs to be redrawn, or BRect(-1,-1,-1,-1) if nothing
		virtual void ColumnWidthChanged(int32 column_index, float column_width, ColumnListView* the_view);
		virtual void FrameChanged(int32 column_index, BRect new_frame, ColumnListView* the_view);

	private:
		BBitmap* fIcon;
		char* fText[2];
		char* fTruncatedText[2];
		bool fNeedsTruncation[2];
		BRect fCachedRect[2];
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
GenericWindow::GenericWindow()
: BWindow(BRect(50,50,450,450),"Generic test window",B_DOCUMENT_WINDOW,0)
{
	BRect bounds = Bounds();
	bounds.right -= B_V_SCROLL_BAR_WIDTH;
	bounds.bottom -= B_H_SCROLL_BAR_HEIGHT;
	CLVContainerView* ContainerView;
	MyColumnListView = new ColumnListView(bounds,&ContainerView,NULL,B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE,B_SINGLE_SELECTION_LIST,true,true,true,false,B_NO_BORDER);
	MyColumnListView->AddColumn(new CLVColumn(NULL,20.0,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	MyColumnListView->AddColumn(new CLVColumn(NULL,20.0,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	MyColumnListView->AddColumn(new CLVColumn("Name",108.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->AddColumn(new CLVColumn("Description",120.0,CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|
		CLV_TELL_ITEMS_WIDTH));
	MyColumnListView->SetSortFunction(DemoItem::MyCompare);
	AddCLVItems(MyColumnListView);
	AddChild(ContainerView);
	Show();
}

void GenericWindow::AddCLVItems(ColumnListView* MyColumnListView)
{
	MyColumnListView->AddItem(new DemoItem(0,false,false,Paper_MICN,	".bash_history",	"Previous commands typed in the shell"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,Paper_MICN,	".profile",			"Setup commands for the shell"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,CDPlayer_MICN,	"CDPlayer",			"Play CD's"));
	MyColumnListView->AddItem(new DemoItem(0,true, true, Folder_MICN,	"config",			"User settings"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"add-ons",			"System and application add-ons"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"be",				"The coolest company on earth"));
	MyColumnListView->AddItem(new DemoItem(2,false,false,BeIDE_MICN,	"BeIDE",			"My choice of development environment"));
	MyColumnListView->AddItem(new DemoItem(2,true, false,Folder_MICN,	"apps",				"Programs"));
	MyColumnListView->AddItem(new DemoItem(3,true, false,Folder_MICN,	"Metrowerks",		"Another cool company"));
	MyColumnListView->AddItem(new DemoItem(4,false,false,BeIDE_MICN,	"BeIDE",			"A metrowerks product"));
	MyColumnListView->AddItem(new DemoItem(2,true, false,Folder_MICN,	"preferences",		"System settings programs"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"bin",				"Shell programs"));
	MyColumnListView->AddItem(new DemoItem(1,true, false,Folder_MICN,	"boot",				"Stuff to happen at boot time"));
	MyColumnListView->AddItem(new DemoItem(0,true, false,Folder_MICN,	"Desktop",			"We want system-provided desktop images!"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,Paper_MICN,	"Doggie.jpg",		"What's that smell?"));
	MyColumnListView->AddItem(new DemoItem(0,false,false,Midi_MICN,		"Midi",				"Music"));
}


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


//******************************************************************************************************
//**** CLVListItem
//******************************************************************************************************
DemoItem::DemoItem(uint32 level, bool superitem, bool expanded, BBitmap* icon, const char* text0,
	const char* text1)
: CLVListItem(level, superitem, expanded, 20.0)
{
	fIcon = icon;
	fText[0] = new char[strlen(text0)+1];
	strcpy(fText[0],text0);
	fTruncatedText[0] = new char[strlen(text0)+3];
	strcpy(fTruncatedText[0],text0);
	fNeedsTruncation[0] = true;
	fCachedRect[0].Set(-1,-1,-1,-1);
	fText[1] = new char[strlen(text1)+1];
	strcpy(fText[1],text1);
	fTruncatedText[1] = new char[strlen(text1)+3];
	strcpy(fTruncatedText[1],text1);
	fNeedsTruncation[1] = true;
	fCachedRect[1].Set(-1,-1,-1,-1);
}


DemoItem::~DemoItem()
{
	for(int Counter = 0; Counter < 2; Counter++)
	{
		delete[] fText[Counter];
		delete[] fTruncatedText[Counter];
	}
}


void DemoItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
{
	if(column_index == 2 || column_index == 3)
		fCachedRect[column_index-2] = item_column_rect;
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
	if(column_index == 1)
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
	else if(column_index >= 2)
	{
		if(fNeedsTruncation[column_index-2])
		{
			TruncateText(column_index,item_column_rect.right-item_column_rect.left);
			fNeedsTruncation[column_index-2] = false;
		}
		owner->DrawString(fTruncatedText[column_index-2],
			BPoint(item_column_rect.left+2.0,item_column_rect.top+fTextOffset));
	}
	owner->ConstrainClippingRegion(NULL);
}


BRect DemoItem::TruncateText(int32 column_index, float column_width)
//Returns whether the truncated text has changed
{
	column_width -= 2;
		//Because when I draw the text I start drawing 2 pixels to the right from the column's left edge
	BRect invalid(-1,-1,-1,-1);
	char* new_text = new char[strlen(fText[column_index-2])+3];
	GetTruncatedString(fText[column_index-2],new_text,column_width);
	if(strcmp(fTruncatedText[column_index-2],new_text)!=0)
	{
		//The truncated text has changed
		invalid = fCachedRect[column_index-2];
		if(invalid != BRect(-1,-1,-1,-1))
		{
			//Figure out which region just got changed
			int32 cmppos;
			int32 cmplen = strlen(new_text);
			char remember = 0;
			for(cmppos = 0; cmppos <= cmplen; cmppos++)
				if(new_text[cmppos] != fTruncatedText[column_index-2][cmppos])
				{
					remember = new_text[cmppos];
					new_text[cmppos] = 0;
					break;
				}
			invalid.left += 2 + be_plain_font->StringWidth(new_text);
			new_text[cmppos] = remember;
		}
		//Remember the new truncated text
		strcpy(fTruncatedText[column_index-2],new_text);
	}
	delete[] new_text;
	return invalid;
}


void DemoItem::ColumnWidthChanged(int32 column_index, float column_width, ColumnListView* the_view)
{
	if(fCachedRect[column_index-2] != BRect(-1,-1,-1,-1))
	{
		float usable_width = column_width - ExpanderShift(column_index,the_view);
		float width_delta = usable_width-(fCachedRect[column_index-2].right-fCachedRect[column_index-2].left);		
		fCachedRect[column_index-2].right += width_delta;
		//Need to update the cached rects of any columns that follow
		for(int32 column_check = 2; column_check < 4; column_check++)
			if(fCachedRect[column_check-2].left > fCachedRect[column_index-2].left)
				//This column comes after the one that just got resized, so update it's cached rect
				fCachedRect[column_check-2].OffsetBy(width_delta,0);

		//If it's onscreen, truncate and invalidate the changed area
		if(fCachedRect[column_index-2].Intersects(the_view->Bounds()))
		{
			BRect invalid = TruncateText(column_index, usable_width);
			fNeedsTruncation[column_index-2] = false;
			if(invalid != BRect(-1.0,-1.0,-1.0,-1.0))
				the_view->Invalidate(invalid);
		}
		else
			//If it's not onscreen flag it for truncation the next time it's drawn
			fNeedsTruncation[column_index-2] = true;
	}
}


void DemoItem::FrameChanged(int32 column_index, BRect new_frame, ColumnListView* the_view)
{
	if(fCachedRect[column_index-2] != new_frame)
	{
		fCachedRect[column_index-2] = new_frame;

		//If it's onscreen, truncate and invalidate the changed area
		if(new_frame.Intersects(the_view->Bounds()))
		{
			BRect invalid = TruncateText(column_index, new_frame.right-new_frame.left);
			fNeedsTruncation[column_index-2] = false;
			if(invalid != BRect(-1.0,-1.0,-1.0,-1.0))
				the_view->Invalidate(invalid);
		}
		else
			//If it's not onscreen flag it for truncation the next time it's drawn
			fNeedsTruncation[column_index-2] = true;		
	}
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
	DemoItem* Item1 = (DemoItem*)a_Item1;
	DemoItem* Item2 = (DemoItem*)a_Item2;

	char* Text1 = ToUpperStr(Item1->fText[KeyColumn-2]);
	char* Text2 = ToUpperStr(Item2->fText[KeyColumn-2]);

	int Result = strcmp(Text1,Text2);
	delete[] Text1;
	delete[] Text2;
	return Result;
}
