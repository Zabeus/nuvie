#ifndef __ActorView_h__
#define __ActorView_h__

/*
 *  ActorView.h
 *  Nuvie
 *
 *  Created by Eric Fry on Fri Aug 22 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "View.h"

class Configuration;
class Screen;
class Text;
class TileManager;
class ObjManager;
class Portrait;
class Party;

class ActorView : public View {

 Portrait *portrait;

 unsigned char *portrait_data;
 struct actcursor_pos_s
 {
	uint8 x;
	uint32 px, py;
 } cursor_pos;
 Tile *cursor_tile;
 bool show_cursor;

 public:
 ActorView(Configuration *cfg);
 ~ActorView();

 bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Text *t, Party *p, TileManager *tm, ObjManager *om, Portrait *port);

 bool set_party_member(uint8 party_member);

 void Display(bool full_redraw);
 void update() { update_display = true; }
 void set_show_cursor(bool state);
 void moveCursorToButton(sint8 button_num);

 protected:

 void add_command_icons(Screen *tmp_screen, void *view_manager);
 void display_name();
 void display_actor_stats();
 bool in_party;
 GUI_status KeyDown(SDL_keysym key);
 void update_cursor();
 void select_button();
};

#endif /* __ActorView_h__ */

