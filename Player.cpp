/*
 *  player.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Mar 23 2003.
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
#include "U6misc.h"
#include "NuvieIOFile.h"

#include "Player.h"

Player::Player(Configuration *cfg)
{
 config = cfg;
 config->value("config/GameType",game_type);
 
 karma = 0;
 questf = 0;
}

bool Player::init(ActorManager *am, MapWindow *mw, GameClock *c, Party *p)
{
 clock = c;
 actor_manager = am;
 map_window = mw;
 party = p;
 actor = actor_manager->get_actor(1);
 
 loadObjlistData(); //load Player name, Karma.
 
 actor->set_in_party(true);
 actor_manager->set_player(actor);

 return true;
}


void Player::set_actor(Actor *new_actor)
{
 actor = new_actor;
}

Actor *Player::get_actor()
{
 return actor;
}

void Player::get_location(uint16 *ret_x, uint16 *ret_y, uint8 *ret_level)
{
 actor->get_location(ret_x,ret_y,ret_level);
}

char *Player::get_name()
{
 return name;
}


/* Add to Player karma. Handle appropriately the karma min/max limits.
 */
void Player::add_karma(uint8 val)
{
    karma = ((karma + val) <= 99) ? karma + val : 99;
}


/* Subtract from Player karma. Handle appropriately the karma min/max limits.
 */
void Player::subtract_karma(uint8 val)
{
    karma = ((karma - val) >= 0) ? karma - val : 0;
}


char *Player::get_gender_title()
{
 if(gender == 0)
   return "milord";
 else
   return "milady";
}

void Player::moveRelative(sint16 rel_x, sint16 rel_y)
{
 if(rel_x < 0)
   actor->set_direction(3);
 if(rel_x > 0)
   actor->set_direction(1);
 if(rel_y < 0)
   actor->set_direction(0);
 if(rel_y > 0)
   actor->set_direction(2);
   
 if(actor->moveRelative(rel_x,rel_y))
   map_window->moveMapRelative(rel_x,rel_y);
   
 clock->inc_move_counter();
 actor_manager->updateActors();
}

void Player::move(sint16 new_x, sint16 new_y, uint8 new_level)
{
 if(actor->move(new_x,new_y,new_level))
   {
    //map_window->moveMap(new_x,new_y,new_level);
    map_window->centerMapOnActor(actor);
   } 
}

void Player::moveLeft()
{
 moveRelative(-1,0);
}

void Player::moveRight()
{
 moveRelative(1,0);
}

void Player::moveUp()
{
 moveRelative(0,-1);
}

void Player::moveDown()
{
 moveRelative(0,1);
}

void Player::pass()
{
 clock->inc_move_counter_by_a_minute();
}

bool Player::loadObjlistData()
{
 std::string filename;
 NuvieIOFileRead objlist;
// int game_type;
 
 config_get_path(config,"savegame/objlist",filename);

 if(objlist.open(filename) == false)
   return false;

 objlist.seek(0xf00);
 
 objlist.readToBuf((unsigned char *)name,14); // read in Player name. 
 
 if(game_type == NUVIE_GAME_U6)
   {
    objlist.seek(0x1bf1); // U6 Quest Flag
    questf = objlist.read1();

    objlist.seek(0x1bf9); // Player Karma.
    karma = objlist.read1();

    objlist.seek(0x1c71); // Player Gender.
    gender = objlist.read1();
   }
   
 if(game_type == NUVIE_GAME_MD)
   {
    objlist.seek(0x1d27); // Player Gender.
    gender = objlist.read1();
   }

 
 return true;
}
