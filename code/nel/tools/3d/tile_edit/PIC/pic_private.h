// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _PIC_PRIVATE_H_
#define _PIC_PRIVATE_H_

// ----------------------------------------------------------------------------------------------------------------------------------

typedef struct PIC_PICTURE
{
	unsigned long		ID;
	unsigned long		Width;
	unsigned long		Height;
	unsigned long		Depth;
	char				*pDatas;
	char				*pPal;
	struct PIC_PICTURE	*Next;
} PIC_PICTURE;

// ----------------------------------------------------------------------------------------------------------------------------------


//
// JPG
//

	
extern unsigned long	Pic_JPG_Read(	const char *FileName, 
										char **ppPal, char **ppDatas, 
										unsigned long *w, unsigned long *h);

extern unsigned long	Pic_JPG_Write(	const char *FileName, 
										unsigned long Qual, 
										char *pDatas, 
										unsigned long w, unsigned long h);
//
// TGA
//
extern unsigned long	Pic_TGA_Read(	const char *FileName,
										char **ppPal, char **ppDatas,
										unsigned long *pWidth, unsigned long *pHeight, 
										unsigned long *pDepth);
extern unsigned long	Pic_TGA_Write(	const char *FileName, 
										char *pPal, char *pDatas, 
										unsigned long w, unsigned long h, unsigned long d);
//
// BMP
//
extern unsigned long	Pic_BMP_Read(	const char *FileName,
										char **ppPal, char **ppDatas,
										unsigned long *pWidth, unsigned long *pHeight, 
										unsigned long *pDepth);

extern unsigned long	Pic_BMP_Write(	const char *FileName, 
										char *pPal, char *pDatas, 
										unsigned long w, unsigned long h, unsigned long d);
//
// System
//
extern void*			Pic_malloc(unsigned long size);
extern void*			Pic_calloc(unsigned long count, unsigned long size);
extern void				Pic_free(void *memblock);
extern unsigned long	Pic__msize(void *memblock);
extern void				Pic_SetError(const char *msg, ...);

// ----------------------------------------------------------------------------------------------------------------------------------

#endif