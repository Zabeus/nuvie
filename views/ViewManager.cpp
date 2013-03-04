/*
 *  ViewManager.cpp
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

#include "nuvieDefs.h"
#include "Configuration.h"
#include "U6misc.h"

#include "GUI.h"

#include "ViewManager.h"

#include "Actor.h"

#include "ActorView.h"
#include "PortraitView.h"
#include "InventoryView.h"
#include "DollViewGump.h"
#include "ContainerViewGump.h"
#include "PortraitViewGump.h"
#include "SignViewGump.h"
#include "PartyView.h"
#include "SpellView.h"
#include "SpellViewGump.h"
#include "MapWindow.h"
#include "MapEditorView.h"
#include "MsgScroll.h"
#include "Party.h"

ViewManager::ViewManager(Configuration *cfg)
{
 config = cfg;
 config->value("config/GameType",game_type);
 current_view = NULL; gui = NULL; text = NULL;
 tile_manager = NULL; obj_manager = NULL; party = NULL;
 portrait = NULL; actor_view = NULL; inventory_view = NULL;
 portrait_view = NULL; party_view = NULL; spell_view = NULL;
 doll_next_party_member = 0;
}

ViewManager::~ViewManager()
{
 // only delete the views that are not currently active
 if (current_view != actor_view)     delete actor_view;
 if (current_view != inventory_view) delete inventory_view;
 if (current_view != party_view)     delete party_view;
 if (current_view != portrait_view)  delete portrait_view;
 if (current_view != spell_view)  delete spell_view;
}

bool ViewManager::init(GUI *g, Text *t, Party *p, Player *player, TileManager *tm, ObjManager *om, Portrait *por)
{
 gui = g;
 text = t;
 party = p;
 tile_manager = tm;
 obj_manager = om;
 portrait = por;

 uint16 x_off = Game::get_game()->get_game_x_offset();
 uint16 y_off = Game::get_game()->get_game_y_offset();

 inventory_view = new InventoryView(config);
 inventory_view->init(gui->get_screen(), this, 176+x_off,8+y_off, text, party, tile_manager, obj_manager);

 portrait_view = new PortraitView(config);
 portrait_view->init(176+x_off,8+y_off, text, party, tile_manager, obj_manager, portrait);

 if(Game::get_game()->is_orig_style())
 {
	 //inventory_view = new InventoryView(config);
	 //inventory_view->init(gui->get_screen(), this, 176+x_off,8+y_off, text, party, tile_manager, obj_manager);
	 actor_view = new ActorView(config);
	 actor_view->init(gui->get_screen(), this, 176+x_off,8+y_off, text, party, tile_manager, obj_manager, portrait);

	 party_view = new PartyView(config);
	 party_view->init(this,168+x_off,6+y_off, text, party, player, tile_manager, obj_manager);

	 spell_view = new SpellView(config);
 }
 else
 {
	 //inventory_view = new InventoryViewGump(config);
	 //inventory_view->init(gui->get_screen(), this, 176+x_off,8+y_off, text, party, tile_manager, obj_manager);

	 spell_view = new SpellViewGump(config);
 }

 uint16 spell_x_offset = 168+x_off;
 if(Game::get_game()->is_new_style())
 {
	spell_x_offset = Game::get_game()->get_game_width() - SPELLVIEWGUMP_WIDTH + x_off;
 }

 spell_view->init(gui->get_screen(), this, spell_x_offset, 6+y_off, text, party, tile_manager, obj_manager);
 //set_current_view((View *)party_view);

 return true;
}

void ViewManager::reload()
{
 if(Game::get_game()->is_orig_style())
   actor_view->set_party_member(0);
 inventory_view->set_party_member(0);

 set_party_mode();
 update();
}

bool ViewManager::set_current_view(View *view)
{
 uint8 cur_party_member;

 //actor_view->set_party_member(cur_party_member);
 if(view == NULL ) // || game_type != NUVIE_GAME_U6) //HACK! remove this when views support MD and SE
   return false;

 if(current_view == view) // nothing to do if view is already the current_view.
   return false;

 if(current_view != NULL)
   {
    gui->removeWidget((GUI_Widget *)current_view);//remove current widget from gui

    cur_party_member = current_view->get_party_member_num();
    view->set_party_member(cur_party_member);
   }

 current_view = view;
 view->Show();
 gui->AddWidget((GUI_Widget *)view);
 view->Redraw();
 gui->Display();

  if(inventory_view)
  {
    if(view != inventory_view)
    {
      inventory_view->set_show_cursor(false);
      inventory_view->release_focus();
    } 
  }

 return true;
}

void ViewManager::close_current_view()
{
	if(current_view == NULL)
		return;

	gui->removeWidget((GUI_Widget *)current_view);//remove current widget from gui
	current_view = NULL;
}

void ViewManager::update()
{
 if(current_view)
   current_view->Redraw();

 return;
}

// We only change to portrait mode if the actor has a portrait.
void ViewManager::set_portrait_mode(Actor *actor, const char *name)
{
 if(portrait_view->set_portrait(actor, name) == true)
  {
   set_current_view((View *)portrait_view);
  }
}

void ViewManager::set_inventory_mode()
{
 set_current_view((View *)inventory_view);
}

void ViewManager::set_party_mode()
{
 if(Game::get_game()->is_orig_style())
	 set_current_view((View *)party_view);
 return;
}

void ViewManager::set_actor_mode()
{
 set_current_view((View *)actor_view);
}

void ViewManager::set_spell_mode(Actor *caster, Obj *spell_container, bool eventMode)
{
 if(spell_view != NULL)
 {
   spell_view->set_spell_caster(caster, spell_container, eventMode);
   set_current_view((View *)spell_view);
 }
 return;
}

void ViewManager::close_spell_mode()
{
	  //FIXME this should set previous view. Don't default to inventory view.
	  spell_view->release_focus();
	  if(Game::get_game()->is_orig_style())
		  set_inventory_mode();
	  else
		  close_current_view();
}

void ViewManager::open_doll_view(Actor *actor)
{
	Screen *screen = Game::get_game()->get_screen();

	if(Game::get_game()->is_new_style())
	{
		if(actor == NULL)
		{
			actor = doll_view_get_next_party_member();
		}
		DollViewGump *doll = get_doll_view(actor);
		if(doll == NULL)
		{
			uint16 x_off = Game::get_game()->get_game_x_offset();
			uint16 y_off = Game::get_game()->get_game_y_offset();
			uint8 num_doll_gumps = doll_gumps.size();
			doll = new DollViewGump(config);
			uint16 x = 12 * num_doll_gumps;
			uint16 y = 12 * num_doll_gumps;

			if(y + DOLLVIEWGUMP_HEIGHT > screen->get_height())
				y = screen->get_height() - DOLLVIEWGUMP_HEIGHT;

			doll->init(Game::get_game()->get_screen(), this, x + x_off, y + y_off, actor, text, party, tile_manager, obj_manager);

			add_view((View *)doll);
			add_gump(doll);
			doll_gumps.push_back(doll);
		}
		else
		{
			move_gump_to_top(doll);
		}
	}
}

Actor *ViewManager::doll_view_get_next_party_member()
{
	if(doll_gumps.empty())
	{
		doll_next_party_member = 0; //reset to first party member when there are no doll gumps on screen.
	}
	Actor *a = party->get_actor(doll_next_party_member);
	doll_next_party_member = (doll_next_party_member + 1) % party->get_party_size();

	return a;
}

DollViewGump *ViewManager::get_doll_view(Actor *actor)
{
	std::list<DraggableView *>::iterator iter;
	for(iter=doll_gumps.begin(); iter != doll_gumps.end();iter++)
	{
		DollViewGump *view = (DollViewGump *)*iter;
		if(view->get_actor() == actor)
		{
			return view;
		}
	}

	return NULL;
}

ContainerViewGump *ViewManager::get_container_view(Actor *actor, Obj *obj)
{
	std::list<DraggableView *>::iterator iter;
	for(iter=container_gumps.begin(); iter != container_gumps.end();iter++)
	{
		ContainerViewGump *view = (ContainerViewGump *)*iter;
		if(actor)
		{
			if(view->is_actor_container() && view->get_actor() == actor)
			{
				return view;
			}
		}
		else if(obj)
		{
			if(!view->is_actor_container() && view->get_container_obj() == obj)
			{
				return view;
			}
		}
	}

	return NULL;
}

void ViewManager::open_container_view(Actor *actor, Obj *obj)
{
	ContainerViewGump *view = get_container_view(actor, obj);

	if(view == NULL)
	{
		uint16 x_off = Game::get_game()->get_game_x_offset();
		uint16 y_off = Game::get_game()->get_game_y_offset();

		view = new ContainerViewGump(config);
		view->init(Game::get_game()->get_screen(), this, Game::get_game()->get_game_width() - 120 + x_off, 20 + y_off, text, party, tile_manager, obj_manager, obj);
		if(actor)
			view->set_actor(actor);
		else
			view->set_container_obj(obj);

		container_gumps.push_back(view);
		add_gump(view);
		add_view((View *)view);
	}
	else
	{
		move_gump_to_top(view);
	}
}

void ViewManager::close_container_view(Actor *actor)
{
	ContainerViewGump *view = get_container_view(actor, NULL);

	if(view)
	{
		close_gump(view);
	}
}

void ViewManager::open_mapeditor_view()
{
	if(Game::get_game()->is_new_style() && Game::get_game()->is_roof_mode())
	{
		uint16 x_off = Game::get_game()->get_game_x_offset();
		uint16 y_off = Game::get_game()->get_game_y_offset();
		x_off += Game::get_game()->get_game_width() - 90;
		MapEditorView *view = new MapEditorView(config);
		view->init(Game::get_game()->get_screen(), this, x_off , y_off, text, party, tile_manager, obj_manager);
		add_view((View *)view);
		view->grab_focus();
	}
}

void ViewManager::open_portrait_gump(Actor *a)
{
	if(Game::get_game()->is_new_style())
	{
		uint16 x_off = Game::get_game()->get_game_x_offset();
		uint16 y_off = Game::get_game()->get_game_y_offset();
		PortraitViewGump *view = new PortraitViewGump(config);
		view->init(Game::get_game()->get_screen(), this, 62 + x_off, y_off, text, party, tile_manager, obj_manager, portrait, a);
		add_view((View *)view);
		add_gump(view);
		view->grab_focus();
	}
}

void ViewManager::open_sign_gump(const char *sign_text, uint16 length)
{
	if(Game::get_game()->is_using_text_gumps()) // check should be useless
	{
		SignViewGump *view = new SignViewGump(config);
		view->init(Game::get_game()->get_screen(), this, text, party, tile_manager, obj_manager, sign_text, length);
		add_view((View *)view);
		add_gump(view);
		view->grab_focus();
	}
}

void ViewManager::add_view(View *view)
{
	view->Show();
	gui->AddWidget((GUI_Widget *)view);
	if(Game::get_game()->is_new_style())
	{
		Game::get_game()->get_scroll()->moveToFront();
	}
	view->Redraw();
	gui->Display();
}

void ViewManager::add_gump(DraggableView *gump)
{
	gumps.push_back(gump);
	Game::get_game()->get_map_window()->set_walking(false);
}

void ViewManager::close_gump(DraggableView *gump)
{
	gumps.remove(gump);
	container_gumps.remove(gump);
	doll_gumps.remove(gump);

	gump->close_view();
	gump->Delete();
	//gui->removeWidget((GUI_Widget *)gump);
}

void ViewManager::close_all_gumps()
{
	std::list<DraggableView *>::iterator iter;
	for(iter=gumps.begin(); iter != gumps.end();)
	{
		DraggableView *gump = *iter;
		iter++;

		close_gump(gump);
	}
	//TODO make sure all gump objects have been deleted by GUI.
}

void ViewManager::move_gump_to_top(DraggableView *gump)
{
	gump->moveToFront();
	Game::get_game()->get_scroll()->moveToFront();
}

// callbacks for switching views

GUI_status partyViewButtonCallback(void *data)
{
 ViewManager *view_manager;

 view_manager = (ViewManager *)data;

 view_manager->set_party_mode();

 return GUI_YUM;
}

GUI_status actorViewButtonCallback(void *data)
{
 ViewManager *view_manager;

 view_manager = (ViewManager *)data;

 view_manager->set_actor_mode();

 return GUI_YUM;
}

GUI_status inventoryViewButtonCallback(void *data)
{
 ViewManager *view_manager;

 view_manager = (ViewManager *)data;

 view_manager->set_inventory_mode();

 return GUI_YUM;
}
