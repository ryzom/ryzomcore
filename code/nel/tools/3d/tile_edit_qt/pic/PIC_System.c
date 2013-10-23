#include <stdio.h>
#include <stdlib.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#include <string.h>
#include <stdarg.h>

#define PIC_ERRSIZE		256

static size_t	PIC_Sys_MEM_Allocated;
static size_t	PIC_Sys_MEM_NbAllocs;

#if defined(__APPLE__)
#define _msize malloc_size
#elif defined(__GNUC__)
#define _msize malloc_usable_size
#endif /* __GNUC__ */

/* ---------------------------------------------------------------------------------------------------------------------------------- */

void *Pic_malloc(unsigned long size)
{
	void	*mem;
	mem=malloc(size);
	if (mem) 
	{
		PIC_Sys_MEM_Allocated+=size;
		PIC_Sys_MEM_NbAllocs++;
	}
	return(mem);
}
/* ----- */
void *Pic_calloc(size_t count, size_t size)
{
	void	*mem;
	mem=calloc(count,size);
	if (mem) 
	{
		PIC_Sys_MEM_Allocated+=(size*count);
		PIC_Sys_MEM_NbAllocs++;
	}
	return(mem);
}
/* ----- */
void Pic_free(void *memblock)
{
	size_t	size;
	size=_msize(memblock);
	PIC_Sys_MEM_Allocated-=size;
	PIC_Sys_MEM_NbAllocs--;
	free(memblock);
}
/* ----- */
size_t Pic__msize(void *memblock)
{
	return(_msize(memblock));
}
/* ----- */
size_t PIC_GetMemNbAllocs(void)
{
	return(PIC_Sys_MEM_NbAllocs);
}
/* ----- */
size_t PIC_GetMemAllocated(void)
{
	return(PIC_Sys_MEM_Allocated);
}

/* ---------------------------------------------------------------------------------------------------------------------------------- */

static unsigned char	PIC_ErrorFlag;
static unsigned char	PIC_ErrorString[PIC_ERRSIZE];
static unsigned char	PIC_Sys_FnctActive=0;
static void				(*PIC_Sys_Fnct)(void);

void Pic_SetError(const char *msg, ...)
{
	unsigned char	curerr[PIC_ERRSIZE],olderr[PIC_ERRSIZE];
	va_list			args;

	va_start(args,msg);
	vsprintf(curerr,msg,args);
	va_end(args);
	if ( (strlen(curerr)+strlen(PIC_ErrorString))>PIC_ERRSIZE ) return;

	if (PIC_ErrorFlag)
	{
		strcpy(olderr,PIC_ErrorString);
		sprintf(PIC_ErrorString,"--- [PIC#%03d] :\n%s",PIC_ErrorFlag,curerr);
		strcat(PIC_ErrorString,"\n");
		strcat(PIC_ErrorString,olderr);
	}
	else
	{
		sprintf(PIC_ErrorString,"--- [PIC#%03d] :\n%s",PIC_ErrorFlag,curerr);
	}
	PIC_ErrorFlag++;
	if (PIC_Sys_FnctActive) PIC_Sys_Fnct();
	return;
}
/* ----- */
char* PIC_GetError(void)
{
	return(PIC_ErrorString);
}
/* ----- */
unsigned char PIC_Error(void)
{
	return(PIC_ErrorFlag);
}
/* ----- */
void PIC_ResetError(void)
{
	strcpy(PIC_ErrorString,"");
	PIC_ErrorFlag=0;
}
/* ----- */
unsigned char PIC_OnErrorCall( void pFnct(void) )
{
	if (pFnct != NULL)
	{
		PIC_Sys_Fnct=pFnct;
		PIC_Sys_FnctActive=1;
	}
	else
	{
		PIC_Sys_FnctActive=0;
	}
	return(1);
}

/* ---------------------------------------------------------------------------------------------------------------------------------- */

