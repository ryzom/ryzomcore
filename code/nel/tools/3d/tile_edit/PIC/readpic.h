

#ifndef	_READPIC_H_
#define	_READPIC_H_

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


#include <string>
#include <vector>

#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>

//============================================================
// API.
//============================================================


bool	PIC_LoadPic(std::string Path, std::vector<NLMISC::CBGRA> &Tampon, uint &Width, uint &Height);



#endif