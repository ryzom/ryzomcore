#include <string.h>
#include "pic_private.h"
#include "pic.h"

static unsigned long	NbPics=0;
static PIC_PICTURE		*HeadPic=NULL;

// ----------------------------------------------------------------------------------------------------------------------------------

static PIC_PICTURE *GetPic(unsigned long id)
{
	PIC_PICTURE	*pic;

	for(pic=HeadPic ; pic ; pic=pic->Next)
	{
		if (pic->ID==id)
		{
			return(pic);
		}
	}
	return(NULL);
}

// ----------------------------------------------------------------------------------------------------------------------------------

unsigned long PIC_Load(char* FileName, unsigned char Quantize)
{
	char	ext[4];
	unsigned long	type;
	unsigned long	i,taken,id;
	PIC_PICTURE		*pic;
	char			*pDatas;
	char			*pPal;
	unsigned long	w,h,Depth;
	unsigned long	ret;

	// --- Init
	ret=0;
	type=0;
	id=0;
	taken=0;
	w=0;
	h=0;
	Depth=0;
	pic=NULL;
	pDatas=NULL;
	pPal=NULL;
	// --- Get 1st available ID
	for(i=1 ; i<=NbPics+1 ; i++)
	{
		taken=0;
		for(pic=HeadPic ; pic ; pic=pic->Next)
		{
			if (pic->ID==i)
			{
				taken=1;
				break;
			}
		}
		if (!taken)
		{
			id=i;
			break;
		}
	}
	if (!id)
	{
		Pic_SetError("Load, unable to create ID");
		return(0);
	}
	// --- Load pic
	if (FileName)
	{
		ext[0]=FileName[strlen(FileName)-3];
		ext[1]=FileName[strlen(FileName)-2];
		ext[2]=FileName[strlen(FileName)-1];
		ext[3]=0;
		strupr(ext);
		if ( !strcmp(ext,"JPG") )
		{
			type=1;
		}
		else if ( !strcmp(ext,"TGA") )
		{
			type=2;
		}
		else if ( !strcmp(ext,"BMP") )
		{
			type=3;
		}

		switch(type)
		{
		// - JPG
		case 1:
			if (!Quantize)
			{
				Depth=24;
				ret=Pic_JPG_Read(FileName,NULL,&pDatas,&w,&h);
			}
			else
			{
				Depth=8;
				ret=Pic_JPG_Read(FileName,&pPal,&pDatas,&w,&h);
			}
			if (!ret)
			{
				Pic_SetError("Load, unable to load JPG file %s",FileName);
				return(0);
			}
			break;
		// - TGA
		case 2:
			ret=Pic_TGA_Read(FileName,&pPal,&pDatas,&w,&h,&Depth);
			if (!ret)
			{
				Pic_SetError("Load, unable to load TGA file %s",FileName);
				return(0);
			}
			break;
		// - BMP
		case 3:
			ret=Pic_BMP_Read(FileName,&pPal,&pDatas,&w,&h,&Depth);
			if (!ret)
			{
				Pic_SetError("Load, unable to load BMP file %s",FileName);
				return(0);
			}
			break;
		// - Unknown
		default:
			Pic_SetError("Load, unknown extension for %s",FileName);
			return(0);
		}
	}

	// --- Create and place new pic struct
	pic=(PIC_PICTURE *)Pic_calloc(1,sizeof(PIC_PICTURE));
	if (!pic)
	{
		Pic_SetError("Load, not enough memory for internal structure");
		return(0);
	}
	pic->Next=HeadPic;
	HeadPic=pic;
	NbPics++;
	pic->ID=id;
	pic->pDatas=pDatas;
	pic->pPal=pPal;
	pic->Width=w;
	pic->Height=h;
	pic->Depth=Depth;
	return(id);
}

// ----------------------------------------------------------------------------------------------------------------------------------

unsigned long PIC_Create(char* pPal, char* pDatas, unsigned long w, unsigned long h, unsigned long d)
{
	unsigned long	i,taken,id;
	PIC_PICTURE		*pic;

	// --- Init
	id=0;
	taken=0;
	pic=NULL;
	// --- Get 1st available ID
	for(i=1 ; i<=NbPics+1 ; i++)
	{
		taken=0;
		for(pic=HeadPic ; pic ; pic=pic->Next)
		{
			if (pic->ID==i)
			{
				taken=1;
				break;
			}
		}
		if (!taken)
		{
			id=i;
			break;
		}
	}
	if (!id)
	{
		Pic_SetError("Create, unable to create ID");
		return(0);
	}
	// --- Create pic
	if (!pDatas)
	{
		pDatas=(char *)Pic_calloc(1,w*h*d/8);
		if (!pDatas)
		{
			Pic_SetError("Create, not enough memory for datas");
			return(0);
		}
	}
	if (d==8)
	{
		if (!pPal)
		{
			pPal=(char *)Pic_calloc(1,256*3);
			if (!pPal)
			{
				Pic_SetError("Create, not enough memory for palette");
				return(0);
			}
		}
	}
	else
	{
		pPal=NULL;
	}
	// --- Create and place new pic struct
	pic=(PIC_PICTURE *)Pic_calloc(1,sizeof(PIC_PICTURE));
	if (!pic)
	{
		Pic_SetError("Create, not enough memory for internal structure");
		return(0);
	}
	pic->Next=HeadPic;
	HeadPic=pic;
	NbPics++;
	pic->ID=id;
	pic->pDatas=pDatas;
	pic->pPal=pPal;
	pic->Width=w;
	pic->Height=h;
	pic->Depth=d;
	return(id);

}

// ----------------------------------------------------------------------------------------------------------------------------------

unsigned long PIC_GetInfos(	unsigned long id, 
							char **ppPal, char **ppDatas, 
							unsigned long *pW, unsigned long *pH, unsigned long *pD)
{
	PIC_PICTURE	*pic;

	pic=GetPic(id);
	if (!pic)
	{
		Pic_SetError("GetInfos, picture internal structure not found");
		return(0);
	}
	if (ppPal)
	{
		*ppPal=pic->pPal;
	}
	if (ppDatas)
	{
		*ppDatas=pic->pDatas;
	}
	if (pW)
	{
		*pW=pic->Width;
	}
	if (pH)
	{
		*pH=pic->Height;
	}
	if (pD)
	{
		*pD=pic->Depth;
	}
	return(id);
}

// ----------------------------------------------------------------------------------------------------------------------------------

static char* Conv8To24(unsigned long id)
{
	PIC_PICTURE		*pic;
	char	*buf;
	unsigned long	i;

	pic=GetPic(id);
	if (!pic)
	{
		Pic_SetError("Conv8To24, picture internal structure not found");
		return(NULL);
	}
	buf=(char *)Pic_malloc(pic->Width*pic->Height*3);
	if (!buf)
	{
		Pic_SetError("Conv8To24, not enough memory for temporary buffer");
		return(NULL);
	}
	for(i=0 ; i<pic->Width*pic->Height ; i++)
	{
		buf[i*3+0]=pic->pPal[pic->pDatas[i]*3+0];
		buf[i*3+1]=pic->pPal[pic->pDatas[i]*3+1];
		buf[i*3+2]=pic->pPal[pic->pDatas[i]*3+2];
	}
	return(buf);
}

// ----------------------------------------
static char* Conv8To16(unsigned long id)
{
	PIC_PICTURE		*pic;
	unsigned short	*buf;
	unsigned long	i;
	unsigned short	r,g,b,pix16;

	pic=GetPic(id);
	if (!pic)
	{
		Pic_SetError("Conv8To24, picture internal structure not found");
		return(NULL);
	}
	buf=(unsigned short*)Pic_malloc(pic->Width*pic->Height*2);
	if (!buf)
	{
		Pic_SetError("Conv8To24, not enough memory for temporary buffer");
		return(NULL);
	}
	for(i=0 ; i<pic->Width*pic->Height ; i++)
	{
		b=pic->pPal[pic->pDatas[i]*3+0];
		g=pic->pPal[pic->pDatas[i]*3+1];
		r=pic->pPal[pic->pDatas[i]*3+2];
		r>>=3; 
		g>>=3; g&=0x3E;
		b>>=3;
		pix16=(r<<10)+(g<<5)+b;
		buf[i]=pix16;
	}
	return (char*)buf;
}

// ----------------------------------------

static char* Conv16To24(unsigned long id)
{
	PIC_PICTURE		*pic;
	unsigned short	*pDatas;
	unsigned char	*buf;
	unsigned long	i;
	unsigned short	r,g,b;

	pic=GetPic(id);
	if (!pic)
	{
		Pic_SetError("Conv16To24, picture internal structure not found");
		return(NULL);
	}
	buf=(unsigned char *)Pic_malloc(pic->Width*pic->Height*3);
	if (!buf)
	{
		Pic_SetError("Conv16To24, not enough memory for temporary buffer");
		return(NULL);
	}
	pDatas=(unsigned short*)pic->pDatas;
	for(i=0 ; i<pic->Width*pic->Height ; i++)
	{
		r=(pDatas[i] & 0x7C00)>>(10-3);
		g=(pDatas[i] & 0x03E0)>>(5-3);
		b=(pDatas[i] & 0x001F)<<3;
		buf[i*3+0]=(unsigned char)r;
		buf[i*3+1]=(unsigned char)g;
		buf[i*3+2]=(unsigned char)b;
	}
	return (char*)buf;
}

// ----------------------------------------

static char* Conv24To16(unsigned long id)
{
	PIC_PICTURE		*pic;
	unsigned short	*buf;
	unsigned long	i;
	unsigned short	r,g,b;
	unsigned short	pix16;

	pic=GetPic(id);
	if (!pic)
	{
		Pic_SetError("Conv24To16, picture internal structure not found");
		return(NULL);
	}
	buf=(unsigned short*)Pic_malloc(pic->Width*pic->Height*2);
	if (!buf)
	{
		Pic_SetError("Conv24To16, not enough memory for temporary buffer");
		return(NULL);
	}
	for(i=0 ; i<pic->Width*pic->Height ; i++)
	{
		r=pic->pDatas[i*3+0];
		g=pic->pDatas[i*3+1];
		b=pic->pDatas[i*3+2];
		// r : 5 bits forts			(0x7C)
		// g : 5 bits (6e zapped)	(0x3E)
		// b : 5 bits faibles		(0x1F)
		r>>=3; 
		g>>=3; g&=0x3E;
		b>>=3;
		pix16=(r<<10)+(g<<5)+b;
		buf[i]=pix16;
	}
	return (char*)buf;
}

// ----------------------------------------

static char* ConvPic(PIC_PICTURE *pic, unsigned long type, char* pErr)
{
	char	*buf;
	unsigned long	src,dst;

	*pErr=0;
	buf=NULL;
	src=pic->Depth;
	if (type==PIC_TYPE_TGA8 || type==PIC_TYPE_BMP8)
	{
		dst=8;
	}
	if (type==PIC_TYPE_TGA16)
	{
		dst=16;
	}
	if (type==PIC_TYPE_JPG || type==PIC_TYPE_TGA24 || type==PIC_TYPE_BMP24)
	{
		dst=24;
	}
	// ---
	if (src==dst)
	{
		return(NULL);
	}
	// ---
	if (src==8 && dst==24)
	{
		buf=Conv8To24(pic->ID);
		if (!buf)
		{
			*pErr=1;
		}
		return(buf);
	}
	if (src==8 && dst==16)
	{
		buf=Conv8To16(pic->ID);
		if (!buf)
		{
			*pErr=1;
		}
		return(buf);
	}
	// ---
	if (src==16 && dst==24)
	{
		buf=Conv16To24(pic->ID);
		if (!buf)
		{
			*pErr=1;
		}
		return(buf);
	}
	// ---
	if (src==24 && dst==16)
	{
		buf=Conv24To16(pic->ID);
		if (!buf)
		{
			*pErr=1;
		}
		return buf;
	}
	// ---
	if (src==24 && dst==8)
	{
		Pic_SetError("ConvPic, downsampling 24 to 8 bits unsupported");
		*pErr=1;
		return(NULL);
	}
	Pic_SetError("ConvPic, conversion %d to %d unsupported",src,dst);
	*pErr=1;
	return(NULL);
}

// ----------------------------------------

unsigned long PIC_Save(unsigned long id, char* FileName, unsigned long type, unsigned long qual)
{
	PIC_PICTURE		*pic;
	char	err;
	char	*buf;
	char	*freeit;
	unsigned long	depth;

	freeit=NULL;
	pic=GetPic(id);
	if (!pic)
	{
		Pic_SetError("Save %s, picture internal structure not found",FileName);
		return(0);
	}
	freeit = ConvPic(pic,type,&err);
	if (err)
	{
		Pic_SetError("Save %s, error while converting picture",FileName);
		return(0);
	}
	if (!freeit)
	{
		buf=pic->pDatas;
	}
	else
	{
		buf=freeit;
	}
	err=0;
	switch(type)
	{
	// ---
	case PIC_TYPE_JPG:
		if ( !Pic_JPG_Write(FileName,qual,buf,pic->Width,pic->Height) )
		{
			if (freeit)
			{
				Pic_free(buf);
			}
			Pic_SetError("Save %s, error while saving JPG file",FileName);
			err=1;
		}
		break;
	// ---
	case PIC_TYPE_TGA8:
	case PIC_TYPE_TGA16:
	case PIC_TYPE_TGA24:
		if (type==PIC_TYPE_TGA8)
		{
			depth=8;
		}
		if (type==PIC_TYPE_TGA16)
		{
			depth=16;
		}
		if (type==PIC_TYPE_TGA24)
		{
			depth=24;
		}
		if ( !Pic_TGA_Write(FileName,pic->pPal,buf,pic->Width,pic->Height,depth) )
		{
			if (freeit)
			{
				Pic_free(freeit);
			}
			Pic_SetError("Save %s, error while saving TGA file",FileName);
			err=1;
		}
		break;
	// ---
	case PIC_TYPE_BMP8:
	case PIC_TYPE_BMP24:
		if (type==PIC_TYPE_BMP8)
		{
			depth=8;
		}
		if (type==PIC_TYPE_BMP24)
		{
			depth=24;
		}
		if ( !Pic_BMP_Write(FileName,pic->pPal,buf,pic->Width,pic->Height,depth) )
		{
			if (freeit)
			{
				Pic_free(freeit);
			}
			Pic_SetError("Save %s, error while saving BMP file",FileName);
			err=1;
		}
		break;
	// ---
	default:		
		Pic_SetError("Save %s, unknow save format/type",FileName);
		err=1;
		break;
	}
	if (freeit)
	{
		Pic_free(freeit);
	}
	return(err-1);
}

// ----------------------------------------------------------------------------------------------------------------------------------

unsigned long PIC_Destroy(unsigned long id)
{
	PIC_PICTURE		*prevpic,*pic;
	unsigned long	found;

	prevpic=NULL;
	found=0;
	for(pic=HeadPic ; pic ; pic=pic->Next)
	{
		if (pic->ID==id)
		{
			found=1;
			break;
		}
		prevpic=pic;
	}
	if (!found)
	{
		Pic_SetError("Destroy, picture internal structure not found");
		return(0);
	}
	if (prevpic)
	{
		prevpic->Next=pic->Next;
	}
	if (pic->pDatas)
	{
		Pic_free(pic->pDatas);
	}
	if (pic->pPal)
	{
		Pic_free(pic->pPal);
	}
	if (pic==HeadPic)
	{
		HeadPic=pic->Next;
	}
	Pic_free(pic);
	return(1);
}

// ----------------------------------------------------------------------------------------------------------------------------------