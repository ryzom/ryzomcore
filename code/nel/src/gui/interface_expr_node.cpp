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


#include "stdpch.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/cdb_branch.h"
#include "nel/gui/interface_expr_node.h"

using NLMISC::ICDBNode;
using NLMISC::CCDBNodeBranch;
using NLMISC::CCDBNodeLeaf;

namespace NLGUI
{

	// *******************************************************************************************************
	void CInterfaceExprNodeValue::eval(CInterfaceExprValue &result)
	{
		result = Value;
	}

	void CInterfaceExprNodeValue::evalWithDepends(CInterfaceExprValue &result, std::vector<ICDBNode *> &/* nodes */)
	{
		result = Value;
	}

	void CInterfaceExprNodeValue::getDepends(std::vector<ICDBNode *> &/* nodes */)
	{
	}


	// *******************************************************************************************************
	void CInterfaceExprNodeValueFnCall::eval(CInterfaceExprValue &result)
	{
		nlassert(Func);
		uint numParams = (uint)Params.size();
		std::vector<CInterfaceExprValue> params(numParams);
		for(uint k = 0; k < numParams; ++k)
		{
			Params[k]->eval(params[k]);
		}
		Func(params, result); // do actual call
	}

	void CInterfaceExprNodeValueFnCall::evalWithDepends(CInterfaceExprValue &result, std::vector<ICDBNode *> &nodes)
	{
		nlassert(Func);
		uint numParams = (uint)Params.size();
		std::vector<CInterfaceExprValue> params(numParams);
		for(uint k = 0; k < numParams; ++k)
		{
			Params[k]->evalWithDepends(params[k], nodes);
		}
		Func(params, result); // do actual call
	}

	void CInterfaceExprNodeValueFnCall::getDepends(std::vector<ICDBNode *> &nodes)
	{
		uint numParams = (uint)Params.size();
		for(uint k = 0; k < numParams; ++k)
		{
			Params[k]->getDepends(nodes);
		}
	}


	CInterfaceExprNodeValueFnCall::~CInterfaceExprNodeValueFnCall()
	{
		for(uint k = 0; k < Params.size(); ++k)
		{
			delete Params[k];
		}
	}

	// *******************************************************************************************************
	void CInterfaceExprNodeDBLeaf::eval(CInterfaceExprValue &result)
	{
		nlassert(Leaf);
		result.setInteger(Leaf->getValue64());
	}

	void CInterfaceExprNodeDBLeaf::evalWithDepends(CInterfaceExprValue &result,std::vector<ICDBNode *> &nodes)
	{
		nlassert(Leaf);
		result.setInteger(Leaf->getValue64());
		if (std::find(nodes.begin(), nodes.end(), Leaf) == nodes.end())
		{
			nodes.push_back(Leaf);
		}
	}

	void CInterfaceExprNodeDBLeaf::getDepends(std::vector<ICDBNode *> &nodes)
	{
		nlassert(Leaf);
		if (std::find(nodes.begin(), nodes.end(), Leaf) == nodes.end())
		{
			nodes.push_back(Leaf);
		}
	}


	// *******************************************************************************************************
	void CInterfaceExprNodeDBBranch::eval(CInterfaceExprValue &result)
	{
		nlassert(Branch);
		result.setInteger(0);
	}

	void CInterfaceExprNodeDBBranch::evalWithDepends(CInterfaceExprValue &result,std::vector<ICDBNode *> &nodes)
	{
		nlassert(Branch);
		result.setInteger(0);
		if (std::find(nodes.begin(), nodes.end(), Branch) == nodes.end())
		{
			nodes.push_back(Branch);
		}
	}

	void CInterfaceExprNodeDBBranch::getDepends(std::vector<ICDBNode *> &nodes)
	{
		nlassert(Branch);
		if (std::find(nodes.begin(), nodes.end(), Branch) == nodes.end())
		{
			nodes.push_back(Branch);
		}
	}


	// *******************************************************************************************************
	void CInterfaceExprNodeDependantDBRead::eval(CInterfaceExprValue &result)
	{
		// no gain there, but barely used
		CInterfaceExpr::eval(Expr, result);
	}

	void CInterfaceExprNodeDependantDBRead::evalWithDepends(CInterfaceExprValue &result, std::vector<ICDBNode *> &nodes)
	{
		CInterfaceExpr::eval(Expr, result, &nodes);
	}

	void CInterfaceExprNodeDependantDBRead::getDepends(std::vector<ICDBNode *> &nodes)
	{
		CInterfaceExprValue dummyResult;
		CInterfaceExpr::eval(Expr, dummyResult, &nodes, true);
	}

}

