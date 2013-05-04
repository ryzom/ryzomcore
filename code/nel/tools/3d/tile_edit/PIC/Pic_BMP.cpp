#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pic_private.h"
#include "pic.h"

// ----------------------------------------------------------------------------------------------------------------------------------

#pragma pack(1)
typedef struct BMP_HEADER
{
	unsigned short	bfType;
	unsigned long	bfSize;
	unsigned short	Res1;
	unsigned short	Res2;
	unsigned long	bfOffBits;
	unsigned long	biSize;
	unsigned long	biWidth;
	unsigned long	biHeight;
	unsigned short	biPlanes;
	unsigned short	biBitCount;
	unsigned long	biCompression;
	unsigned long	biSizeImage;
	unsigned long	biXPelsPerMeter;
	unsigned long	biYPelsPerMeter;
	unsigned long	biClrUsed;
	unsigned long	biClrImportant;
} BMP_HEADER;
#pragma pack()

// ----------------------------------------------------------------------------------------------------------------------------------

unsigned long Pic_BMP_Write(	const char *FileName, 
								char *pPal, char *pDatas, 
								unsigned long w, unsigned long h, unsigned long d)

{
	FILE			*file;
	BMP_HEADER		bmph;
	unsigned long	slsize;
	unsigned char	*scanline;	
	unsigned long	i;
	long			x,y,rest;
	unsigned char	r,g,b;

	file=fopen(FileName,"wb");
	if (!file)
	{
		return(0);
	}
	memset(&bmph,0,sizeof(BMP_HEADER));
	bmph.bfType=19778;
	bmph.bfSize=sizeof(BMP_HEADER);
	bmph.bfSize+=w*h*d/8;
	if (pPal)
	{
		bmph.bfSize+=(256*4);
	}
	bmph.bfOffBits=sizeof(BMP_HEADER);
	if (pPal)
	{
		bmph.bfOffBits+=(256*4);
	}
	bmph.biSize=40;//sizeof(BMP_HEADER);
	bmph.biWidth=w;
	bmph.biHeight=h;
	bmph.biPlanes=1;
	bmph.biBitCount=(unsigned short)d;
	bmph.biCompression=0;
	bmph.biSizeImage=w*h*d/8;

	fwrite(&bmph,1,sizeof(BMP_HEADER),file);
	if (pPal)
	{
		for(i=0 ; i<256 ; i++)
		{
			fwrite(&pPal[i*3+0],1,1,file);
			fwrite(&pPal[i*3+1],1,1,file);
			fwrite(&pPal[i*3+2],1,1,file);
			fwrite(&pPal[i*3+2],1,1,file);
		}
	}
	slsize=w*d/8;
	scanline=(unsigned char*)Pic_calloc(1,slsize);
	if (!scanline)
	{
		Pic_SetError("BMP_Write, not enough memory for scanline");
		return(0);
	}
	for(rest=0 ; ((w*d/8)+rest)%4!=0 ; rest++);
	for(y=0 ; y<(long)h ; y++)
	{
		memcpy(scanline,&pDatas[(h-y-1)*slsize],slsize);
		if (d==24)
		{
			for(x=0 ; x<(long)w ; x++)
			{
				b=scanline[x*3+0];
				g=scanline[x*3+1];
				r=scanline[x*3+2];
				scanline[x*3+0]=b;
				scanline[x*3+1]=g;
				scanline[x*3+2]=r;				
			}
		}
		fwrite(scanline,1,slsize,file);
		if (rest)
		{
			fwrite(scanline,1,rest,file);
		}
	}
	Pic_free(scanline);
	fclose(file);
	return(1);
}

// ----------------------------------------------------------------------------------------------------------------------------------

unsigned long Pic_BMP_Read(	const char *FileName,
							char **ppPal, char **ppDatas,
							unsigned long *pWidth, unsigned long *pHeight, 
							unsigned long *pDepth)
{
	FILE			*file;
	BMP_HEADER		bmph;
	char			*pPal;
	char			*pDatas;
	unsigned char	*scanline;
	long			w,h,d;	
	long			i,x,y,rest;
	unsigned char	r,g,b;
	unsigned char	pad[4];

	pPal=NULL;
	pDatas=NULL;
	file=fopen(FileName,"rb");
	if (!file)
	{
		Pic_SetError("BMP_Read, unable to open %s",FileName);
		return(0);
	}
	fread(&bmph,1,sizeof(BMP_HEADER),file);
	*pWidth=w=bmph.biWidth;
	*pHeight=h=bmph.biHeight;
	*pDepth=d=bmph.biBitCount;
	if (d!=8 && d!=24)
	{
		Pic_SetError("BMP_Read, number of bits per pixel unsupported");
		return(0);
	}
	if (*pDepth==8)
	{
		pPal=(char*)Pic_calloc(1,256*3);
		if (!pPal)
		{
			Pic_SetError("BMP_Read, not enough memory for palette");
			return(0);
		}
		for(i=0 ; i<256 ; i++)
		{
			fread(&pPal[i*3+2],1,1,file);		
			fread(&pPal[i*3+1],1,1,file);		
			fread(&pPal[i*3+0],1,1,file);		
			fread(&pad[0],1,1,file);
		}		
	}
	pDatas=(char*)Pic_calloc(1,w*h*d/8);
	if (!pDatas)
	{
		if (pPal)
		{
			Pic_free(pPal);
		}
		Pic_SetError("BMP_Read, not enough memory for datas");
		return(0);
	}
	scanline=(unsigned char*)Pic_calloc(1,w*h*d/8);
	if (!scanline)
	{
		if (pPal)
		{
			Pic_free(pPal);
		}
		Pic_free(pDatas);
		Pic_SetError("BMP_Read, not enough memory for scanline");
		return(0);
	}
	for(rest=0 ; (w+rest)%4!=0 ; rest++);
	for(y=0 ; y<h ; y++)
	{
		fread(scanline,w,d/8,file);
		if (d==24)
		{
			for(x=0 ; x<w ; x++)
			{
				r=scanline[x*3+0];
				g=scanline[x*3+1];
				b=scanline[x*3+2];
				scanline[x*3+0]=b;
				scanline[x*3+1]=g;
				scanline[x*3+2]=r;				
			}
		}
		memcpy(&pDatas[(h-y-1)*w*d/8],scanline,w*d/8);
		fread(pad,rest,d/8,file);
	}
	fclose(file);
	Pic_free(scanline);
	*ppPal=pPal;
	*ppDatas=pDatas;
	return(1);
}

// ----------------------------------------------------------------------------------------------------------------------------------
