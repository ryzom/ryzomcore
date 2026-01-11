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
 */

#ifndef NL_VAL_NODES_H_
#define NL_VAL_NODES_H_

template<class T> class CInfValNode : public INode<T>
{
	private:
		T	*_Val
	public:
		INode();
		virtual const INode<T> *getTrueNode();
		virtual const INode<T> *getFalseNode();
		virtual bool propagRecord(CRecord *);
};


template<class T> const INode<T> *INode<T> *CInfValNode<T>::getTrueNode()
{
	return _TrueNode;
}

template<class T> const INode<T> *INode<T> *CInfValNode<T>::getFalseNode()
{
	return _FalseNode;
}

virtual template<class T> bool CInfValNode<T>::propagRecord(CRecord *)
{
	return false;
}

#endif