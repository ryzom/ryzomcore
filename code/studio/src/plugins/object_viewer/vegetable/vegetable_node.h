/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef VEGETABLE_NODE_H
#define VEGETABLE_NODE_H

// STL includes
#include <string>

// NeL includes
#include <nel/3d/vegetable.h>

namespace NLQT
{

class CVegetableNode
{
public:
	/// Constructor
	CVegetableNode(void);

	// init the vegetable
	void initDefaultVegetable();

	void initVegetable(const NL3D::CVegetable &vegetable);

	// update VegetableName according to Vegetable
	void updateVegetableName();

	// delete the vegetable
	void deleteVegetable();

	// The vegetable
	NL3D::CVegetable *_vegetable;

	// The name of this vegetable.
	std::string	_vegetableName;

	// Visibility. Editor feature only
	bool _visible;

}; /* class CVegetableNode */

} /* namespace NLQT */

#endif // VEGETABLE_NODE_H
