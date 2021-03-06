/*
 *  GUI_Console.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Mon Apr 4 2011.
 *  Copyright (c) 2011. All rights reserved.
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

#include <assert.h>
#include <cmath>
#include <string>
#include "nuvieDefs.h"
#include "U6misc.h"
#include "GUI.h"
#include "GUI_widget.h"
#include "GUI_Console.h"


GUI_Console::GUI_Console(uint16 x, uint16 y, uint16 w, uint16 h)
 : GUI_Widget(NULL, x, y, w, h)
{
	bg_color = new GUI_Color(0,0,0);
	font = new GUI_Font(1);
	font->SetColoring(0xff, 0xff, 0xff, 0, 0, 0);
	num_rows = (uint16)(h / font->CharHeight());
	num_cols = (uint16)(w / font->CharWidth());
}

GUI_Console::~GUI_Console()
{
	delete bg_color;
	delete font;
}

/* Map the color to the display */
void GUI_Console::SetDisplay(Screen *s)
{
	GUI_Widget::SetDisplay(s);
	bg_color->map_color(surface);
}

/* Show the widget  */
void GUI_Console:: Display(bool full_redraw)
{
 SDL_Rect framerect;

 framerect = area;

 SDL_FillRect(surface, &framerect, bg_color->sdl_color);

 uint16 i=0;
 for(std::list<std::string>::iterator it = data.begin(); it != data.end(); it++)
 {
	 font->TextOut(surface, area.x, area.y + i * font->CharHeight(), (*it).c_str(), false);
	 i++;
 }
 screen->update(area.x,area.y,area.w,area.h);

 return;
}

void GUI_Console::AddLine(std::string line)
{
	uint16 len = line.length();
	uint16 i;

	if(len > num_cols)
	{
		for(i=0;i+num_cols<len;i+=num_cols)
			data.push_back(std::string(line.substr(i,num_cols)));

		if(i < len)
			data.push_back(std::string(line.substr(i,len - i)));
	}
	else
		data.push_back(line);


	for(i=data.size();i>num_rows;i--)
		data.pop_front();
}

GUI_status GUI_Console::MouseDown(int x, int y, int button)
{


 //grab_focus();

 return GUI_YUM;
}

GUI_status GUI_Console::MouseUp(int x, int y, int button)
{

 // release_focus();

 return GUI_YUM;
}

GUI_status GUI_Console::MouseMotion(int x,int y,Uint8 state)
{


 //GUI::get_gui()->moveWidget(this,dx,dy);
// Redraw();

 return (GUI_YUM);
}
