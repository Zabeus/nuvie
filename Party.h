#ifndef _PARTY_H_
#define _PARTY_H_
/*
 *  Party.h
 *  Nuvie
 *
 *  Created by Eric Fry on Tue May 06 2003.
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

#include <string.h>

#include "U6def.h"
#include "Configuration.h"
#include "U6File.h"
#include "U6LList.h"

#include "ActorManager.h"
#include "Actor.h"

class Game;

struct PartyMember {
char name[14];
Actor *actor;
uint8 combat_position;

};

class Party {
 Game *game; // get pointers here to avoid construct order issues in loadGame()
 Configuration *config;
 ActorManager *actor_manager;
 
 PartyMember member[16];
 uint8 num_in_party; //number of party members.
 
 public:
 
 Party(Configuration *cfg);
 ~Party();
 
 bool init(Game *g, ActorManager *am);
 
 bool contains_actor(Actor *actor)
 {
  if(get_member_num(actor) >= 0)
   return(true);
  return(false);
 }
 bool contains_actor(uint8 a)
 {
  return(contains_actor(actor_manager->get_actor(a)));
 }
 bool add_actor(Actor *actor);
 bool remove_actor(Actor *actor);
 
 void split_gold();
 void gather_gold();
 
 uint8 get_party_size();
 uint8 get_party_max() { return(8); } // U6
 Actor *get_actor(uint8 member_num);
 char *get_actor_name(uint8 member_num);
 sint8 get_member_num(Actor *actor);
 sint8 get_member_num(uint8 a)
 {
  return(get_member_num(actor_manager->get_actor(a)));
 }

 protected:

 bool loadParty();
};

#endif /* _PARTY_H_ */
