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

//-------------------------------------------------------------------------------------------------
// project: sadge lib
// file:	text_output.h
// version:	$Id: text_output.h,v 1.1 2004/02/10 17:54:46 boucher Exp $	
//
// This file contains a string class derived from the STL string
// The string compare functions od the class are case insensitive 
//
// The coding style is not CPU efficent - the routines are not designed for performance
//-------------------------------------------------------------------------------------------------

#ifndef SADGE_TEXT_OUTPUT_H
#define SADGE_TEXT_OUTPUT_H

//-------------------------------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/log.h"
#include "nel/misc/sstring.h"

#include <stdio.h>

//----------------------------------------------------------------------
// CTextOutput - an object that encapsulates output
//----------------------------------------------------------------------

class CTextOutput
{
public:
	static CTextOutput Nul;			// an empty tet output object
	static CTextOutput StdErr;		// an object that spews out to the stderr stream
	static CTextOutput StdOut;		// an object that spews out to the stdout stream
	static CTextOutput NelInfo;		// an object that spews out to nlinfo
	static CTextOutput NelDebug;	// an object that spews out to nldebug
	static CTextOutput NelWarning;	// an object that spews out to nlwarning
	static CTextOutput NelError;	// an object that spews out to nlerror
	static CTextOutput NelAssert;	// an object that spews out to nlassert

public:
	enum { NUL=0, ECHO_STDOUT=1, ECHO_STDERR=2, RECORD=4 };
	enum TSpecialCodes { HEX };

	CTextOutput(uint flags=ECHO_STDOUT, NLMISC::CLog* log=NULL)
	{
		_initData();
		_Echo=			(flags&ECHO_STDOUT)!=0;
		_EchoStdErr=	(flags&ECHO_STDERR)!=0;
		_Record=		(flags&RECORD)!=0;
		_MiscLog=		log;
	}

	CTextOutput(CTextOutput& indirect)
	{
		_initData();
		_Indirect=&indirect;
	}

	~CTextOutput()
	{
//		_push('\n');
	}

	bool empty()
	{
		if (this==NULL)
			return true;
		if (_Indirect!=NULL)
			return _Indirect->empty();

		return _RecordBuff.empty();
	}

	void clear(CTextOutput *warnChannel=NULL,bool warnNotEmpty=false)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->clear(warnChannel,warnNotEmpty);
			return;
		}

		if (warnNotEmpty && warnChannel != NULL && !(_RecordBuff.empty()))
			*warnChannel<<"WARNING: CTextOutput::clear(): some content in channel was lost...\n";

		_RecordBuff.clear();
	}

	void write(FILE *outf,bool resetAfterWrite=true)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->write(outf,resetAfterWrite);
			return;
		}

		// write the buffer and close the file
		fwrite(_RecordBuff.data(), 1, _RecordBuff.size(), outf);

		// clear the buffer
		if (resetAfterWrite)
			_RecordBuff.clear();
	}

	void write(const NLMISC::CSString& fileName,bool resetAfterWrite=true)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->write(fileName,resetAfterWrite);
			return;
		}

		// open the output file
		FILE *outf=fopen(fileName.c_str(),"wt");
		if (outf==NULL)
		{
			fprintf(stderr,"Failed to open file for output: %s\n",fileName.c_str());
			return;
		}

		// write the buffer and close the file
		write(outf,resetAfterWrite);
		fclose(outf);
	}

	void display(bool resetAfterDisplay=false)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->display(resetAfterDisplay);
			return;
		}

		// write the buffer to the console
		write(stdout,resetAfterDisplay);
	}

	void setEchoStdOut(bool echo)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->setEchoStdOut(echo);
			return;
		}

		_Echo=echo;
	}

	void setEchoStdErr(bool echo)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->setEchoStdErr(echo);
			return;
		}

		_EchoStdErr=echo;
	}

	void setRecord(bool record,bool clearBuffer=false)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->setRecord(record,clearBuffer);
			return;
		}

		_Record=record;

		if (clearBuffer)
			_RecordBuff.clear();
	}

	void setMiscLog(NLMISC::CLog* log)
	{
		if (this==NULL)
			return;
		if (_Indirect!=NULL)
		{
			_Indirect->setMiscLog(log);
			return;
		}

		_MiscLog=log;
	}

	template <class C> CTextOutput &operator<<(const C& code)
	{
		if (this==NULL)
			return *this;

		if (_Indirect!=NULL)
		{
			return *_Indirect<<code;
		}

		_push(code);
		return *this;
	}

private:
	void _push(TSpecialCodes code)
	{
		switch (code)
		{
		case HEX:
			_ModeHex=true;
			break;

		default:
			fprintf(stderr,"\n"__FILE__":%d: BUG: invlid special code in CTextOutput object: %d\n",__LINE__,(int)code);
		}
	}

	void _push(const std::string &s)	{ _pushSString(s); }
	void _push(const char *s)			{ _pushString(s); }
	void _push(char c)					{ _pushChar(c); }

//	void _push(unsigned u)	{ _pushUnsigned(u,"%08X"); }
	void _push(uint32 u)	{ _pushUnsigned(u,"%08X"); }
	void _push(uint16 u)	{ _pushUnsigned(u,"%04X"); }
	void _push(uint8  u)	{ _pushUnsigned(u,"%02X"); }

//	void _push(int i)		{ _pushSigned(i,"%08X"); }
	void _push(sint32 i)	{ _pushSigned(i,"%08X"); }
	void _push(sint16 i)	{ _pushSigned(i,"%04X"); }
	void _push(sint8  i)	{ _pushSigned(i,"%02X"); }

	void _push(double d)	{ _pushFloat(d); }

	void _push(uint64 u)	
	{
		if (_ModeHex)
		{
			_pushHex(unsigned(u>>32),"%08X");
			_pushHex(unsigned(u),"%08X");
			return; 
		}

		uint32 tera=	uint32(u/1000000/1000000); 		u-=	tera*1000000*1000000;
		uint32 mega=	uint32(u/1000000);			   	u-=	mega*1000000;
		uint32 units=	uint32(u);					   	u-=	units;

		if (tera!=0) 
		{
			_pushUnsigned(tera,"");
			_pushUnsigned(mega,"",6);
			_pushUnsigned(units,"",6);
		}
		else if (mega!=0) 
		{
			_pushUnsigned(mega,"");
			_pushUnsigned(units,"",6);
		}
		else
		{
			_pushUnsigned(units,"");
		}
	}

	void _push(sint64 i)	
	{
		if (_ModeHex || i>=0)
		{
			_push((uint64) i);
		}	
		else
		{
			_push('-');
			_push((uint64) -i);
		}
	}

	void _pushSString(const std::string &s)
	{
		_pushValue("%s",s.c_str());
	}
	
	void _pushString(const char *s)
	{
		_pushValue("%s",s);
	}
	
	void _pushChar(char c)
	{
		if (_ModeHex)
		{
			_pushHex((int)c,"%02X");
		}
		else
		{
			_pushValue("%c",c);
		}
	}
	
	void _pushUnsigned(unsigned u,const char* hexFmt,int padding=0)
	{
		_pushInt((int)u,'d',hexFmt,padding);
	}
	
	void _pushSigned(int i,const char* hexFmt,int padding=0)
	{
		_pushInt(i,'d',hexFmt,padding);
	}

	void _pushHex(int i,const char* hexFmt)
	{
		_pushValue(hexFmt,i);
		_ModeHex=false;
	}

	void _pushInt(int i,char fmtChar,const char* hexFmt,int padding=0)
	{
		if (_ModeHex)
		{
			_pushHex(i,hexFmt);
			return;
		}

		if (padding==0)
		{
			char fmt[]="% ";
			fmt[1]=fmtChar;
			_pushValue(fmt,i);
		}
		else
		{
			NLMISC::CSString fmt="%0"+NLMISC::CSString(padding)+fmtChar;
			_pushValue(fmt.c_str(),i);
		}
	}
	
	void _pushFloat(double d)
	{
		_pushValue("%f",d);
	}

	template <class C> void _pushValue(const char* fmt,const C& val)
	{
		if (_Echo) 			printf(fmt,val);
		if (_EchoStdErr)	fprintf(stderr,fmt,val);
		if (_Record)		_RecordBuff+=NLMISC::toString(fmt,val);
		if (_MiscLog!=NULL)
		{
			_MiscLogString+=NLMISC::toString(fmt,val);
			while (_MiscLogString.contains("\n"))
			{
				_MiscLog->displayNL("%s",_MiscLogString.splitTo('\n').c_str());
				_MiscLogString=_MiscLogString.splitFrom('\n');
			}
		}
	}

	void _initData()
	{
		_Echo=false;
		_EchoStdErr=false;
		_Record=false;

		_MiscLog=NULL;
		_Indirect=NULL;

		_ModeHex=false;
	}


private:
	bool _EchoStdErr;
	bool _Echo;
	bool _Record;

	NLMISC::CLog* _MiscLog;
	NLMISC::CSString _MiscLogString;
	CTextOutput*  _Indirect;

	bool _ModeHex;

	NLMISC::CSString _RecordBuff;
};


#endif
