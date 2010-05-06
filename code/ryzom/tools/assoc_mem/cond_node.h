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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. 

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_COND_NODE_H_
#define NL_COND_NODE_H_

#include <iostream>

#include "field.h"
#include "node.h"
#include "record.h"

class ICondNode : public INode
{
	protected:
		int					_Key;
		CField				*_Field;
		std::vector<INode *> _Nodes;
	public:
		ICondNode();
		ICondNode(CField *, int);
		~ICondNode();

		void setKey(int);
		void addNode(INode *);
};

template<class T> class CEqualNode : public ICondNode {
	private:
		std::vector<T>	_Values;
	public:
		CEqualNode();
		CEqualNode(CField *, int);
		~CEqualNode();

		virtual bool propagRecord(CRecord *);
};

template<class T> CEqualNode<T>::CEqualNode() : ICondNode()
{
}

template<class T> CEqualNode<T>::CEqualNode(CField *field, int key) : ICondNode(field, key)
{
}

template<class T> CEqualNode<T>::~CEqualNode()
{
}

template<class T> bool CEqualNode<T>::propagRecord(CRecord *record)
{
	std::vector<IValue *>::const_iterator it_val = _Field->getPossibleValues().begin();
	std::vector<IValue *>::const_iterator it_end = _Field->getPossibleValues().end();
	int id_node = 0;
	while ( it_val != it_end )
	{
		if (  (  (CValue<T> *) (*record)[ _Key ] )->getValue() == ( (CValue<T> *) (*it_val))->getValue() )
		{
			std::cout << std::endl << "EQUAL_NODE (" << _Field->getName() << _Key << " = " << ((CValue<T> *)(*it_val))->getValue() << " ) -> Node " << id_node << std::endl;
			return _Nodes[id_node]->propagRecord( record );
		}
		id_node++;
		it_val++;
	}
}

/*
template<class T> class CInfNode : public ICondNode {
	private:
		T		_Value;
	public:
		CInfNode();
		~CInfNode();

		virtual bool propagRecord(CRecord *);
};

template<class T> CInfNode<T>::CInfNode() : ICondNode()
{
}

template<class T> CInfNode<T>::~CInfNode()
{
}

template<class T> bool CInfNode<T>::propagRecord(CRecord *record)
{
	if ( ( (CValue<T> *) (*record)[ _Key ] )->getValue() <= _Value )
		return _TrueNode->propagRecord( record );
	else
		return _FalseNode->propagRecord( record );
}

template<class T> class CSupNode : public ICondNode {
	private:
		T		_Value;
	public:
		CSupNode();
		~CSupNode();

		virtual bool propagRecord(CRecord *);
};

template<class T> CSupNode<T>::CSupNode() : ICondNode()
{
}

template<class T> CSupNode<T>::~CSupNode()
{
}

template<class T> bool CSupNode<T>::propagRecord(CRecord *record)
{
	if ( ( (CValue<T> *) (*record)[ _Key ] )->getValue() >= _Value )
		return _TrueNode->propagRecord( record );
	else
		return _FalseNode->propagRecord( record );
}
*/

#endif
