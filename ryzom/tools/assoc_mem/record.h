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

#include <vector>
#include "value.h"

#ifndef NL_RECORD_H_
#define NL_RECORD_H_

class CRecord {
	private:
		std::vector<IValue *> _Values;

	public:
		CRecord();
		CRecord(std::vector<IValue *> &);
		virtual ~CRecord();
		const std::vector<IValue *> &getValues();
		void addValue(IValue *);
		void addValue(std::string &);
		void addValue(bool);
		void addValue(int);

		int size();

		virtual const IValue *operator[](int);
};

#endif