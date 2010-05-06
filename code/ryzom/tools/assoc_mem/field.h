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

#ifndef NL_FIELD_H_
#define NL_FIELD_H_

#include <vector>

#include "value.h"

class INode;
class ICondNode;
class CRecord;

class CField {
	protected:
		std::string				_Name;	
		std::vector<IValue *>	_PossibleValues;
	public:
		CField();
		CField(std::string);
		~CField();
		virtual const std::vector<IValue *> &getPossibleValues() const;
		virtual void addPossibleValue(IValue *);
		virtual ICondNode *createNode(int, int, std::vector<CRecord *> &) = 0;
		const std::string &getName() const;
};

//////////////////////////////////////////////////////////////////

class CBoolField : public CField {
	public:
		CBoolField();
		CBoolField(std::string);
		virtual ICondNode *createNode(int, int, std::vector<CRecord *> &);
};


//////////////////////////////////////////////////////////////////

class CStringField : public CField {
	public:
		CStringField();
		CStringField(std::string, std::vector<std::string> &);
		virtual ICondNode *createNode(int, int, std::vector<CRecord *> &);
};


//////////////////////////////////////////////////////////////////

class CIntField : public CField {
	public:
		CIntField();
		CIntField(std::string, std::vector<int> &);
		virtual ICondNode *createNode(int, int, std::vector<CRecord *> &);
};

//////////////////////////////////////////////////////////////////

class CRealField : public CField {
	private:
		std::vector<double> _Ranges;	
		void computeRanges();
	public:
		CRealField();
		CRealField(std::string, std::vector<double> &);
		virtual ICondNode *createNode(int, int, std::vector<CRecord *> &);
};

#endif
