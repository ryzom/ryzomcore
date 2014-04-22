#ifndef _PIC_PRIVATE_H_
#define _PIC_PRIVATE_H_
#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------------------------------------------------------------- */

typedef struct PIC_PICTURE
{
	unsigned long		ID;
	unsigned long		Width;
	unsigned long		Height;
	unsigned long		Depth;
	unsigned char		*pDatas;
	unsigned char		*pPal;
	struct PIC_PICTURE	*Next;
} PIC_PICTURE;

/* ---------------------------------------------------------------------------------------------------------------------------------- */


/*
 * JPG
 */

	
extern unsigned long	Pic_JPG_Read(	const char *FileName, 
										unsigned char **ppPal, unsigned char **ppDatas, 
										unsigned long *w, unsigned long *h);

extern unsigned long	Pic_JPG_Write(	const char *FileName, 
										unsigned long Qual, 
										unsigned char *pDatas, 
										unsigned long w, unsigned long h);
/*
 * TGA
 */
extern unsigned long	Pic_TGA_Read(	const char *FileName,
										unsigned char **ppPal, unsigned char **ppDatas,
										unsigned long *pWidth, unsigned long *pHeight, 
										unsigned long *pDepth);
extern unsigned long	Pic_TGA_Write(	const char *FileName, 
										unsigned char *pPal,unsigned char *pDatas, 
										unsigned long w, unsigned long h, unsigned long d);
/*
 * BMP
 */
extern unsigned long	Pic_BMP_Read(	const char *FileName,
										unsigned char **ppPal, unsigned char **ppDatas,
										unsigned long *pWidth, unsigned long *pHeight, 
										unsigned long *pDepth);

extern unsigned long	Pic_BMP_Write(	const char *FileName, 
										unsigned char *pPal,unsigned char *pDatas, 
										unsigned long w, unsigned long h, unsigned long d);
/*
 * System
 */
extern void*			Pic_malloc(size_t size);
extern void*			Pic_calloc(size_t count, size_t size);
extern void				Pic_free(void *memblock);
extern size_t			Pic__msize(void *memblock);
extern void				Pic_SetError(const char *msg, ...);

/* ---------------------------------------------------------------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif
#endif
