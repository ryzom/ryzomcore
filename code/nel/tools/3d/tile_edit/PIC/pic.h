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

#ifndef _PIC_H_
#define _PIC_H_

// ----------------------------------------------------------------------------------------------------------------------------------

#define	PIC_TYPE_JPG		1
#define	PIC_TYPE_TGA8		2
#define	PIC_TYPE_TGA16		3
#define	PIC_TYPE_TGA24		4
#define PIC_TYPE_BMP8		5
#define PIC_TYPE_BMP24		6

// ----------------------------------------------------------------------------------------------------------------------------------

//
// Basic API
//
extern unsigned long	PIC_Load(char* FileName, unsigned char Quantize);

extern unsigned long	PIC_Create(char* pPal, char* pDatas, unsigned long w, unsigned long h, unsigned long d);

extern unsigned long	PIC_Save(unsigned long id, char* FileName, unsigned long type, unsigned long qual);

extern unsigned long	PIC_GetInfos(	unsigned long id, 
										char **ppPal, char **ppDatas, 
										unsigned long *pW, unsigned long *pH, unsigned long *pD);


extern unsigned long	PIC_Destroy(unsigned long id);
//
// System
// 
extern unsigned long	PIC_GetMemNbAllocs(void);
extern unsigned long	PIC_GetMemAllocated(void);
extern char*			PIC_GetError(void);
extern unsigned char	PIC_Error(void);
extern void				PIC_ResetError(void);
extern unsigned char	PIC_OnErrorCall( void pFnct(void) );

// ----------------------------------------------------------------------------------------------------------------------------------

#endif
