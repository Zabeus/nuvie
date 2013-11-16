#ifndef __NuvieBmpFile_h__
#define __NuvieBmpFile_h__

/*
 *  NuvieBmpFile.h
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Nov 10 2013.
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

#include <string>
#include "SDL.h"
#include "NuvieIOFile.h"
#include "TileManager.h"

class Screen;

class NuvieBmpFile
{
 private:
 unsigned char *data;
 uint32 palette[256];
 sint32 prev_width;
 sint32 prev_height;
 uint16 prev_bits;

 struct {
    uint16 type;                    /* Magic identifier            */
    uint32 size;                    /* File size in bytes          */
    uint16 reserved1, reserved2;
    uint32 offset;                  /* Offset to image data, bytes */
 } header;

 struct {
    uint32 size;                    /* Header size in bytes      */
    sint32 width,height;            /* Width and height of image */
    uint16 planes;                  /* Number of colour planes   */
    uint16 bits;                    /* Bits per pixel            */
    uint32 compression;             /* Compression type          */
    uint32 imagesize;               /* Image size in bytes       */
    sint32 xresolution,yresolution; /* Pixels per meter          */
    uint32 ncolours;                /* Number of colours         */
    uint32 importantcolours;        /* Important colours         */
 } infoHeader;

 public:

   NuvieBmpFile();
   ~NuvieBmpFile();

   bool load(std::string filename);

   uint16 getWidth() { return (uint16)infoHeader.width; }
   uint16 getHeight() { return (uint16)infoHeader.height; }


   Tile *getTile();
   unsigned char *getRawIndexedData();
   unsigned char *getRawIndexedDataCopy();
   SDL_Surface *getSdlSurface8(Screen *screen);
   SDL_Surface *getSdlSurface32();
   SDL_Surface *getSdlSurface32(std::string filename);

 private:
   bool handleError(std::string error);
};

#endif /* __NuvieBmpFile_h__ */
