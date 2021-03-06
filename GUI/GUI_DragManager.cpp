/*
 *  GUI_DragManager.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Mon Sep 01 2003.
 *  Copyright (c) 2003 The Nuvie Team. All rights reserved.
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

#include <cstdio>

#include "nuvieDefs.h"

#include "GUI_DragArea.h"
#include "GUI_DragManager.h"

#include "Game.h"  // -+- Included so we can set WAIT_MODE while dragging.
#include "Event.h" // /
#include "MsgScroll.h"

 GUI_DragManager::GUI_DragManager(Screen *s)
 {
  screen = s;

  message = 0;
  data = NULL;
  drag_source = NULL;
  is_out_of_range = false;
 }

GUI_status GUI_DragManager::start_drag(GUI_DragArea *src, int msg, void *d, unsigned char *icon_buf, uint16 w, uint16 h, uint8 bpp, bool out_of_range)
{
 DEBUG(0,LEVEL_DEBUGGING,"Start Drag\n");

 drag_source = src;
 message = msg;
 data = d;
 is_out_of_range = out_of_range;
 //Game::get_game()->get_event()->set_mode(WAIT_MODE);
 Game::get_game()->pause_user();
 return GUI_DRAG_AND_DROP;
}

void GUI_DragManager::drop(GUI_DragArea *drag_target, int x, int y)
{
 DEBUG(0,LEVEL_DEBUGGING,"Drop\n");

 //Game::get_game()->get_event()->endAction(); // WAIT_MODE
 Game::get_game()->unpause_user();
 if(is_out_of_range)
 {
   MsgScroll *scroll = Game::get_game()->get_scroll();
   scroll->display_string("Move-");
   scroll->display_string(Game::get_game()->get_obj_manager()->look_obj((Obj*)data)); // getting obj name
   scroll->message(".\n\nOut of range!\n\n");
   drag_source->drag_drop_failed(x,y,message,data);
 }
 else if(drag_target->drag_accept_drop(x,y,message,data))
  {
   //inform the source of the success so it can perform operations before the drop takes place.
   drag_source->drag_drop_success(x,y,message,data);
   drag_target->drag_perform_drop(x,y,message,data);
  }
 else
   drag_source->drag_drop_failed(x,y,message,data);


 drag_source = NULL;

 return;
}

void GUI_DragManager::draw(int x, int y)
{
	if (!drag_source)
		return;

	drag_source->drag_draw (x, y, message, data);
}

GUI_DragArea *GUI_DragManager::get_source()
{
	return drag_source;
}

