/*
 *  EggManager.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Thu Mar 20 2003.
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

#include <list>

#include "nuvieDefs.h"
#include "Configuration.h"
 
#include "Actor.h"
#include "TileManager.h"
#include "ActorManager.h"
#include "U6misc.h"
#include "U6LList.h"
#include "EggManager.h"
#include "misc.h"
#include "NuvieIOFile.h"
#include "GameClock.h"


EggManager::EggManager(Configuration *cfg, Map *m)
{
 config = cfg;
 map = m;
}

EggManager::~EggManager()
{
 std::list<Egg *>::iterator egg;
 std::list<Egg *>::iterator next_egg;
 
 for(egg = egg_list.begin(); egg != egg_list.end();)
   {
    next_egg = egg_list.erase(egg);
    delete *egg;
    egg = next_egg;
   }

}
 
void EggManager::add_egg(Obj *egg_obj)
{
 Egg *egg;
 
 if(egg_obj == NULL)
  return;

 egg = new Egg();
 egg->obj = egg_obj;
 
 egg_list.push_back(egg);

 return;
}


void EggManager::remove_egg(Obj *egg_obj)
{
 std::list<Egg *>::iterator egg;

 for(egg = egg_list.begin(); egg != egg_list.end();)
   {
    if((*egg)->obj == egg_obj)
       {
        egg_list.remove(*egg);
        delete *egg;
        
        break;
       }

    egg++;
   }

 return;
}

void EggManager::spawn_eggs(uint16 x, uint16 y, uint8 z)
{
 std::list<Egg *>::iterator egg;
 sint16 dist_x, dist_y;
 uint16 i;
 Obj *obj, *spawned_obj;
 U6Link *link;
 uint8 hatch_probability;

 for(egg = egg_list.begin(); egg != egg_list.end();)
   {
    dist_x = abs((sint16)(*egg)->obj->x - x);
    dist_y = abs((sint16)(*egg)->obj->y - y);

    if(dist_x <= 32 && dist_y <= 32 && (*egg)->obj->z == z)
      {
       
       if(((*egg)->obj->status & OBJ_STATUS_EGG_ACTIVE) == 0 || ((*egg)->obj->status & OBJ_STATUS_SEEN_EGG) == 0)
         {
          (*egg)->obj->status |= OBJ_STATUS_SEEN_EGG;
		  (*egg)->obj->status |= OBJ_STATUS_EGG_ACTIVE;
          
          hatch_probability = NUVIE_RAND()%100;
          printf("Checking Egg (%x,%x,%x). Rand: %d Probability: %d%%\n",(*egg)->obj->x, (*egg)->obj->y, (*egg)->obj->z,hatch_probability,(*egg)->obj->qty);
          
          // check random probability that the egg will hatch
          if((*egg)->obj->qty == 100 || hatch_probability <= (*egg)->obj->qty)  // Hatch the egg.
            {
             for(link = (*egg)->obj->container->start(); link != NULL; link = link->next)
               {
                obj = (Obj *)link->data;
                for(i = 0;i < obj->qty;i++)
                 {
				  if(obj->quality != 0) /* spawn temp actor we know it's an actor if it has a non-zero worktype. */
                     actor_manager->create_temp_actor(obj->obj_n,(*egg)->obj->x+i,(*egg)->obj->y,(*egg)->obj->z,obj->quality);
				  else
					{ /* spawn temp object */
					 spawned_obj = new Obj();
					 spawned_obj->obj_n = obj->obj_n;
					 spawned_obj->x = (*egg)->obj->x+i;
					 spawned_obj->y = (*egg)->obj->y;
					 spawned_obj->z = (*egg)->obj->z;
					 spawned_obj->qty = obj->qty;
					 spawned_obj->status |= OBJ_STATUS_TEMPORARY | OBJ_STATUS_OK_TO_TAKE;

					 obj_manager->add_obj(spawned_obj);
					}
                 }
               }
            }   
         }
      }
    else
      (*egg)->obj->status &= (0xff ^ OBJ_STATUS_EGG_ACTIVE);

    egg++;
   }
 
 return;
}
