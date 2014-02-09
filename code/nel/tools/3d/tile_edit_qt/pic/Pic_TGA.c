#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pic_private.h"
#include "pic.h"

/* ---------------------------------------------------------------------------------------------------------------------------------- */

#pragma pack(1)
typedef struct TGA_HEADER
{
	unsigned char	LengthID;
	unsigned char	CMapType;
	unsigned char	ImageType;
	unsigned short	Origin;
	unsigned short	Length;
	unsigned char	Depth;
	unsigned short	XOrg;
	unsigned short	YOrg;
	unsigned short	Width;
	unsigned short	Height;
	unsigned char	ImageDepth;
	unsigned char	Desc;
} TGA_HEADER;
#pragma pack()

/* ---------------------------------------------------------------------------------------------------------------------------------- */

unsigned long Pic_TGA_Read(	const char *FileName,
							unsigned char **ppPal, unsigned char **ppDatas,
							unsigned long *pWidth, unsigned long *pHeight, 
							unsigned long *pDepth)
{
	FILE			*file;
	TGA_HEADER		tgah;
	long			w,h,d;
	unsigned long	size;
	unsigned char	*pDatas;
	unsigned char	*pPal;
	long			x,y;
	long			slsize;
	unsigned char	*scanline;
	unsigned char	r,g,b;
	long			i;
	int				upSideDown;

	pDatas=NULL;
	pPal=NULL;
	file=fopen(FileName,"rb");
	if (!file)
	{
		Pic_SetError("TGA_Read, unable to open %s",FileName);
		return(0);
	}
	fread(&tgah,1,sizeof(TGA_HEADER),file);
	if (tgah.ImageType>3)
	{
		Pic_SetError("TGA_Read, unsupported TGA format");
		return(0);
	}
	*pWidth=w=tgah.Width;
	*pHeight=h=tgah.Height;
	*pDepth=d=tgah.ImageDepth;
	upSideDown = ((tgah.Desc & (1 << 5))==0);

	size=tgah.Width*tgah.Height*(tgah.ImageDepth/8);
	pDatas=Pic_malloc(size);
	if (!pDatas)
	{
		Pic_SetError("TGA_Read, not enough memory");
		return(0);
	}
	if (*pDepth==8)
	{
		if (!ppPal)
		{
			Pic_free(pDatas);
			Pic_SetError("TGA_Read, need a pointer to palette");
			return(0);
		}
		pPal=Pic_calloc(1,256*3);
		if (!pPal)
		{
			Pic_SetError("TGA_Read, not enough memory for palette");
			return(0);
		}
		if (tgah.ImageType==1)
		{
			for(i=0 ; i<256*3 ; i+=3)
			{
				fread(&pPal[i+2],1,1,file);
				fread(&pPal[i+1],1,1,file);
				fread(&pPal[i+0],1,1,file);
			}
		}
		*ppPal=pPal;
	}

	slsize=w*d/8;
	scanline=Pic_calloc(1,slsize);
	if (!scanline)
	{
		if (pPal)
		{
			Pic_free(pPal);
		}
		Pic_free(pDatas);
		Pic_SetError("TGA_Read, not enough memory for scanline");
		return(0);
	}
	for(y=0 ; y<h ; y++)
	{
		fread(scanline,1,slsize,file);
		if (d==24 || d==32)
		{
			long	mult=3;
			if(d==32)  mult=4;
			for(x=0 ; x<w ; x++)
			{
				r=scanline[x*mult+0];
				g=scanline[x*mult+1];
				b=scanline[x*mult+2];
				scanline[x*mult+0]=b;
				scanline[x*mult+1]=g;
				scanline[x*mult+2]=r;
			}
		}
		if (upSideDown)
			memcpy(&pDatas[(h-y-1)*slsize], scanline, slsize);
		else
			memcpy(&pDatas[(y)*slsize], scanline, slsize);
	}
	Pic_free(scanline);
	fclose(file);
	*ppDatas=pDatas;
	return(1);

}

/* ---------------------------------------------------------------------------------------------------------------------------------- */

unsigned long Pic_TGA_Write(	const char *FileName, 
								unsigned char *pPal,unsigned char *pDatas, 
								unsigned long w, unsigned long h, unsigned long d)
{
	FILE			*file;
	TGA_HEADER		tgah;
	long			x,y;
	long			slsize;
	unsigned char	*scanline;
	unsigned char	r,g,b;

	file=fopen(FileName,"wb");
	if (!file)
	{
		Pic_SetError("TGA_Write, unable to open %s",FileName);
		return(0);
	}
	memset(&tgah,0,sizeof(TGA_HEADER));
	tgah.LengthID=0;
	if (d>8)
	{
		tgah.CMapType=0;
		tgah.ImageType=2;
		tgah.Length=0;
		tgah.Depth=0;
	}
	else
	{
		tgah.CMapType=1;
		tgah.ImageType=1;
		tgah.Length=256;
		tgah.Depth=24;
	}	
	tgah.Origin=0;
	tgah.XOrg=0;
	tgah.YOrg=0;
	tgah.Width=(unsigned short)w;
	tgah.Height=(unsigned short)h;
	tgah.ImageDepth=(unsigned char)d;
	tgah.Desc=0;
	fwrite(&tgah,1,sizeof(TGA_HEADER),file);
	if (d==8)
	{
		fwrite(pPal,1,256*3,file);
	}
	slsize=w*d/8;
	scanline=Pic_calloc(1,slsize);
	if (!scanline)
	{
		Pic_SetError("TGA_Write, not enough memory for scanline");
		return(0);
	}
	for(y=0 ; y<(long)h ; y++)
	{
		memcpy(scanline,&pDatas[(h-y-1)*slsize],slsize);
		if (d==24)
		{
			for(x=0 ; x<(long)w ; x++)
			{
				r=scanline[x*3+0];
				g=scanline[x*3+1];
				b=scanline[x*3+2];
				scanline[x*3+0]=b;
				scanline[x*3+1]=g;
				scanline[x*3+2]=r;				
			}
		}
		fwrite(scanline,1,slsize,file);
	}
	Pic_free(scanline);
	fclose(file);
	return(1);
}
