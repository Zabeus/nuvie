/*
 *  U6Actor.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Jul 27 2003.
 *  Copyright (c) Nuvie Team 2003. All rights reserved.
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
#include <cstdlib>

#include "nuvieDefs.h"
#include "U6LList.h"

#include "Game.h"
#include "U6UseCode.h"
#include "SchedPathFinder.h"
#include "U6AStarPath.h"
#include "MsgScroll.h"
#include "U6Actor.h"

#include "Party.h"
#include "ActorManager.h"
#include "ViewManager.h"
#include "Converse.h"
#include "Effect.h"
#include "CombatPathFinder.h"

#include "U6ActorTypes.h"
#include "U6WorkTypes.h"


U6Actor::U6Actor(Map *m, ObjManager *om, GameClock *c): Actor(m,om,c)
{
 beg_mode = 0; // beggers are waiting for targets
 walk_frame_inc = 1;
 poison_counter = 0;

 foe = attacker = 0;
}

U6Actor::~U6Actor()
{
}

bool U6Actor::init()
{
 Actor::init();
 set_actor_obj_n(obj_n); //set actor_type

 base_actor_type = get_actor_type(base_obj_n);
 
 body_armor_class = base_actor_type->body_armor_class;
 
 if(temp_actor)
   {
    strength = base_actor_type->str;
    dex = base_actor_type->dex;
    intelligence = base_actor_type->intelligence;
    hp = base_actor_type->hp;
   }

 if(alignment == ACTOR_ALIGNMENT_DEFAULT)
   set_alignment(base_actor_type->alignment);
      
 if(actor_type->tile_type == ACTOR_QT && frame_n == 0) //set the two quad tile actors to correct frame number.
   frame_n = 3;

 discover_direction();

 if(has_surrounding_objs())
   clear_surrounding_objs_list(); //clean up the old list if required.

 switch(obj_n) //gather surrounding objects from map if required
  {
   case OBJ_U6_SHIP : init_ship(); break;

   case OBJ_U6_HYDRA : init_hydra(); break;

   case OBJ_U6_DRAGON : init_dragon(); break;

   case OBJ_U6_SILVER_SERPENT : init_silver_serpent(); break;

   case OBJ_U6_GIANT_SCORPION :
   case OBJ_U6_GIANT_ANT :
   case OBJ_U6_COW :
   case OBJ_U6_ALLIGATOR :
   case OBJ_U6_HORSE :
   case OBJ_U6_HORSE_WITH_RIDER : init_splitactor(); break;


   default : break;
  }


 if(actor_type->can_sit) // For some reason U6 starts with actors standing on their chairs.
   {                     // We need to sit them down.
    Obj *obj = obj_manager->get_obj(x,y,z);
    sit_on_chair(obj); // attempt to sit on obj.
   }

 return true;
}

bool U6Actor::init_ship()
{
 Obj *obj;
 uint16 obj1_x, obj1_y, obj2_x, obj2_y;

 obj1_x = x;
 obj1_y = y;
 obj2_x = x;
 obj2_y = y;

 switch(direction)
  {
   case NUVIE_DIR_N : obj1_y = y+1;
                      obj2_y = y-1;
                      break;
   case NUVIE_DIR_E : obj1_x = x+1;
                      obj2_x = x-1;
                      break;
   case NUVIE_DIR_S : obj1_y = y-1;
                      obj2_y = y+1;
                      break;
   case NUVIE_DIR_W : obj1_x = x-1;
                      obj2_x = x+1;
                      break;
  }

 obj = obj_manager->get_obj(obj1_x,obj1_y,z);
 if(obj == NULL)
   return false;
 surrounding_objects.push_back(obj);

 obj = obj_manager->get_obj(obj2_x,obj2_y,z);
 if(obj == NULL)
   return false;
 surrounding_objects.push_back(obj);

 return true;
}

bool U6Actor::init_splitactor()
{
 uint16 obj_x, obj_y;

 obj_x = x;
 obj_y = y;

 switch(direction) //FIX for world wrapping
  {
   case NUVIE_DIR_N : obj_y = y+1;
                      break;
   case NUVIE_DIR_E : obj_x = x-1;
                      break;
   case NUVIE_DIR_S : obj_y = y-1;
                      break;
   case NUVIE_DIR_W : obj_x = x+1;
                      break;
  }

 init_surrounding_obj(obj_x, obj_y, z, obj_n, frame_n + 8); // init back object

 return true;
}

bool U6Actor::init_dragon()
{
 uint16 head_x, head_y, tail_x, tail_y;
 uint16 wing1_x, wing1_y, wing2_x, wing2_y;

 head_x = tail_x = x;
 wing1_x = wing2_x = x;
 head_y = tail_y = y;
 wing1_y = wing2_y = y;

 switch(direction)
  {
   case NUVIE_DIR_N : head_y = y-1;
                      tail_y = y+1;
                      wing1_x = x-1;
                      wing2_x = x+1;
                      break;
   case NUVIE_DIR_E : head_x = x+1;
                      tail_x = x-1;
                      wing1_y = y-1;
                      wing2_y = y+1;
                      break;
   case NUVIE_DIR_S : head_y = y+1;
                      tail_y = y-1;
                      wing1_x = x+1;
                      wing2_x = x-1;
                      break;
   case NUVIE_DIR_W : head_x = x-1;
                      tail_x = x+1;
                      wing1_y = y+1;
                      wing2_y = y-1;
                      break;
  }

 init_surrounding_obj(head_x, head_y, z, obj_n, frame_n + 8);
 init_surrounding_obj(tail_x, tail_y, z, obj_n, frame_n + 16);
 init_surrounding_obj(wing1_x, wing1_y, z, obj_n, frame_n + 24);
 init_surrounding_obj(wing2_x, wing2_y, z, obj_n, frame_n + 32);

 return true;
}

bool U6Actor::init_hydra()
{
 // For some reason a Hydra has a different object number for its tenticles. :-(

 init_surrounding_obj(x,   y-1, z, OBJ_U6_HYDRA_BODY, 0);
 init_surrounding_obj(x+1, y-1, z, OBJ_U6_HYDRA_BODY, 4);
 init_surrounding_obj(x+1, y, z, OBJ_U6_HYDRA_BODY, 8);
 init_surrounding_obj(x+1, y+1, z, OBJ_U6_HYDRA_BODY, 12);
 init_surrounding_obj(x,   y+1, z, OBJ_U6_HYDRA_BODY, 16);
 init_surrounding_obj(x-1, y+1, z, OBJ_U6_HYDRA_BODY, 20);
 init_surrounding_obj(x-1, y, z, OBJ_U6_HYDRA_BODY, 24);
 init_surrounding_obj(x-1, y-1, z, OBJ_U6_HYDRA_BODY, 28);

 return true;
}

bool U6Actor::init_silver_serpent()
{
 uint16 sx, sy, sz;
 Obj *obj;
 uint8 tmp_frame_n=0;
 
 sx = x;
 sy = y;
 sz = z;
 
 switch(direction)
 {
   case NUVIE_DIR_N : sy++;
                      tmp_frame_n = 1;
                      break;
   case NUVIE_DIR_E : sx--;
                      tmp_frame_n = 3;
                      break;
   case NUVIE_DIR_S : sy--;
                      tmp_frame_n = 5;
                      break;
   case NUVIE_DIR_W : sx++;
                      tmp_frame_n = 7;
                      break;
 }

 obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_SILVER_SERPENT, 1, id_n, sx, sy, sz);

 if(obj != NULL) //old snake
  gather_snake_objs_from_map(obj, x, y, z);
 else //new snake
  { //FIXME we need to make long, randomly layed out snakes here!
   init_surrounding_obj(sx, sy, sz, OBJ_U6_SILVER_SERPENT, tmp_frame_n);
  }
 
 return true;
}

void U6Actor::gather_snake_objs_from_map(Obj *start_obj, uint16 ax, uint16 ay, uint16 az)
{
 Obj *obj;
 uint16 px, py, pz;
 uint16 nx, ny, nz;
 uint8 seg_num;
 
 px = ax;
 py = ay;
 pz = az;
 
 obj = start_obj;
 surrounding_objects.push_back(obj);

 for(seg_num = 2;obj && obj->frame_n >= 8;seg_num++)
  {

   nx = obj->x;
   ny = obj->y;
   nz = obj->z;
   //work out the location of the next obj based on the current frame_n and relative movement.
   switch(obj->frame_n)
    {
     //up down
     case  8 : if(ny - 1 == py)
                 ny++;
              else
                 ny--;
               break;
     //left right
     case  9 : if(nx - 1 == px)
                 nx++;
               else
                 nx--;
               break;
     //up right
     case 10 : if(ny - 1 == py)
                 nx++;
               else
                 ny--;
               break;
     //down right
     case 11 : if(ny + 1 == py)
                 nx++;
               else
                 ny++;
               break;
     //left down
     case 12 : if(nx - 1 == px)
                 ny++;
               else
                 nx--;
               break;
     //left up
     case 13 : if(nx - 1 == px)
                 ny--;
               else
                 nx--;
               break;
    }
    
    px = obj->x;
    py = obj->y;
    pz = obj->z;

    obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_SILVER_SERPENT, seg_num, id_n, nx, ny, nz);

    if(obj)
      surrounding_objects.push_back(obj);
  }
     
}

uint16 U6Actor::get_downward_facing_tile_num()
{
 uint8 shift = 0;

 if(base_actor_type->frames_per_direction > 1) //we want the second frame for most actor types.
   shift = 1;

 return obj_manager->get_obj_tile_num(base_actor_type->base_obj_n) + base_actor_type->tile_start_offset + (NUVIE_DIR_S * base_actor_type->tiles_per_direction + base_actor_type->tiles_per_frame - 1) + shift;
}

void U6Actor::update()
{
// Party *party = Game::get_game()->get_party();
 if(!alive)  //we don't need to update dead actors.
   return;

 Actor::update();
// moved to Party::follow(), but I think it should be here
// if(in_party && !party->is_in_combat_mode() && party->get_actor(party->get_leader()) != this)
//  set_worktype(WORKTYPE_U6_IN_PARTY); // revert to normal worktype

 preform_worktype();

 if(is_poisoned())
  updatePoison();

 return;
}

bool U6Actor::updateSchedule(uint8 hour)
{
 bool ret;
 if((ret = Actor::updateSchedule(hour)) == true) //walk to next schedule location if required.
   {
    if(sched[sched_pos] != NULL && (sched[sched_pos]->x != x || sched[sched_pos]->y != y || sched[sched_pos]->z != z))
       set_worktype(WORKTYPE_U6_WALK_TO_LOCATION);
   }

 return ret;
}

inline void U6Actor::updatePoison()
{
 if(poison_counter == 0)
  {
   poison_counter = NUVIE_RAND() % 35;
   hit(1, ACTOR_FORCE_HIT);
  }
 else
   poison_counter--;
}

// workout our direction based on actor_type and frame_n
inline void U6Actor::discover_direction()
{
 if(actor_type->frames_per_direction != 0)
   direction = (frame_n - actor_type->tile_start_offset ) / actor_type->tiles_per_direction;
 else
   direction = NUVIE_DIR_S;
}

void U6Actor::set_direction(uint8 d)
{
 uint8 frames_per_dir = (actor_type->frames_per_direction != 0)
                         ? actor_type->frames_per_direction : 4;
 if(d >= 4)
   return;

 if(walk_frame == 0)
   walk_frame_inc = 1; // loop forward
 else if(walk_frame == (frames_per_dir - 1))
   walk_frame_inc = -1; // loop backward
 walk_frame = (walk_frame + walk_frame_inc)%frames_per_dir;

 if(has_surrounding_objs())
   {
    if(direction != d)
      set_direction_of_surrounding_objs(d);
    else
     {
      if(can_move && actor_type->twitch_rand) //only twitch actors with a non zero twitch_rand.
        twitch_surrounding_objs();
     }
   }

 direction = d;

 //only change direction frame if the actor can twitch ie isn't sitting or in bed etc.
 if(can_move)
   frame_n = actor_type->tile_start_offset + (direction * actor_type->tiles_per_direction +
             (walk_frame * actor_type->tiles_per_frame ) + actor_type->tiles_per_frame - 1);

}

void U6Actor::face_location(uint16 lx, uint16 ly)
{
 if(obj_n != OBJ_U6_SILVER_SERPENT) //snakes cannot turn on the spot.
   Actor::face_location(lx, ly);

 return;
}

void U6Actor::clear()
{
 if(has_surrounding_objs())
  {
   remove_surrounding_objs_from_map();
   clear_surrounding_objs_list(REMOVE_SURROUNDING_OBJS);
  }

 Actor::clear();

 return;
}



bool U6Actor::move(sint16 new_x, sint16 new_y, sint8 new_z, ActorMoveFlags flags)
{
// bool force_move = flags & ACTOR_FORCE_MOVE;
 bool ret;
 sint16 rel_x, rel_y;
 MsgScroll *scroll = Game::get_game()->get_scroll();
 Player *player = Game::get_game()->get_player();
 Party *party = player->get_party();
 MapCoord old_pos = get_location();

 if(has_surrounding_objs())
   remove_surrounding_objs_from_map();

 rel_x = new_x - x;
 rel_y = new_y - y;

 if(flags&ACTOR_OPEN_DOORS && worktype != WORKTYPE_U6_WALK_TO_LOCATION)
    flags^=ACTOR_OPEN_DOORS; // only use doors when walking to schedule location
 ret = Actor::move(new_x,new_y,new_z,flags);

 if(ret == true)
  {
   if(has_surrounding_objs())
      move_surrounding_objs_relative(rel_x, rel_y);

   Obj *obj = obj_manager->get_obj(new_x,new_y,new_z); // Ouch, we get obj in Actor::move() too :(
   if(obj)
     {
      if(actor_type->can_sit)
        {
         sit_on_chair(obj); // make the Actor sit if they are on top of a chair.
        }

      if(obj->obj_n == OBJ_U6_FIRE_FIELD) // ouch
        {
         hit(5); // -?? hp?
         scroll->display_string("\n");
         scroll->display_prompt();
        }
      if(obj->obj_n == OBJ_U6_POISON_FIELD && !is_poisoned()) // ick
        {
         set_poisoned(true);
         
         if(in_party)
           {
            scroll->display_string(party->get_actor_name(party->get_member_num(this)));
            scroll->display_string(" poisoned!\n\n");
            scroll->display_prompt();
           }
        }
      if(obj->obj_n == OBJ_U6_SLEEP_FIELD) // Zzz
        {
         new HitEffect(this); // no hp loss
         //fall asleep (change worktype?)
         if(in_party)
            scroll->message("Zzz...\n");
        }

      if(obj->obj_n == OBJ_U6_TRAP)
        {
         hit(25); //FIXME find proper amount.
         obj->status &= (0xff ^ OBJ_STATUS_INVISIBLE); //show trap. FIXME should this logic go else ware.
        }

      if(obj->obj_n == OBJ_U6_SPIKES)
        {
         hit(NUVIE_RAND()%8+1); // I think this is the proper amount. (SB-X)
        }
     }

     // temp. fix; this too should be done with UseCode (and don't move the mirror)
     if(old_pos.y > 0 && new_y > 0)
     {
         Obj *old_mirror = obj_manager->get_obj_of_type_from_location(OBJ_U6_MIRROR,old_pos.x,old_pos.y-1,old_pos.z);
         Obj *mirror = obj_manager->get_obj_of_type_from_location(OBJ_U6_MIRROR,new_x,new_y-1,new_z);
         if(old_mirror) old_mirror->frame_n = 0;
         if(mirror)     mirror->frame_n = 1;
     }

   // Cyclops: shake ground if player is near
   if(actor_type->base_obj_n == OBJ_U6_CYCLOPS && is_nearby(player->get_actor()))
      new QuakeEffect(1, 200, player->get_actor());
  }

 if(has_surrounding_objs()) //add our surrounding objects back onto the map.
   add_surrounding_objs_to_map();

 return ret;
}

bool U6Actor::check_move(sint16 new_x, sint16 new_y, sint8 new_z, ActorMoveFlags flags)
{
// bool ignore_actors = flags & ACTOR_IGNORE_OTHERS;
 Tile *map_tile;

 if(Actor::check_move(new_x, new_y, new_z, flags) == false)
    return false;

 if(obj_n == OBJ_U6_SILVER_SERPENT && check_move_silver_serpent(new_x, new_y) == false)
   return false;
   
    switch(actor_type->movetype)
      {
       case MOVETYPE_U6_WATER_HIGH : // for HIGH we only want to move to open water.
                                     // No shorelines.
                                     map_tile = map->get_tile(new_x, new_y, new_z, MAP_ORIGINAL_TILE);
                                     if(map_tile->tile_num >= 16 && map_tile->tile_num <= 47)
                                       return false;
                                    //fall through to MOVETYPE_U6_WATER_LOW
       case MOVETYPE_U6_WATER_LOW : if(!map->is_water(new_x, new_y, new_z))
                                       return false;
                                    break;

       case MOVETYPE_U6_AIR_LOW : map_tile = map->get_tile(new_x, new_y, new_z, MAP_ORIGINAL_TILE);
                                  if(map_tile->flags1 & TILEFLAG_WALL) //low air boundry
                                    return false;

                                  map_tile = obj_manager->get_obj_tile(new_x, new_y, new_z, false);
                                  if(map_tile && (map_tile->flags1 & TILEFLAG_WALL ||
                                     (map_tile->flags2 & (TILEFLAG_DOUBLE_WIDTH | TILEFLAG_DOUBLE_HEIGHT)) == (TILEFLAG_DOUBLE_WIDTH | TILEFLAG_DOUBLE_HEIGHT)))
                                    return false;
                                  break;

                                  //fall through to MOVETYPE_U6_AIR_HIGH
       case MOVETYPE_U6_AIR_HIGH : if(map->is_boundary(new_x, new_y, new_z))
                                    return false; //FIX for proper air boundary
                                  break;
       case MOVETYPE_U6_LAND :
       default : if(map->is_passable(new_x,new_y,new_z) == false)
       {
                    if(obj_n == OBJ_U6_MOUSE // try to go through mousehole
                       && obj_manager->get_obj_of_type_from_location(OBJ_U6_MOUSEHOLE,new_x,new_y,new_z) != NULL)
                       return(true);
                    if(obj_n == OBJ_U6_SILVER_SERPENT //silver serpents can crossover themselves
                       && obj_manager->get_obj_of_type_from_location(OBJ_U6_SILVER_SERPENT,new_x,new_y,new_z) != NULL)
                       return(true);
                       
                    return false;
       }                 

      }

 return(true);
}

bool U6Actor::check_move_silver_serpent(uint16 new_x, uint16 new_y)
{
 if(new_x != x && new_y != y) //snakes can't move diagonally
  return false;

 Obj *obj = (Obj *)surrounding_objects.front(); //retrieve the first body segment.
 
 if(obj->x == new_x && obj->y == new_y) //snakes can't move backwards.
  return false;
  
 return true;
}

// attempt to sit if obj is a chair.

bool U6Actor::sit_on_chair(Obj *obj)
{
   if(actor_type->can_sit && obj)
     {
         if(obj->obj_n == OBJ_U6_CHAIR)  // make the actor sit on a chair.
           {
            frame_n = (obj->frame_n * 4) + 3;
            direction = obj->frame_n;
            can_move = false;
            return true;
           }

         if(obj->obj_n == OBJ_U6_THRONE  && obj->frame_n == 3) //make actor sit on LB's throne.
           {
            frame_n = 8 + 3; //sitting facing south.
            direction = NUVIE_DIR_S;
            can_move = false;
            return true;
           }
     }

 return false;
}

uint8 U6Actor::get_object_readiable_location(uint16 obj_n)
{
 uint16 i;

 for(i=0;readiable_objects[i].obj_n != OBJ_U6_NOTHING;i++)
   {
    if(obj_n == readiable_objects[i].obj_n)
      return readiable_objects[i].readiable_location;
   }

 return ACTOR_NOT_READIABLE;
}

const CombatType *U6Actor::get_object_combat_type(uint16 obj_n)
{
 uint16 i;

 for(i=0;u6combat_objects[i].obj_n != OBJ_U6_NOTHING;i++)
   {
    if(obj_n == u6combat_objects[i].obj_n)
      return &u6combat_objects[i];
   }

 return NULL;
}

const CombatType *U6Actor::get_hand_combat_type()
{
 return &u6combat_hand;
}

bool U6Actor::weapon_can_hit(const CombatType *weapon, uint16 target_x, uint16 target_y)
{
 sint16 off_x, off_y;

 if(!weapon)
   return false;

 off_x = x - target_x;
 off_y = y - target_y;
 
 if(abs(off_x) > 5 || abs(off_y) > 5)
   return false;
  
 if(weapon->hit_range == 0)
   return true;

 return (bool)u6combat_hitrange_tbl[weapon->hit_range - 1][(5 + off_y) * 11 + (5 + off_x)];
}

void U6Actor::twitch()
{

 if(can_move == false || actor_type->twitch_rand == 0)
   return;

 if(NUVIE_RAND()%actor_type->twitch_rand == 1)
  {
   if(actor_type->frames_per_direction == 0)
     walk_frame = (walk_frame + 1) % 4;
   else
     walk_frame = NUVIE_RAND()%actor_type->frames_per_direction;

   if(has_surrounding_objs())
    {
	 switch(obj_n)
	   {
		case OBJ_U6_HYDRA : twitch_surrounding_hydra_objs(); break;
		case OBJ_U6_DRAGON :
		default : twitch_surrounding_objs(); break;
	   }
	}

   frame_n = actor_type->tile_start_offset + (direction * actor_type->tiles_per_direction + (walk_frame * actor_type->tiles_per_frame)  + actor_type->tiles_per_frame - 1);
  }

 return;
}

void U6Actor::set_poisoned(bool poisoned)
{
 if(poisoned)
  {
   status_flags |= ACTOR_STATUS_POISONED;
   new HitEffect(this); // no direct hp loss
   Game::get_game()->get_view_manager()->update();
  }
 else
  {
   status_flags &= (0xff ^ ACTOR_STATUS_POISONED);
   poison_counter = 0;
  }
}

void U6Actor::preform_worktype()
{
 switch(worktype)
  {
//   case WORKTYPE_U6_IN_PARTY : wt_party(); break;
   case WORKTYPE_U6_PLAYER : wt_player(); break;
   case WORKTYPE_U6_COMBAT_FRONT :
   case WORKTYPE_U6_COMBAT_REAR :
   case WORKTYPE_U6_COMBAT_FLANK :
   case WORKTYPE_U6_COMBAT_BERSERK :
   case WORKTYPE_U6_COMBAT_RETREAT :
   case WORKTYPE_U6_COMBAT_ASSAULT : // WILD
   case WORKTYPE_U6_COMBAT_SHY :
   case WORKTYPE_U6_COMBAT_LIKE :
//   case WORKTYPE_U6_COMBAT_IMMOBILE : immobile actors can fight, but wt_combat can't handle them yet
   case WORKTYPE_U6_COMBAT_UNFRIENDLY : wt_combat(); break;
   case WORKTYPE_U6_FACE_NORTH :
   case WORKTYPE_U6_FACE_EAST  :
   case WORKTYPE_U6_FACE_SOUTH :
   case WORKTYPE_U6_FACE_WEST  :
     break;
   case WORKTYPE_U6_WALK_TO_LOCATION : wt_walk_to_location();
                                      break;

   case WORKTYPE_U6_GUARD_WALK_NORTH_SOUTH :
   case WORKTYPE_U6_GUARD_WALK_EAST_WEST   :
   case WORKTYPE_U6_WALK_NORTH_SOUTH :
   case WORKTYPE_U6_WALK_EAST_WEST   : wt_walk_straight(); break;

   case WORKTYPE_U6_WORK :
   case WORKTYPE_U6_WANDER_AROUND   : wt_wander_around(); break;
   case WORKTYPE_U6_ANIMAL_WANDER : wt_farm_animal_wander(); break;
   case WORKTYPE_U6_BEG : wt_beg(); break;
//   case WORKTYPE_U6_
//                     break;
  }

 return;
}

void U6Actor::set_worktype(uint8 new_worktype)
{
 if(new_worktype == worktype)
   return;

 //reset to base obj_n
 if(worktype > 2) //don't revert for party worktypes as they might be riding a horse.
   set_actor_obj_n(base_actor_type->base_obj_n);

 if(worktype == WORKTYPE_U6_SLEEP || worktype == WORKTYPE_U6_PLAY_LUTE)
   {
    frame_n = old_frame_n;
   }
 Actor::set_worktype(new_worktype);

 //FIX from here.

 switch(worktype)
  {
   case WORKTYPE_U6_FACE_NORTH : set_direction(NUVIE_DIR_N); break;
   case WORKTYPE_U6_FACE_EAST  : set_direction(NUVIE_DIR_E); break;
   case WORKTYPE_U6_FACE_SOUTH : set_direction(NUVIE_DIR_S); break;
   case WORKTYPE_U6_FACE_WEST  : set_direction(NUVIE_DIR_W); break;

   case WORKTYPE_U6_SLEEP : wt_sleep(); break;
   case WORKTYPE_U6_PLAY_LUTE : wt_play_lute(); break;
  }
}


void U6Actor::wt_walk_to_location()
{
    if(sched[sched_pos] != NULL)
    {
        if(x == sched[sched_pos]->x && y == sched[sched_pos]->y
                                    && z == sched[sched_pos]->z)
        {
            set_worktype(sched[sched_pos]->worktype);
            delete_pathfinder();
            return;
        }
        if(!pathfinder)
        {
            work_location.x = sched[sched_pos]->x;
            work_location.y = sched[sched_pos]->y;
            work_location.z = sched[sched_pos]->z;
//            if(!work_location.is_visible() || !get_location().is_visible())
//                set_pathfinder(new OffScreenPathFinder(this, work_location, new U6AStarPath));
//            else
                set_pathfinder(new SchedPathFinder(this, work_location, new U6AStarPath));
        }
    }
}


void U6Actor::wt_walk_straight()
{
 uint8 dir = get_direction();
// set_direction(dir); //update walk frame FIX this!
 if(NUVIE_RAND()%2) // sometimes guards do stop, even if they can move (SB-X)
   {
 if(worktype == WORKTYPE_U6_WALK_NORTH_SOUTH || worktype == WORKTYPE_U6_GUARD_WALK_NORTH_SOUTH)
   {
    if(dir == NUVIE_DIR_N) // move up if blocked face down
       {
        if(moveRelative(0,-1) == false)
          set_direction(NUVIE_DIR_S);
        else
          set_direction(NUVIE_DIR_N);
       }
    else // move down if blocked face up
       {
        if(moveRelative(0,1) == false)
          set_direction(NUVIE_DIR_N);
        else
          set_direction(NUVIE_DIR_S);
       }
   }
 else //WORKTYPE_U6_WALK_EAST_WEST, WORKTYPE_U6_GUARD_WALK_EAST_WEST
   {
    if(dir == NUVIE_DIR_W) //move left if blocked face right
       {
        if(moveRelative(-1,0) == false)
          set_direction(NUVIE_DIR_E);
        else
          set_direction(NUVIE_DIR_W);
       }
    else  //move right if blocked face left
       {
        if(moveRelative(1,0) == false)
          set_direction(NUVIE_DIR_W);
        else
          set_direction(NUVIE_DIR_E);
       }
   }
   } else set_moves_left(moves - 5); // FIXME: might use tile cost
}

// Loitering/Working actors stay close to their schedule location. (SB-X)
void U6Actor::wt_wander_around()
{
 const int area = (worktype == WORKTYPE_U6_WORK) ? 1 : 8;
 uint8 new_direction;
 sint32 xdist = 0, ydist = 0;

 if(work_location.x || work_location.y)
   {
    xdist = (sint32)x - work_location.x;
    ydist = (sint32)y - work_location.y;
   }
 if(NUVIE_RAND()%8 == 1)
   {
    new_direction = NUVIE_RAND()%4;
    switch(new_direction)
      {
       case 0 : if(ydist > -area) if(moveRelative(0,-1)) set_direction(new_direction); break;
       case 1 : if(xdist < +area) if(moveRelative(1,0)) set_direction(new_direction); break;
       case 2 : if(ydist < +area) if(moveRelative(0,1)) set_direction(new_direction); break;
       case 3 : if(xdist > -area) if(moveRelative(-1,0)) set_direction(new_direction); break;
      }
   }
 else set_moves_left(moves - 5);

 return;
}

// wander around but don't cross boundaries or fences. Used for cows and horses.
// FIX: once we've tested that hazards are working properly, this won't be needed
void U6Actor::wt_farm_animal_wander()
{
 uint8 new_direction;
 sint8 rel_x = 0, rel_y = 0;

 if(NUVIE_RAND()%8 == 1)
   {
    new_direction = NUVIE_RAND()%4;

    switch(new_direction)
      {
       case NUVIE_DIR_N : rel_y = -1; break;
       case NUVIE_DIR_E : rel_x = 1; break;
       case NUVIE_DIR_S : rel_y = 1; break;
       case NUVIE_DIR_W : rel_x = -1; break;
      }

    if(obj_manager->get_obj_of_type_from_location(OBJ_U6_FENCE,x + rel_x, y + rel_y, z) == NULL)
        {
         if(moveRelative(rel_x,rel_y))
            set_direction(new_direction);
        }

   }
 else set_moves_left(moves - 5);

 return;
}


/* Wander around, approach and talk to the player character if visible.
 * Beg modes: 0 = waiting for target, 1 = following, 2 = just loiter
 * FIXME: This is really done using 2 worktypes. (CONV, then LOIT)
 */
void U6Actor::wt_beg()
{
    Player *player = Game::get_game()->get_player();
    Actor *actor = player->get_actor();
    uint32 dist_from_start = 0;
    if(work_location.x || work_location.y)
        dist_from_start = get_location().distance(work_location);

    if(beg_mode == 0)
        if(is_nearby(actor)) // look for victi... er, player
            beg_mode = 1;
    if(beg_mode == 1)
    {
        beg_mode = is_nearby(actor) ? 1 : 0; // still visible?
        Party *party = player->get_party();
        MapCoord me(x,y,z), them(0,0,0);
        for(uint32 p = 0; p < party->get_party_size(); p++)
        {
            party->get_actor(p)->get_location(&them.x, &them.y, &them.z);
            if(me.distance(them) <= 1 && z == them.z)
            {
                // talk to me :)
                delete_pathfinder();
                // FIXME: this check should be in Converse
                if(!player->in_party_mode())
                {
                    MsgScroll *scroll = Game::get_game()->get_scroll();
                    scroll->display_string("\nNot in solo mode.\n");
                    scroll->display_string("\n");
                    scroll->display_prompt();
                }
                else if(Game::get_game()->get_converse()->start(this))
                {
                    actor->face_actor(this);
                    face_actor(actor);
                }
                beg_mode = 2;
                return; // done
            }
        }
        // get closer
        if(dist_from_start < 4)
            attract_to(actor);
        else
            pathfind_to(work_location); // U6 probably doesn't even do this
    }
    else
        wt_wander_around();
}


void U6Actor::wt_sleep()
{
 Obj *obj = obj_manager->get_obj(x,y,z);

 can_move = false;
 status_flags |= ACTOR_STATUS_ASLEEP;
 if(obj)
   {
    if(obj->obj_n == OBJ_U6_BED)
      {
       if(obj->frame_n == 1 || obj->frame_n == 5) //horizontal bed
         {
          old_frame_n = frame_n;
          obj_n = OBJ_U6_PERSON_SLEEPING;
          frame_n = 0;
         }
       if(obj->frame_n == 7 || obj->frame_n == 10) //vertical bed
         {
          old_frame_n = frame_n;
          obj_n = OBJ_U6_PERSON_SLEEPING;
          frame_n = 1;
         }
       return;
      }
   }

 // lay down on the ground using the dead body frame
 if(actor_type->can_laydown)
   {
    old_frame_n = frame_n;
    obj_n = actor_type->dead_obj_n;
    frame_n = actor_type->dead_frame_n;
   }

}

void U6Actor::wt_play_lute()
{
 old_frame_n = frame_n;

 set_actor_obj_n(OBJ_U6_MUSICIAN_PLAYING);

 frame_n = direction * actor_type->tiles_per_direction;

 return;
}

// Combat worktypes/strategies
// Monsters are always in "combat-mode", but that isn't apparent
// unless an enemy is nearby.
void U6Actor::wt_combat()
{
    bool attacking = false;
    ActorManager *actor_mgr = Game::get_game()->get_actor_manager();
    Player *player = Game::get_game()->get_player();
    Actor *pactor = player->get_actor();
    MapCoord ploc = pactor->get_location();

// FIXME: This needs to be replaced with special cases for each worktype.
//        Player combat modes are especially more involved. It should also call
//        specific methods for different monster types. (that do various idle and attack actions)

    if(NUVIE_RAND()%4 != 0) // only move 1/4th of the time; looks closer to U6
    {
        set_moves_left(moves-10);
        return;
    }

    // retreat
    if(worktype == WORKTYPE_U6_COMBAT_SHY
       || worktype == WORKTYPE_U6_COMBAT_RETREAT)
    {
        ActorList *actors = in_party?find_enemies():find_players();
        if(actors)
        {
printf("%s yells \"Aiee!", get_name());
            actor_mgr->sort_nearest(actors,x,y,z);
            if(worktype == WORKTYPE_U6_COMBAT_RETREAT)
            {
printf(" %s is trying to get me!\"", actors->front()->get_name());
                repel_from(actors->front());
            }
            else // shy creatures only flee if very close
            {
                ActorIterator a = actors->begin();
                while(a != actors->end())
                {
                    pactor = *a;
                    ploc = pactor->get_location();
                    if(is_nearby(ploc, 2))
                    {
printf(" %s is trying to get me!", pactor->get_name());
                        repel_from(pactor);
                        break;
                    }
                    ++a;
                }
            }
            delete actors;
printf("\"\n");
        }
    }
    // follow
    if(worktype == WORKTYPE_U6_COMBAT_LIKE)
    {
        ActorList *actors = find_players();
        if(actors)
        {
            actor_mgr->sort_nearest(actors,x,y,z);
            if(worktype == WORKTYPE_U6_COMBAT_LIKE && !pathfinder)
                attract_to(actors->front());
            delete actors;
        }
    }
    // turn wild if near player
    if(worktype == WORKTYPE_U6_COMBAT_UNFRIENDLY)
    {
printf("%s looks threatening.\n", get_name());
        ActorList *actors = find_players();
        if(actors)
        {
            delete actors;
            if(NUVIE_RAND()%4)
                set_worktype(WORKTYPE_U6_COMBAT_ASSAULT);
        }
    }
    // attack or chase enemies
    if(worktype == WORKTYPE_U6_COMBAT_FRONT
       || worktype == WORKTYPE_U6_COMBAT_REAR
       || worktype == WORKTYPE_U6_COMBAT_FLANK
       || worktype == WORKTYPE_U6_COMBAT_BERSERK
       || worktype == WORKTYPE_U6_COMBAT_ASSAULT
       || worktype == WORKTYPE_U6_COMBAT_REAR)
    {
printf("%s is looking for trouble", get_name());
        ActorList *actors = find_enemies();
        if(actors)
        {
printf(", and finds someone to fight!\n");
            if(worktype == WORKTYPE_U6_COMBAT_BERSERK)
                sort(actors->begin(), actors->end(), cmp_level());
            else
                actor_mgr->sort_nearest(actors,x,y,z);
            attacking = combat_try_attack(actors);
            if(!attacking && !pathfinder)
            {
                foe = (U6Actor*)actors->front();
                attract_to(actors->front());
            }
            delete actors;
        }
else printf(".\n");
    }

    if(!attacking && !pathfinder)
        wt_wander_around();
}

// switch to the next person
void U6Actor::wt_player()
{
    Player *player = Game::get_game()->get_player();
    if(player->get_actor()->get_actor_num() != id_n)
    {
        delete_pathfinder();
        player->update_player(this);
    }

    // else do player update, raft, balloon, etc
    printf("U6Actor: Player (%d)\n", id_n);
}

// Returns true if the enemy could be attacked. If false is returned, the actor
// probably needs to be closer. (or use a long-range weapon)
bool U6Actor::combat_try_attack(U6Actor *enemy)
{
    if(weapon_can_hit(get_weapon(ACTOR_NO_READIABLE_LOCATION), enemy->x,enemy->y))
    {
        face_actor(enemy);
        attack(ACTOR_NO_READIABLE_LOCATION, enemy);
        foe = enemy;
        return true;
    }
    return false;
}

// Try to attack anyone in the enemies list, which must not be empty.
bool U6Actor::combat_try_attack(ActorList *enemies)
{
    ActorIterator a = enemies->begin();
    do
    {
        if(combat_try_attack((U6Actor*)*a))
            return true;
    } while(++a != enemies->end());
    return false;
}

void U6Actor::set_actor_obj_n(uint16 new_obj_n)
{
 old_frame_n = frame_n;

 obj_n = new_obj_n;
 actor_type = get_actor_type(new_obj_n);

 return;
}

inline const U6ActorType *U6Actor::get_actor_type(uint16 new_obj_n)
{
 const U6ActorType *type;

 for(type = u6ActorTypes; type->base_obj_n != OBJ_U6_NOTHING; type++)
  {
   if(type->base_obj_n == new_obj_n)
     break;
  }

 return type;
}

inline bool U6Actor::has_surrounding_objs()
{
 if(actor_type->tile_type == ACTOR_DT || actor_type->tile_type == ACTOR_MT)
   return true;

 return false;
}

inline void U6Actor::remove_surrounding_objs_from_map()
{
 std::list<Obj *>::iterator obj;

 for(obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++)
    obj_manager->remove_obj((*obj));

 return;
}

inline void U6Actor::add_surrounding_objs_to_map()
{
 std::list<Obj *>::reverse_iterator obj;

 for(obj = surrounding_objects.rbegin(); obj != surrounding_objects.rend(); obj++)
    obj_manager->add_obj((*obj),OBJ_ADD_TOP);

 return;
}

inline void U6Actor::move_surrounding_objs_relative(sint16 rel_x, sint16 rel_y)
{
 std::list<Obj *>::iterator obj;
 
 if(obj_n == OBJ_U6_SILVER_SERPENT)
  {
   move_silver_serpent_objs_relative(rel_x, rel_y);
  }
 else
  { 
   for(obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++)
    {
      (*obj)->x += rel_x;
      (*obj)->y += rel_y;
    }
  }

 return;
}

inline void U6Actor::move_silver_serpent_objs_relative(sint16 rel_x, sint16 rel_y)
{
 std::list<Obj *>::iterator obj;
 uint8 old_frame_n;
 uint8 tmp_frame_n;
 uint16 old_x, old_y;
 uint16 tmp_x, tmp_y;
 sint8 new_pos;
 sint8 old_pos;

 const uint8 new_frame_n_tbl[5][5] = 
 {{ 8,10, 0,13, 0},
  {12, 9, 0, 0,13},
  { 0, 0, 0, 0, 0},
  {11, 0, 0, 9,10},
  { 0,11, 0,12, 8}};
 
  const uint8 new_tail_frame_n_tbl[8][6] =
 {{0,0,0,0,0,0},
  {1,0,0,3,7,0},
  {0,0,0,0,0,0},
  {0,3,0,0,5,1},
  {0,0,0,0,0,0},
  {5,0,3,0,0,7},
  {0,0,0,0,0,0},
  {0,7,1,5,0,0}};

 if(surrounding_objects.empty())
   return;

 obj = surrounding_objects.begin();

 new_pos = 2 + rel_x + (rel_y * 2);

 old_x = (*obj)->x;
 old_y = (*obj)->y;

 (*obj)->x = x - rel_x; // old actor x
 (*obj)->y = y - rel_y; // old actor y
 
 old_pos = 2 + ((*obj)->x - old_x) + (((*obj)->y - old_y) * 2);
 
 old_frame_n = (*obj)->frame_n;
 (*obj)->frame_n = new_frame_n_tbl[new_pos][old_pos];
 obj++; 
 for(;obj != surrounding_objects.end(); obj++)
  {
    tmp_x = (*obj)->x;
    tmp_y = (*obj)->y;
    tmp_frame_n = (*obj)->frame_n;

    (*obj)->x = old_x;
    (*obj)->y = old_y;

    if(tmp_frame_n < 8) //tail, work out new tail direction
      (*obj)->frame_n = new_tail_frame_n_tbl[tmp_frame_n][old_frame_n-8];
    else
      (*obj)->frame_n = old_frame_n;

    old_x = tmp_x;
    old_y = tmp_y;
    old_frame_n = tmp_frame_n;
  }

 return;
}


inline void U6Actor::set_direction_of_surrounding_objs(uint8 new_direction)
{
 remove_surrounding_objs_from_map();

 switch(obj_n)
   {
    case OBJ_U6_SHIP : set_direction_of_surrounding_ship_objs(new_direction); break;

    case OBJ_U6_GIANT_SCORPION :
    case OBJ_U6_GIANT_ANT :
    case OBJ_U6_COW :
    case OBJ_U6_ALLIGATOR :
    case OBJ_U6_HORSE :
    case OBJ_U6_HORSE_WITH_RIDER : set_direction_of_surrounding_splitactor_objs(new_direction); break;

	case OBJ_U6_DRAGON : set_direction_of_surrounding_dragon_objs(new_direction); break;
   }

 add_surrounding_objs_to_map();

 return;
}

inline void U6Actor::set_direction_of_surrounding_ship_objs(uint8 new_direction)
{
 std::list<Obj *>::iterator obj;
 uint16 pitch = map->get_width(z);

 obj = surrounding_objects.begin();
 if(obj == surrounding_objects.end())
  return;

 (*obj)->x = x;
 (*obj)->y = y;

 (*obj)->frame_n =  new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1;
 switch(new_direction)
  {
   case NUVIE_DIR_N : if(y == 0)
                        (*obj)->y = pitch - 1;
                      else
                        (*obj)->y = y - 1;
                      break;

   case NUVIE_DIR_E : if(x == pitch - 1)
                        (*obj)->x = 0;
                      else
                        (*obj)->x = x + 1;
                      break;

   case NUVIE_DIR_S : if(y == pitch - 1)
                        (*obj)->y = 0;
                      else
                        (*obj)->y = y + 1;
                      break;

   case NUVIE_DIR_W : if(x == 0)
                        (*obj)->x = pitch - 1;
                      else
                        (*obj)->x = x - 1;
                      break;
  }

 obj++;
 if(obj == surrounding_objects.end())
  return;

 (*obj)->x = x;
 (*obj)->y = y;

 (*obj)->frame_n =  16 + (new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1);
 switch(new_direction)
  {
   case NUVIE_DIR_N : if(y == pitch - 1)
                        (*obj)->y = 0;
                      else
                        (*obj)->y = y + 1;
                      break;

   case NUVIE_DIR_E : if(x == 0)
                        (*obj)->x = pitch - 1;
                      else
                        (*obj)->x = x - 1;
                      break;

   case NUVIE_DIR_S : if(y == 0)
                        (*obj)->y = pitch - 1;
                      else
                        (*obj)->y = y - 1;
                      break;

   case NUVIE_DIR_W : if(x == pitch - 1)
                        (*obj)->x = 0;
                      else
                        (*obj)->x = x + 1;
                      break;
  }

}

inline void U6Actor::set_direction_of_surrounding_splitactor_objs(uint8 new_direction)
{
 Obj *obj;
 uint16 pitch = map->get_width(z);

 if(surrounding_objects.empty())
   return;

 obj = surrounding_objects.back();

 obj->frame_n =  8 + (new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1);

 obj->x = x;
 obj->y = y;

 switch(new_direction)
  {
   case NUVIE_DIR_N : if(y == pitch - 1)
                        obj->y = 0;
                      else
                        obj->y = y + 1;
                      break;

   case NUVIE_DIR_E : if(x == 0)
                        obj->x = pitch - 1;
                      else
                        obj->x = x - 1;
                      break;

   case NUVIE_DIR_S : if(y == 0)
                        obj->y = pitch - 1;
                      else
                        obj->y = y - 1;
                      break;

   case NUVIE_DIR_W : if(x == pitch - 1)
                        obj->x = 0;
                      else
                        obj->x = x + 1;
                      break;
  }

}

inline void U6Actor::set_direction_of_surrounding_dragon_objs(uint8 new_direction)
{
 std::list<Obj *>::iterator obj;
 uint8 frame_offset =  (new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1);
 Obj *head, *tail, *wing1, *wing2;

 //NOTE! this is dependant on the order the in which the objects are loaded in U6Actor::init_dragon()

 obj = surrounding_objects.begin();
 if(obj == surrounding_objects.end())
  return;
 head = *obj;
 head->frame_n =  8 + frame_offset;
 head->x = x;
 head->y = y;

 obj++;
 if(obj == surrounding_objects.end())
  return;
 tail = *obj;
 tail->frame_n =  16 + frame_offset;
 tail->x = x;
 tail->y = y;

 obj++;
 if(obj == surrounding_objects.end())
  return;
 wing1 = *obj;
 wing1->frame_n =  24 + frame_offset;
 wing1->x = x;
 wing1->y = y;

 obj++;
 if(obj == surrounding_objects.end())
  return;
 wing2 = *obj;
 wing2->frame_n =  32 + frame_offset;
 wing2->x = x;
 wing2->y = y;

 switch(new_direction)
  {
   case NUVIE_DIR_N : head->y = y - 1;
                      tail->y = y + 1;
					  wing1->x = x - 1;
					  wing2->x = x + 1;
					  break;

   case NUVIE_DIR_E : head->x = x + 1;
                      tail->x = x - 1;
					  wing1->y = y - 1;
					  wing2->y = y + 1;
					  break;

   case NUVIE_DIR_S : head->y = y + 1;
                      tail->y = y - 1;
					  wing1->x = x + 1;
					  wing2->x = x - 1;
					  break;

   case NUVIE_DIR_W : head->x = x - 1;
                      tail->x = x + 1;
					  wing1->y = y + 1;
					  wing2->y = y - 1;
					  break;
  }

}

inline void U6Actor::twitch_surrounding_objs()
{
 std::list<Obj *>::iterator obj;

 for(obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++)
   {
    twitch_obj(*obj);
   }

}

inline void U6Actor::twitch_surrounding_dragon_objs()
{
}

inline void U6Actor::twitch_surrounding_hydra_objs()
{
 uint8 i;
 std::list<Obj *>::iterator obj;

 //Note! list order is important here. As it corresponds to the frame order in the tile set. This is definied in init_hydra()

 for(i = 0, obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++, i += 4)
   {
    if(NUVIE_RAND() % 4 == 0)
	   (*obj)->frame_n = i + (((*obj)->frame_n - i + 1) % 4);
   }
}

inline void U6Actor::twitch_obj(Obj *obj)
{
 obj->frame_n = (obj->frame_n / (actor_type->frames_per_direction * 4) * (actor_type->frames_per_direction * 4)) + direction * actor_type->tiles_per_direction +
                       walk_frame * actor_type->tiles_per_frame;

}

inline void U6Actor::clear_surrounding_objs_list(bool delete_objs)
{
 std::list<Obj *>::iterator obj;

 if(surrounding_objects.empty())
   return;

 if(delete_objs == false)
  {
   surrounding_objects.resize(0);
   return;
  }

 obj = surrounding_objects.begin();

 for(;!surrounding_objects.empty();)
  {
   obj_manager->remove_obj(*obj);
   delete *obj;
   obj = surrounding_objects.erase(obj);
  }

 return;
}

inline void U6Actor::init_surrounding_obj(uint16 x, uint16 y, uint8 z, uint16 actor_obj_n, uint16 obj_frame_n)
{
 Obj *obj;

 obj = obj_manager->get_obj(x, y, z);
 if(obj == NULL || actor_obj_n != obj->obj_n || (obj->quality != 0 && obj->quality != id_n))
  {
   obj = new Obj();
   obj->x = x;
   obj->y = y;
   obj->z = z;
   obj->obj_n = actor_obj_n;
   obj->frame_n = obj_frame_n;
   obj_manager->add_obj(obj);
  }

 obj->quality = id_n;
 surrounding_objects.push_back(obj);

 return;
}


void U6Actor::die()
{
 Game *game = Game::get_game();
 Party *party = game->get_party();
 Player *player = game->get_player();
 MapCoord actor_loc = get_location();
 MsgScroll *scroll = game->get_scroll();
 
 if(has_surrounding_objs())
   clear_surrounding_objs_list(true);
     
 Actor::die();
    
 if(in_party)
  {
   if(party->get_member_num(this) == 0) // Avatar
     {
      scroll->display_string("\nAn unending darkness engulfs thee...\n\n");
      scroll->display_string("A voice in the darkness intones, \"KAL LOR!\"\n");

      party->set_in_combat_mode(false);
//      game->get_command_bar()->set_combat_mode(false);
      player->set_party_mode(party->get_actor(0)); //set party mode with the avatar as the leader.
      player->move(0x133,0x160,0); //move to LB's castle.
      set_direction(NUVIE_DIR_N);

      party->heal();
      party->show();
     }
   else
     {
      party->remove_actor(this);
      if(player->get_actor() == this)
        player->set_party_mode(party->get_actor(0)); //set party mode with the avatar as the leader.
     }
  }
  
    // we don't generate a dead body if the avatar dies because they will be ressurrected.  
    if(actor_type->dead_obj_n != OBJ_U6_NOTHING)
    {
        if(in_party && party->get_member_num(this) == 0) // unready all items on the avatar.
          remove_all_readied_objects();
        else
         { //if not avatar then create a dead body and place on the map.
          Obj *dead_body = new Obj;
          dead_body->obj_n = actor_type->dead_obj_n;
          dead_body->frame_n = actor_type->dead_frame_n;
          dead_body->x = actor_loc.x; dead_body->y = actor_loc.y; dead_body->z = actor_loc.z;
          dead_body->quality = id_n;
          dead_body->status = OBJ_STATUS_OK_TO_TAKE;
          if(temp_actor)
            dead_body->status |= OBJ_STATUS_TEMPORARY;
          
          // FIX: move my inventory into the dead body container
          all_items_to_container(dead_body);

          obj_manager->add_obj(dead_body, true);

// FIX: add some blood? or do that in hit?
         }
    }

    if(party->get_member_num(this) != 0)
        move(0,0,0,ACTOR_FORCE_MOVE); // FIXME: move to another plane, same coords
}

// frozen by worktype or status
bool U6Actor::is_immobile()
{
    return((worktype == WORKTYPE_U6_MOTIONLESS
           || worktype == WORKTYPE_U6_IMMOBILE) && !in_party
           /*|| can_move == false*/); // can_move really means can_twitch/animate
}


bool U6Actor::is_sleeping()
{
    return(Actor::is_sleeping() || worktype == WORKTYPE_U6_SLEEP);
}

/* Find Party members. */
ActorList *U6Actor::find_players()
{
    const uint8 in_range = 5;
    ActorManager *actor_mgr = Game::get_game()->get_actor_manager();
    Party *party = Game::get_game()->get_party();
    uint8 psize = party->get_party_size();
    ActorList *actors = new ActorList;

    for(uint32 p = 0; p < psize; p++)
        if(!party->get_actor(p)->is_invisible())
            actors->push_back(party->get_actor(p));
    actors = actor_mgr->filter_distance(actors, x,y,z, in_range);
    if(actors->empty())
    {
        delete actors;
        return NULL;
    }
    return actors;
}

bool U6Actor::can_be_passed(Actor *other)
{
    U6Actor *other_ = static_cast<U6Actor *>(other);
    return(Actor::can_be_passed(other_) && other_->actor_type->movetype != actor_type->movetype);
}

void U6Actor::print()
{
    Actor::print();
// might print U6Actor members here
}

/* Returns name of NPC worktype/activity (game specific) or NULL. */
const char *U6Actor::get_worktype_string(uint32 wt)
{
    const char *wt_string = NULL;
    if(wt == WORKTYPE_U6_MOTIONLESS) wt_string = "Motionless";
    else if(wt == WORKTYPE_U6_PLAYER) wt_string = "Player";
    else if(wt == WORKTYPE_U6_IN_PARTY) wt_string = "In Party";
    else if(wt == WORKTYPE_U6_ANIMAL_WANDER) wt_string = "Graze (animal wander)";
    else if(wt == WORKTYPE_U6_WALK_TO_LOCATION) wt_string = "Walk to Schedule";
    else if(wt == WORKTYPE_U6_FACE_NORTH) wt_string = "Stand (North)";
    else if(wt == WORKTYPE_U6_FACE_SOUTH) wt_string = "Stand (South)";
    else if(wt == WORKTYPE_U6_FACE_EAST) wt_string = "Stand (East)";
    else if(wt == WORKTYPE_U6_FACE_WEST) wt_string = "Stand (West)";
    else if(wt == WORKTYPE_U6_WALK_NORTH_SOUTH) wt_string = "Guard North/South";
    else if(wt == WORKTYPE_U6_WALK_EAST_WEST) wt_string = "Guard East/West";
    else if(wt == WORKTYPE_U6_WANDER_AROUND) wt_string = "Wander";
    else if(wt == WORKTYPE_U6_WORK) wt_string = "Loiter (work)";
    else if(wt == WORKTYPE_U6_SLEEP) wt_string = "Sleep";
    else if(wt == WORKTYPE_U6_PLAY_LUTE) wt_string = "Play";
    else if(wt == WORKTYPE_U6_BEG) wt_string = "Converse";
    else if(wt == WORKTYPE_U6_COMBAT_FRONT) wt_string = "Combat Front";
    else if(wt == 0x04) wt_string = "Combat Rear";
    else if(wt == 0x05) wt_string = "Combat Flank";
    else if(wt == 0x06) wt_string = "Combat Berserk";
    else if(wt == 0x07) wt_string = "Combat Retreat";
    else if(wt == 0x08) wt_string = "Combat Assault/Wild";
    else if(wt == 0x09) wt_string = "Shy";
    else if(wt == 0x0a) wt_string = "Like";
    else if(wt == 0x0b) wt_string = "Unfriendly";
    else if(wt == 0x0d) wt_string = "Tangle";
    else if(wt == 0x0e) wt_string = "Immobile";
    else if(wt == 0x92) wt_string = "Sit";
    else if(wt == 0x93) wt_string = "Eat";
    else if(wt == 0x94) wt_string = "Farm";
    else if(wt == 0x98) wt_string = "Ring Bell";
    else if(wt == 0x99) wt_string = "Brawl";
    else if(wt == 0x9a) wt_string = "Mousing";
    return(wt_string);
}

/* Return the first food or drink object in inventory. */
Obj *U6Actor::inventory_get_food(Obj *container)
{
    U6UseCode *uc = (U6UseCode*)Game::get_game()->get_usecode();
    U6LList *inv=container?container->container:get_inventory_list();
    U6Link *link=0;
    Obj *obj=0;
    for(link=inv->start(); link!=NULL; link=link->next)
    {
        obj = (Obj*)link->data;
        if(uc->is_food(obj))
            return obj;
        if(obj->container) // search within container
            return inventory_get_food(obj);
    }
    return 0;
}

/* Set worktype to normal non-combat activity. */
void U6Actor::revert_worktype()
{
    Party *party = Game::get_game()->get_party();
    if(in_party)
        set_worktype(WORKTYPE_U6_IN_PARTY);
    if(party->get_leader() >= 0 && party->get_actor(party->get_leader()) == this)
        set_worktype(WORKTYPE_U6_PLAYER);
}
