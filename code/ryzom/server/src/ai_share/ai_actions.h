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



// forward declaration of class useful in the case of circular class refferences
#ifndef RYAI_AI_ACTIONS_H
#define RYAI_AI_ACTIONS_H

#include "game_share/persistent_data.h"

#include "ai_alias_description_node.h"
#include "ai_event_description.h"
#include "ai_event_description.h"

/*
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/stream.h"


#include <map>
#include <vector>
#include <string>
*/

class CAIActions
{
public:
	class CArg
	{
	public:
		CArg():							_type(TypeInt),	_i(0)	{}
		CArg(const CArg &other):		_type(TypeInt),	_i(0)	{ *this=other; }

#ifdef NL_COMP_VC6
		CArg(sint arg):					_type(TypeInt),	_i(arg)	{}
		CArg(uint arg):					_type(TypeInt),	_i(arg) {}
#endif
		CArg(bool arg):					_type(TypeBool),_b(arg) {}
		CArg(sint32 arg):				_type(TypeInt),	_i(arg) {}
		CArg(uint32 arg):				_type(TypeInt),	_i(arg) {}
		CArg(float arg):				_type(TypeFloat), _f(arg) {}
		CArg(double arg):				_type(TypeFloat), _f((float)arg) {}
		CArg(const char *arg):			_type(TypeString) {	_s=arg; }
		CArg(const std::string &arg):	_type(TypeString) {	_s=arg; }
		CArg(const CAIAliasDescriptionNode *arg):	_type(TypeAliasTree) {	_a=const_cast<CAIAliasDescriptionNode *>(arg);	}	//	->copyAliasDescriptionNode(); }
		CArg(const CAIEventDescription *arg):		_type(TypeEventTree) {	_e=const_cast<CAIEventDescription *>(arg); }

		~CArg()	{}

#ifdef NL_COMP_VC6
		bool get(sint &arg)			const { if (_type!=TypeInt)	return false; arg=_i; return true; } 
		bool get(uint &arg)			const { if (_type!=TypeInt)	return false; arg=_i; return true; } 
#endif
		bool get(bool &arg)			const { if (_type!=TypeBool)return false; arg=_b; return true; } 
		bool get(sint32 &arg)		const { if (_type!=TypeInt)	return false; arg=_i; return true; } 
		bool get(uint32 &arg)		const { if (_type!=TypeInt)	return false; arg=_i; return true; } 
		bool get(float &arg)		const { if (_type!=TypeFloat)	return false; arg=_f; return true; }
		bool get(double &arg)		const { if (_type!=TypeFloat)	return false; arg=_f; return true; }
		bool get(std::string &arg)	const { if (_type!=TypeString)	return false; arg=_s; return true; }
		bool get(CAIAliasDescriptionNode *&arg) const	{ if (_type!=TypeAliasTree) return false; arg=_a; return true; }
		bool get(CAIEventDescription::TSmartPtr &arg) const	{ if (_type!=TypeEventTree) return false; arg=_e; return true; }

		const CArg &operator =(const CArg &other)
		{
			_type = other._type;
			switch(other._type)
			{
			case TypeString:	_s=other._s;	break;
			case TypeBool:		_b=other._b;	break;
			case TypeInt:		_i=other._i;	break;
			case TypeFloat:		_f=other._f;	break;
			case TypeAliasTree:	_a=other._a;	break;	//->copyAliasDescriptionNode();	break;
			case TypeEventTree: _e=other._e;	break;
			}
			return *this;
		}

		std::string toString() const
		{
			switch(_type)
			{
			case TypeString:	return NLMISC::toString(_s);
			case TypeBool:		return NLMISC::toString(_b);
			case TypeInt:		return NLMISC::toString(_i);
			case TypeFloat:		return NLMISC::toString(_f);
//			case TypeAliasTree:	return _a->treeToString();
//			case TypeEventTree: return _e->toString();
			default:			break;
			}
			return "<Invalid Argument Type>";
		}

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialEnum(_type);
			switch(_type)
			{
				case TypeString:	f.serial(_s);	break;
				case TypeBool:		f.serial(_b);	break;
				case TypeInt:		f.serial(_i);	break;
				case TypeFloat:		f.serial(_f);	break;
//				case TypeAliasTree:	f.serial(_a);	break;
//				case TypeEventTree: f.serial(_e);	break;
				default:			break;
			}
		}

		void pushToPdr(CPersistentDataRecord& pdr) const
		{
			switch(_type)
			{
			case TypeString:	pdr.push(pdr.addString("string"),_s); break;
			case TypeBool:		pdr.push(pdr.addString("bool"),_b?(sint32)1:(sint32)0); break;
			case TypeInt:		pdr.push(pdr.addString("int"),_i); break;
			case TypeFloat:		pdr.push(pdr.addString("float"),_f); break;
			case TypeAliasTree:
				pdr.pushStructBegin(pdr.addString("aliasNodeTree"));
				_a->pushToPdr(pdr);
				pdr.pushStructEnd(pdr.addString("aliasNodeTree"));
				break;
			case TypeEventTree:
				pdr.pushStructBegin(pdr.addString("eventHandler"));
				_e->pushToPdr(pdr);
				pdr.pushStructEnd(pdr.addString("eventHandler"));
				break;
			}
		}

		void popFromPdr(CPersistentDataRecord& pdr)
		{
			uint16 token= pdr.peekNextToken();
			const std::string& tokenName= pdr.peekNextTokenName();

			if (pdr.isStartOfStruct())
			{
				if (tokenName=="aliasNodeTree")
				{
					_type = TypeAliasTree;
					pdr.popStructBegin(token);
					_a= CAIAliasDescriptionNode::popFromPdr(pdr);
					pdr.popStructEnd(token);
					return;
				}
				if (tokenName=="eventHandler")
				{
					_type = TypeEventTree;
					pdr.popStructBegin(token);
					_e= CAIEventDescription::popFromPdr(pdr);
					pdr.popStructEnd(token);
					return;
				}
				WARN("Found unexpected clause in PDR: "+tokenName);
				pdr.skipData();
			}
			else
			{
				if (tokenName=="string")	
				{ 
					_type = TypeString;
					_s=			pdr.popNextArg(token).asString();
					return; 
				}
				if (tokenName=="bool")
				{ 
					_type = TypeBool;
					_b=			pdr.popNextArg(token).asSint()!=0;
					return; 
				}
				if (tokenName=="int")
				{ 
					_type = TypeInt;
					_i= (sint32)	pdr.popNextArg(token).asSint();
					return; 
				}
				if (tokenName=="float")	
				{
					_type = TypeFloat;
					_f = pdr.popNextArg(token).asFloat();
					return; 
				}

				WARN("Found unexpected value in PDR: "+tokenName);
				pdr.skipStruct();
			}
		}

		// a kind of serial used to generate messages from AIDS to AIS
		void serialToString(std::string &s) throw()
		{
			s+=char(_type);
			switch(_type)
			{
				case TypeString:
					s+=_s+char(0);
					break;

				case TypeBool:
					s+=_b ? char(1) : char(0);
					break;

				case TypeInt:
					s+=NLMISC::toString("%*s",sizeof(sint32),"");
					((sint32*)&(s[s.size()]))[-1]=_i;
					break;

				case TypeFloat:
					s+=NLMISC::toString("%*s",sizeof(float),"");
					((float*)&(s[s.size()]))[-1]=_f;
					break;

				case TypeAliasTree:
					// TODO: LOADS OF WORK HERE!!!
					// count the nodes
					// build string table
					// sort string table by size
					// compact string table by removing strings that match tail end of existing strings
					// - use an output buffer - containing all existing strings	(directly in output string?)
					// - a buffer index containing indices where strings end (start is implicit :o)
					// - a heap of node indices, sorted by heapEntry->_name.size()
					// - a vector of string start pointers corresponding to the nodes 
					// resize s to accomodate arriving data
					// write node count
					// write string buffer size
					// write the nodes
					// write string buffer

					break;

				case TypeEventTree:
					// TODO: LOADS OF WORK HERE TOO!!!
					break;
			}
		}

		// a kind of serial used to decrypt messages from AIDS to AIS
		static CArg serialFromString(std::string &data,uint &index) throw()
		{
			// first get hold of the type (a single char)
			if (data.size()-index<1)
			{
				nlwarning("Unexpected end of input data in serialFromString()");
				return CArg();
			}
			TType type=(TType)data[index++]; 

			// now grab the value
			switch(type)
			{
				case TypeString: // strings are stored in asciiz
				{
					char *str=&data[index];
					for(;data[index]!=0;++index)
					{
						if (index>=data.size())
						{
							nlwarning("Unexpected end of input data in serialFromString()");
							index=(uint)data.size()+1;
							return CArg();
						}
					}
					return CArg(str);
				}

				case TypeBool:	// bool are stored directly on one char
					index+=1;
					if (index>data.size())
					{
						nlwarning("Unexpected end of input data in serialFromString()");
						index=(uint)data.size()+1;
						return CArg();
					}
					return CArg(bool(data[index] == 1));

				case TypeInt:	// ints are stored drectly as binary (beware of byte order!!!)
					index+=sizeof(sint32);
					if (index>data.size())
					{
						nlwarning("Unexpected end of input data in serialFromString()");
						index=(uint)data.size()+1;
						return CArg();
					}
					return CArg(((sint32 *)&data[index])[-1]);

				case TypeFloat:	// floats are stored directly as binary (beware of byte order!!!)
					index+=sizeof(float);
					if (index>data.size())
					{
						nlwarning("Unexpected end of input data in serialFromString()");
						index=(uint)data.size()+1;
						return CArg();
					}
					return CArg(((float *)&data[index])[-1]);

				case TypeAliasTree:
					// TODO: LOADS OF WORK HERE!!!
					// return CArg(((float *)&data[index])[-1]);
					break;

				case TypeEventTree:
					// TODO: LOADS OF WORK HERE!!!
					// return CArg(((float *)&data[index])[-1]);
					break;
			}
			nlwarning("Unexpected type in serialFromString()");
			index=(uint)data.size()+1;
			return CArg();
		}

	private:
		enum TType { TypeString, TypeInt, TypeFloat, TypeBool, TypeAliasTree, TypeEventTree } _type;
		bool	_b;
		sint32	_i;
		float	_f;
		std::string _s;
		CAIAliasDescriptionNode			*_a;
		CAIEventDescription::TSmartPtr	_e;
/*
		// I've had to re-code strdup() here as the default one doesn't use the 
		// same memory manager as free() and so causes crashes in debug	mode
		static char *strdup(const char *src)
		{
			// calculate strlen() - *NOTE: i must be signed for use in copy (below)
			sint i;
			for (i=0;src[i];++i);
			// allocate memory
			char *result=(char *)malloc(i+1);
			if (result==NULL)
				return NULL;
			// copy string into new buffer
			for (;i>=0;i--)
				result[i]=src[i];
			return result;
		}
*/
	};

	class IExecutor
	{
	public:
		virtual ~IExecutor() {}

		virtual void openFile(const std::string &fileName) {}
		virtual void closeFile(const std::string &fileName) {}
		virtual void begin(uint32 contextAlias) =0;
		virtual void end(uint32 contextAlias) =0;
		virtual void execute(uint64 action,const std::vector <CArg> &args)=0;
	};

	static void init(IExecutor *executor) { _init=true; _executor=executor;}
	static IExecutor*  getExecuter() { return _init?_executor:0;}
	static void	release() { _init=false; }

	static void openFile(const std::string &fileName);
	static void closeFile(const std::string &fileName);
	static void begin(uint32 contextAlias);
	static void end(uint32 contextAlias);

	static void execute(const std::string &action,const std::vector<CArg> &args);
	static void execute(const std::string &action,const std::vector<std::string> &args);
	static void execute(const std::string &action,const std::vector<int> &args);
	static void execute(const std::string &action,const std::vector<double> &args);
	static void execute(const std::string &action);

	static void exec(const std::string &action)
	{
		std::vector<CArg> args;
		execute(action,args);
	}	
	template <class T0>
	static void exec(const std::string &action,const T0 &arg0)
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		execute(action,args);
	}	
	template <class T0,class T1>
	static void exec(const std::string &action,const T0 &arg0,const T1 &arg1)
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		args.push_back(CArg(arg1));
		execute(action,args);
	}	
	template <class T0,class T1,class T2>
	static void exec(const std::string &action,const T0 &arg0,const T1 &arg1,const T2 &arg2)	
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		args.push_back(CArg(arg1));
		args.push_back(CArg(arg2));
		execute(action,args);
	}	
	template <class T0,class T1,class T2,class T3>
	static void exec(const std::string &action,const T0 &arg0,const T1 &arg1,const T2 &arg2,const T3 &arg3)	
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		args.push_back(CArg(arg1));
		args.push_back(CArg(arg2));
		args.push_back(CArg(arg3));
		execute(action,args);
	}	
	template <class T0,class T1,class T2,class T3,class T4>
	static void exec(const std::string &action,const T0 &arg0,const T1 &arg1,const T2 &arg2,const T3 &arg3,const T4 &arg4)	
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		args.push_back(CArg(arg1));
		args.push_back(CArg(arg2));
		args.push_back(CArg(arg3));
		args.push_back(CArg(arg4));
		execute(action,args);
	}	
	template <class T0,class T1,class T2,class T3,class T4,class T5>
	static void exec(const std::string &action,const T0 &arg0,const T1 &arg1,const T2 &arg2,const T3 &arg3,const T4 &arg4,const T5 &arg5)	
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		args.push_back(CArg(arg1));
		args.push_back(CArg(arg2));
		args.push_back(CArg(arg3));
		args.push_back(CArg(arg4));
		args.push_back(CArg(arg5));
		execute(action,args);
	}	
	template <class T0,class T1,class T2,class T3,class T4,class T5,class T6>
		static void exec(const std::string &action,const T0 &arg0,const T1 &arg1,const T2 &arg2,const T3 &arg3,const T4 &arg4,const T5 &arg5, const T6 &arg6)	
	{
		std::vector<CArg> args;
		args.push_back(CArg(arg0));
		args.push_back(CArg(arg1));
		args.push_back(CArg(arg2));
		args.push_back(CArg(arg3));
		args.push_back(CArg(arg4));
		args.push_back(CArg(arg5));
		args.push_back(CArg(arg6));
		execute(action,args);
	}	
	private:
	static bool _init;
	static IExecutor *_executor;
};

#endif

