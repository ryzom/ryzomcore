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



#ifndef RY_STAT_DB_TREE_VISITOR_H
#define RY_STAT_DB_TREE_VISITOR_H


class CStatDBBranch;
class CStatDBValueLeaf;
class CStatDBTableLeaf;

/**
 * SDB node visitor (visitor design pattern).
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBNodeVisitor
{
public:
	virtual void visitBranch(CStatDBBranch * /* branch */, const std::string & /* path */) {}
	virtual void visitValueLeaf(CStatDBValueLeaf * /* valueLeaf */, const std::string & /* path */) {}
	virtual void visitTableLeaf(CStatDBTableLeaf * /* tableLeaf */, const std::string & /* path */) {}

protected:
	CStatDBNodeVisitor() {}
};


#endif // RY_STAT_DB_TREE_VISITOR_H
