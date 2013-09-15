#ifndef __AudioDialog_h__
#define __AudioDialog_h__
/*
 *  AudioDialog.h
 *  Nuvie
 *  Copyright (C) 2013 The Nuvie Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "GUI_Dialog.h"

class GUI;
class GUI_CallBack;
class GUI_Button;
class GUI_TextToggleButton;

class AudioDialog : public GUI_Dialog {
	protected:
		GUI_CallBack *callback_object;
		GUI_Button *save_button, *load_button, *cancel_button;
		GUI_TextToggleButton *audio_button, *music_button, *musicType_button, *musicVol_button,
		                     *sfx_button, *sfxVol_button, *sfxType_button;

	public:
		AudioDialog(GUI_CallBack *callback);
		~AudioDialog();
		bool init();

		GUI_status close_dialog();
		GUI_status KeyDown(SDL_keysym key);
		GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);
};

#endif /* __AudioDialog_h__ */
