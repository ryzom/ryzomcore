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

#ifndef NL_BRAIN_H_
#define NL_BRAIN_H_

#include "mood.h"
#include "attribute.h"
#include "tree.h"
#include "field.h"
#include <vector>

class CBrain : public CMood
{
	private:

		enum eType {

		};

		CMood					_Personality;
		CMood					_RealTime;
		std::vector<CTree *>	_Trees;
		std::vector<CField *>	_Fields;
		std::vector<CRecord *>	_Records;
		int						_UpdateEvery;
		int						_LastUpdate;
	public:
		CBrain(float,float,float,float,float);
		CBrain(CMood &);
		CBrain(const CBrain &);

		std::vector<std::string> getOutputs();

		void addField(CField *);
		void addTree(CTree *);

		void setUpdateEvery(int);

		virtual float getFear();
		virtual float getAgressivity();
		virtual float getEmpathy();
		virtual float getHappiness();
		virtual float getHunger();


		void addRecord(CRecord *);
		void setInput(CRecord *);
		void build();

		std::string getDebugString();

		void forget();
};

#endif
