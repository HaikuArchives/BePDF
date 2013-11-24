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

#ifndef INPUT_ENABLER_H
#define INPUT_ENABLER_H

#include <be/interface/Control.h>
#include <be/interface/Menu.h>
#include <be/interface/MenuItem.h>
#include <be/interface/TextView.h>

class InputEnablerItem {
		bool cur_state;
		bool new_state;
		int  cmd;
		
	public:
		enum type {
			undefined_type,
			menu_type,
			menu_item_type,
			control_type,
			control_value_type,
			text_view_type,
		};
		InputEnablerItem(bool init, int32 cmd) : cur_state(init), new_state(init), cmd(cmd) { }
		InputEnablerItem(bool init) : cur_state(init), new_state(init), cmd(0) { }
		virtual ~InputEnablerItem() { }
		
		void SetCmd(int cmd)     { this->cmd = cmd; }
		int  Cmd()               { return cmd; }
		void SetEnabled(bool enable) { new_state = enable; }
		void Update();

		virtual void UpdateItem(bool enable) = 0;
		int Type()               { return undefined_type; }
};

// Sets the enabled state of a menu.
class IEMenu : public InputEnablerItem {
	BMenu* menu;
	public:
		IEMenu(BMenu* menu, int cmd) : InputEnablerItem(true, cmd), menu(menu) { }
		void UpdateItem(bool enable) { menu->SetEnabled(enable); }
		int Type()                   { return menu_type; }
};

// Sets the enabled state of a menu item.
class IEMenuItem : public InputEnablerItem {
	BMenuItem* menu;
	public:
		IEMenuItem(BMenuItem* menu, int cmd) : InputEnablerItem(true, cmd), menu(menu) { }
		IEMenuItem(BMenuItem* menu);
		void UpdateItem(bool enable) { menu->SetEnabled(enable); }
		int Type()                   { return menu_item_type; }
};

// Sets the enabled state of a control.
class IEControl : public InputEnablerItem {
	BControl* control;
	
	public:
		IEControl(BControl* control, int cmd) : InputEnablerItem(true, cmd), control(control) { }
	
		void UpdateItem(bool enable) { control->SetEnabled(enable); }
		int Type()                   { return control_type; }
};

// Sets the value of a control to B_CONTROL_ON or B_CONTROL_OFF.
class IEControlValue : public InputEnablerItem {
	BControl* control;
	
	public:
		IEControlValue(BControl* control, int cmd) : InputEnablerItem(true, cmd), control(control) { }
		
		void UpdateItem(bool enable) { control->SetValue(enable ? B_CONTROL_ON : B_CONTROL_OFF); }
		int Type()                   { return control_value_type; }
};

// Sets the editable state of a text view.
class IETextView : public InputEnablerItem {
	BTextView* textView;
	
	public:
		IETextView(BTextView* textView, int cmd) : InputEnablerItem(true, cmd), textView(textView) { }
	
		void UpdateItem(bool enable) { textView->MakeEditable(enable); }
		int Type()                   { return text_view_type; }
};

class InputEnabler {
	BList mItems;
	
		InputEnablerItem* FindItem(int type, int32 cmd);
	public:
		InputEnabler() { };
		~InputEnabler();
		
		void Register(InputEnablerItem* item);
		void Unregister(int type, int32 cmd);
		
		void SetEnabled(int type, int32 cmd, bool enable);
		void SetEnabled(int32 cmd, bool enable);
		void Update();
};

#endif
