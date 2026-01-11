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

#ifndef NL_TREE_H_
#define NL_TREE_H_

#include <vector>
#include <map>
#include "record.h"
#include "node.h"
#include "field.h"


class CTree
{
	private:
		//std::vector<std::string>	_Fields;
		int							_Key;
		INode						*_RootNode;

	public:

		struct greater : public std::binary_function<std::pair<double,int> , std::pair<double,int> , bool> {
			bool operator()(std::pair<double,int> x, std::pair<double,int> y) const
			{
				return x.first > y.first;
			}
		};

	public:
		CTree();
		~CTree();
		void setKey(int);
		int getKey();

		bool getOutput(CRecord *);

		int getNbRecords(std::vector<CRecord *> &,int,IValue *);

		std::vector<CRecord *> getRecords(std::vector<CRecord *> &, int , bool);

		void splitRecords(std::vector<CRecord *> &, int, int &, int &);
		void splitRecords(std::vector<CRecord *> &, int, std::vector<CField *> &, std::vector< std::vector<CRecord *> > &);

		void splitRecords(std::vector<CRecord *> &, int, IValue *, int &, int &);
		void splitRecords(std::vector<CRecord *> &, int, IValue *, bool, int &, int &);

		float findNumKeyValue(std::vector<CRecord *> &, int);

		double log2(double) const;

		double entropy(double, double) const;
		double entropy(std::vector<double> &) const;
		double gain(std::vector<CRecord *> &, int, CField *);

		std::vector<std::pair<double,int> > getSortedFields(std::vector<int> &, std::vector<CRecord *> &, std::vector<CField *> &);
		int getBestAttrib( std::vector<int> &, std::vector<CRecord *> &,  std::vector<CField *> &);
		void rebuild(std::vector<CRecord *> &, std::vector<CField *> &);

		std::string getDebugString(std::vector<CRecord *> &, std::vector<CField *> &);

		// returns the root node of a decision tree built with the ID3 algorithm
		INode *ID3(std::vector<int> &, std::vector<CRecord *> &, std::vector<CField *> &);
};

#endif
