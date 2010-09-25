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