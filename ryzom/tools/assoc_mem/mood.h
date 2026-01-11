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

#ifndef NL_MOOD_H_
#define NL_MOOD_H_


class CMood {
	private:
		float _Fear;
		float _Agressivity;
		float _Empathy;
		float _Happiness;
		float _Hunger;
	public:
		CMood();
		CMood(float,float,float,float,float);
		CMood(const CMood &);
		~CMood();

		CMood operator=(const CMood &);

		virtual float getFear();
		virtual float getAgressivity();
		virtual float getEmpathy();
		virtual float getHappiness();
		virtual float getHunger();

		void setFear(float);
		void setAgressivity(float);
		void setEmpathy(float);
		void setHappiness(float);
		void setHunger(float);
};

#endif