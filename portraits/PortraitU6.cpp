/*
 *  PortraitU6.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Mon Dec 2 2013.
 *  Copyright (c) 2013. All rights reserved.
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

#include "nuvieDefs.h"

#include "Configuration.h"
#include "Console.h"
#include "NuvieIOFile.h"
#include "U6Shape.h"
#include "Dither.h"
#include "Game.h"

#include "ActorManager.h"
#include "Actor.h"
#include "PortraitU6.h"
#include "U6Lzw.h"
#include "U6misc.h"

#include "U6objects.h"

//define temporary portraits.

#define PORTRAIT_U6_GARGOYLE 194
#define PORTRAIT_U6_GUARD 193
#define PORTRAIT_U6_WISP 192
#define PORTRAIT_U6_EXODUS 191
#define PORTRAIT_U6_NOTHING 188


bool PortraitU6::init()
{
 std::string filename;

 avatar_portrait_num = 0;

 width = 56;
 height = 64;

 config_get_path(config,"portrait.a",filename);
 if(portrait_a.open(filename,4) == false)
 {
   ConsoleAddError("Opening " + filename);
   return false;
 }
 config_get_path(config,"portrait.b",filename);
 if(portrait_b.open(filename,4) == false)
 {
   ConsoleAddError("Opening " + filename);
   return false;
 }
 config_get_path(config,"portrait.z",filename);
 if(portrait_z.open(filename,4) == false)
 {
   ConsoleAddError("Opening " + filename);
   return false;
 }

 return true;
}

bool PortraitU6::load(NuvieIO *objlist)
{
  // U6 only?
 objlist->seek(0x1c72);

 avatar_portrait_num = objlist->read1(); //read in the avatar portrait number from objlist.
 if(avatar_portrait_num > 0)
   avatar_portrait_num--;

 return true;
}

uint8 PortraitU6::get_portrait_num(Actor *actor)
{
  uint8 num;

  if(actor == NULL)
    return NO_PORTRAIT_FOUND;

  num = actor->get_actor_num();

  if(actor->is_avatar()) // avatar portrait
  {
    num = avatar_portrait_num;
  }
  else
  {
    if(num != 0)
      num -= 1;

    if(num == (188-1))
      num = PORTRAIT_U6_EXODUS-1; // Exodus
    else if(num >= (192-1) && num <= (200-1)) // Shrines, Temple of Singularity
      return(NO_PORTRAIT_FOUND);
    else if(num > 194) // there are 194 npc portraits
    {
      switch(actor->get_obj_n()) //check for temporary actors with portraits. eg guards and wisps
      {
      case OBJ_U6_GUARD : num = PORTRAIT_U6_GUARD-1; break;
      case OBJ_U6_WISP : num = PORTRAIT_U6_WISP-1; break;
      case OBJ_U6_GARGOYLE : num = PORTRAIT_U6_GARGOYLE-1; break;
      default : return NO_PORTRAIT_FOUND;
      }
    }
  }

  return num;
}

unsigned char *PortraitU6::get_portrait_data(Actor *actor)
{
 U6Lzw lzw;
 U6Lib_n *portrait;
 unsigned char *lzw_data;
 uint32 new_length;
 unsigned char *new_portrait;
 uint8 num = get_portrait_num(actor);
 if(num == NO_PORTRAIT_FOUND)
   return NULL;

 if(actor->is_avatar()) // avatar portrait
 {
   portrait = &portrait_z;
 }
 else
 {
   if(num < 98)
     portrait = &portrait_a;
   else
   {
     num -= 98;
     portrait = &portrait_b;
   }
 }

 lzw_data = portrait->get_item(num);
 if(!lzw_data)
   return NULL;
 new_portrait = lzw.decompress_buffer(lzw_data, portrait->get_item_size(num), new_length);
 free(lzw_data);
 Game::get_game()->get_dither()->dither_bitmap(new_portrait,PORTRAIT_WIDTH,PORTRAIT_HEIGHT,true);

 return new_portrait;
}
