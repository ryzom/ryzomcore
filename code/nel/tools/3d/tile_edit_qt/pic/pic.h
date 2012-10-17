#ifndef _PIC_H_
#define _PIC_H_
#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------------------------------------------------------------- */

#define	PIC_TYPE_JPG		1
#define	PIC_TYPE_TGA8		2
#define	PIC_TYPE_TGA16		3
#define	PIC_TYPE_TGA24		4
#define PIC_TYPE_BMP8		5
#define PIC_TYPE_BMP24		6

/* ---------------------------------------------------------------------------------------------------------------------------------- */

/*
 * Basic API
 */
extern unsigned long	PIC_Load(char* FileName, unsigned char Quantize);

extern unsigned long	PIC_Create(unsigned char* pPal, unsigned char* pDatas, unsigned long w, unsigned long h, unsigned long d);

extern unsigned long	PIC_Save(unsigned long id, const char* FileName, unsigned long type, unsigned long qual);

extern unsigned long	PIC_GetInfos(	unsigned long id, 
										unsigned char* *ppPal, unsigned char* *ppDatas, 
										unsigned long *pW, unsigned long *pH, unsigned long *pD);


extern unsigned long	PIC_Destroy(unsigned long id);
/*
 * System
 */ 
extern unsigned long	PIC_GetMemNbAllocs(void);
extern unsigned long	PIC_GetMemAllocated(void);
extern char*			PIC_GetError(void);
extern unsigned char	PIC_Error(void);
extern void				PIC_ResetError(void);
extern unsigned char	PIC_OnErrorCall( void pFnct(void) );

/* ---------------------------------------------------------------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif
#endif
