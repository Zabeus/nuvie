/*
 *  PortraitView.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Tue May 20 2003.
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

#include "PortraitView.h"

PortraitView::PortraitView(Configuration *cfg) : View(cfg)
{
 portrait_data = NULL;
}

PortraitView::~PortraitView()
{
 if(portrait_data != NULL)
   free(portrait_data);
}

bool PortraitView::init(uint16 x, uint16 y, Screen *s, Text *t, Party *p, TileManager *tm, ObjManager *om, Portrait *port)
{
 View::init(x,y,s,t,p,tm,om);
 
 portrait = port;
 cur_actor_num = 0;
  
 return true;
}

void PortraitView::update_display()
{
 if(portrait_data != NULL)
  {
   screen->blit(origin_x+40,origin_y+16,portrait_data,8,56,64,56,false);
   display_name();
  }
  
}

bool PortraitView::handle_input(SDLKey *input)
{
 return true;
}

void PortraitView::set_portrait(uint8 actor_num, char *name)
{
   
 cur_actor_num = actor_num;
 
 if(portrait_data != NULL)
   free(portrait_data);
   
 portrait_data = portrait->get_portrait_data(cur_actor_num);

 if(name == NULL)
   name_string.assign("");  //FIX
 else
   name_string.assign(name);
}

void PortraitView::display_name()
{
 const char *name;
 
 name = name_string.c_str();
 
 text->drawString(screen, name, origin_x + (136 - strlen(name) * 8) / 2, origin_y+80, 0);
 
 return;
}

