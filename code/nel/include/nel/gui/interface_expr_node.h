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



#ifndef CL_INTERFACE_EXPR_NODE_H
#define CL_INTERFACE_EXPR_NODE_H

#include "interface_expr.h"

namespace NLGUI
{

	/** Base node of an interface expression parse tree
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 2003
	  */
	class CInterfaceExprNode
	{
	public:
		virtual ~CInterfaceExprNode() {}
		// eval result of expression, and eventually get the nodes the epression depends on
		virtual void eval(CInterfaceExprValue &result) = 0;
		// The same, but get db nodes the expression depends on (appended to vector)
		virtual void evalWithDepends(CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> &nodes) = 0;
		// Get dependencies of the node (appended to vector)
		virtual void getDepends(std::vector<NLMISC::ICDBNode *> &nodes) = 0;
	};


	// *******************************************************************************************************
	/** A constant value already parsed by interface (in a interface expr parse tree)
	  */
	class CInterfaceExprNodeValue : public CInterfaceExprNode
	{
	public:
		CInterfaceExprValue Value;
	public:
		virtual void eval(CInterfaceExprValue &result);
		virtual void evalWithDepends(CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> &nodes);
		virtual void getDepends(std::vector<NLMISC::ICDBNode *> &nodes);
	};

	// *******************************************************************************************************
	/** A fct call (in a interface expr parse tree)
	  */
	class CInterfaceExprNodeValueFnCall : public CInterfaceExprNode
	{
	public:
		CInterfaceExpr::TUserFct Func;
		// list of parameters
		std::vector<CInterfaceExprNode *> Params;
	public:
		virtual void eval(CInterfaceExprValue &result);
		virtual void evalWithDepends(CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> &nodes);
		virtual void getDepends(std::vector<NLMISC::ICDBNode *> &nodes);
		virtual ~CInterfaceExprNodeValueFnCall();
	};

	// *******************************************************************************************************
	/** A db leaf read (in a interface expr parse tree)
	  */
	class CInterfaceExprNodeDBLeaf : public CInterfaceExprNode
	{
	public:
		class NLMISC::CCDBNodeLeaf *Leaf;
	public:
		virtual void eval(CInterfaceExprValue &result);
		virtual void evalWithDepends(CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> &nodes);
		virtual void getDepends(std::vector<NLMISC::ICDBNode *> &nodes);
	};

	// *******************************************************************************************************
	/** A db branch read (in a interface expr parse tree)
	  */
	class CInterfaceExprNodeDBBranch : public CInterfaceExprNode
	{
	public:
		class NLMISC::CCDBNodeBranch *Branch;
	public:
		virtual void eval(CInterfaceExprValue &result);
		virtual void evalWithDepends(CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> &nodes);
		virtual void getDepends(std::vector<NLMISC::ICDBNode *> &nodes);
	};

	// *******************************************************************************************************
	/** A dependant db read (in a interface expr parse tree)
	  * This is rarely used so no real optim there..
	  */
	class CInterfaceExprNodeDependantDBRead : public CInterfaceExprNode
	{
	public:
		std::string Expr;
	public:
		virtual void eval(CInterfaceExprValue &result);
		virtual void evalWithDepends(CInterfaceExprValue &result, std::vector<NLMISC::ICDBNode *> &nodes);
		virtual void getDepends(std::vector<NLMISC::ICDBNode *> &nodes);
	};


}


#endif
