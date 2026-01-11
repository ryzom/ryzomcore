// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef CL_INTERFACE_EXPR_H
#define CL_INTERFACE_EXPR_H


#include "nel/misc/ucstring.h"
#include "nel/misc/rgba.h"

namespace NLMISC{
	class ICDBNode;
	class CCDBNodeLeaf;
	class CCDBNodeBranch;
}

namespace NLGUI
{

	struct CInterfaceExprUserType;
	class  CInterfaceExprNode;

	/** a value that can be returned by a CInterfaceExpr instance
	  * It supports basic type;
	  * It can be extended by user defined types
	  */
	class CInterfaceExprValue
	{
	public:
		enum TType { Boolean = 0, Integer, Double, String, RGBA, UserType, NoType };
	public:
		// default ctor
		CInterfaceExprValue() : _Type(NoType) {}
		// copy ctor
		CInterfaceExprValue(const CInterfaceExprValue &other);
		// assignment operator
		CInterfaceExprValue &operator = (const CInterfaceExprValue &other);
		// dtor
		~CInterfaceExprValue() { clean(); }

		TType                  getType() const { return _Type; }
		// get. Should be used only if the type is valid
		bool                   getBool() const;
		sint64                 getInteger() const;
		double                 getDouble() const;
		const std::string	   &getString() const;
		NLMISC::CRGBA		   getRGBA() const;
		CInterfaceExprUserType *getUserType() const;
		// set
		void               setBool(bool value) { clean(); _Type = Boolean; _BoolValue = value; }
		void               setInteger(sint64 value) { clean(); _Type = Integer; _IntegerValue = value; }
		void               setDouble(double value) { clean(); _Type = Double; _DoubleValue = value; }
		void               setString(const std::string &value) { clean(); _Type = String; _StringValue = value; }
		void               setString(const char *value) { clean(); _Type = String; _StringValue = value; }
		void			   setRGBA(NLMISC::CRGBA value) { clean(); _Type = RGBA; _RGBAValue = (uint32)(value.R+(value.G<<8)+(value.B<<16)+(value.A<<24)); }
		void               setUserType(CInterfaceExprUserType *value);
		// reset this object to initial state (no type)
		void			   clean();
		// conversions. They return true if success
		bool               toBool();
		bool               toInteger();
		bool               toDouble();
		bool               toString();
		bool			   toType(TType type);
		bool			   toRGBA();
		// test if the value if a bool, double, or integer
		bool               isNumerical() const;
		/** evaluate a from a string
		  * \param expr : where to start the evaluation
		  * \return the position following the token, or NULL if the parsing failed
		  */
		const char        *initFromString(const char *expr);

	/////////////////////////////////////////////////////////////////////////////////////////////
	private:
		TType	_Type;
		union
		{
			bool					_BoolValue;
			sint64					_IntegerValue;
			double					_DoubleValue;
			CInterfaceExprUserType *_UserTypeValue;
			uint32					_RGBAValue;
		};
		std::string					_StringValue; // well, can't fit in union, unless we do some horrible hack..
	private:
		const char *evalBoolean(const char *expr);
		const char *evalNumber(const char *expr);
		const char *evalString(const char *expr);
	};

	/**
	  * Base class for user defined types that are use by the 'CInterfaceExprValue' class
	  * Derivers should include the 'clone' method
	  *
	  * CInterfaceExprValue instances have ownership of this object.
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 2003
	  */
	struct CInterfaceExprUserType
	{
		// cloning method
		virtual CInterfaceExprUserType *clone() const = 0;
		// dtor
		virtual ~CInterfaceExprUserType() {}
	};


	/** Evaluate expressions used in interface.
	  * It can retrieve values from the database.
	  * It can also build a list of database values it depends of.
	  *
	  * An expression can be :
	  *
	  * - a string : 'toto', 'abcd', 'a\nbcd', 'a\\t', the escape sequences are the one of C
	  * - a integer 1, 2, 3
	  * - a double  1.1, 2.2
	  * - a database entry : @ui:interface:toto:truc. If the address is a leaf, it returns the leaf value and put an observer on it. If not a leaf, it returns 0, but put an observer on it.
	  * - a database indirection @db:value[db:index] is replaced by @db:value0 if db:index == 0 for example
	  * - a user function call : fct(expr0, epxr1, ...).
	  *
	  * NB : The lua language has been integrated since then (2005), and should be more suited
	  *      for most of the tasks.
	  *
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 2002
	  */
	class CInterfaceExpr
	{
	public:
		// list of argument for a function
		typedef std::vector<CInterfaceExprValue> TArgList;
		/** prototype of a user callable function
		  * It should return true if the result is meaningful. If not, the rest of the evaluation is stopped
		  */
		typedef bool (* TUserFct) (TArgList &args, CInterfaceExprValue &result);
	public:

		// release memory
		static void release();

		/** This try to eval the provided expression.
		  * - This returns a result
		  * - This eventually fill a vector with a set of database entries it has dependencies on
		  * \param expr The expression to evaluate
		  * \param result The result value
		  * \param nodes If not NULL, will be filled with the database nodes this expression depends on
		  *              Node will only be inserted once, so we end up with a set of node (not ordered)
		  * \param noFctCalls when set to true, the terminal function calls will not be made, so the evaluation is only used to see which database entries the expression depends on.
		  */
		static bool eval(const std::string &expr, CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> *nodes = NULL, bool noFctCalls = false);

		/** Build a tree from the given expression so that it can be evaluated quickly.
		  * This is useful for a fixed expression that must be evaluated often
		  */
		static CInterfaceExprNode *buildExprTree(const std::string &expr);


		/** Register a function that can have several arguments
		  * // NB : this is case sensitive
		  */
		static void registerUserFct(const char *name, TUserFct fct);
		// Simple evaluations
		static bool evalAsInt(const std::string &expr, sint64 &dest);
		static bool evalAsDouble(const std::string &expr, double &dest);
		static bool evalAsBool(const std::string &expr, bool &dest);
		static bool evalAsString(const std::string &expr, std::string &dest);
	/////////////////////////////////////////////////////////////////////////////////////////////
	private:
		// map of user functions
		typedef std::map<std::string, TUserFct> TUserFctMap;
	private:
		static TUserFctMap *_UserFct;
	private:
		/** eval the value of a single expression
		  * \return position to the next valid character
		  */
		static const char *evalExpr(const char *expr, CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> *nodes, bool noFctCalls);
		static const char *evalFct(const char *expr,CInterfaceExprValue &result,std::vector<NLMISC::ICDBNode *> *nodes, bool noFctCalls);
		static const char *evalDBEntry(const char *expr,CInterfaceExprValue &result,std::vector<NLMISC::ICDBNode *> *nodes);
	public:
		static const char *unpackDBentry(const char *expr, std::vector<NLMISC::ICDBNode *> *nodes, std::string &dest, bool *hasIndirections = NULL);

		/** Build tree of a single expression
		  * \return position to the next valid character
		  */
	private:
		static const char *buildExprTree(const char *expr,   CInterfaceExprNode *&result);
		static const char *buildFctNode(const char *expr,    CInterfaceExprNode *&result);
		static const char *buildDBEntryNode(const char *expr,CInterfaceExprNode *&result);
	};


	// helper macro to register user functions at startup
	#define REGISTER_INTERFACE_USER_FCT(name, fct) \
	const struct __InterUserFctRegister__##fct\
	{\
		__InterUserFctRegister__##fct() { CInterfaceExpr::registerUserFct(name, fct); }\
	} __InterUserFctRegisterInstance__##fct;


	// helper macro to declare a user function
	// the code must follow
	// arguments are available in 'args', result should be put in 'result'
	#define DECLARE_INTERFACE_USER_FCT(name) \
	bool name(CInterfaceExpr::TArgList &args, CInterfaceExprValue &result)


	// helper macro to declare a C constant mirroring
	#define DECLARE_INTERFACE_CONSTANT(_name, _cconst)	\
	static DECLARE_INTERFACE_USER_FCT(_name)			\
	{													\
		result.setInteger(_cconst);						\
		return true;									\
	}													\
	REGISTER_INTERFACE_USER_FCT(#_name, _name)

}

#endif

