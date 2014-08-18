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

#include "stdpch.h"
#include "vegetable_node.h"

#include "modules.h"

namespace NLQT
{

static const char *NL_DefaultVegetName= "<default>";

CVegetableNode::CVegetableNode(void)
{
	_vegetable = NULL;
	_vegetableName = NL_DefaultVegetName;
	_visible = true;
}

void CVegetableNode::initDefaultVegetable()
{
	_vegetable = new NL3D::CVegetable;
	// update vegetableName according to Vegetable
	updateVegetableName();

	// init Vegetable with some good default values.

	// General/Density.
	// Density.
	_vegetable->Density.Abs = NL_VEGETABLE_DENSITY_ABS_DEFAULT;
	_vegetable->Density.Rand = NL_VEGETABLE_DENSITY_RAND_DEFAULT;
	_vegetable->Density.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// disable MaxDensity
	_vegetable->MaxDensity = -1;
	// Leave ShapeName to ""
	// Default DistType is always 0.
	_vegetable->DistType = 0;


	// Apperance
	// BendPhase
	_vegetable->BendPhase.Abs = NL_VEGETABLE_BENDPHASE_ABS_DEFAULT;
	_vegetable->BendPhase.Rand = NL_VEGETABLE_BENDPHASE_RAND_DEFAULT;
	_vegetable->BendPhase.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// BendFactor
	_vegetable->BendFactor.Abs = NL_VEGETABLE_BENDFACTOR_ABS_DEFAULT;
	_vegetable->BendFactor.Rand = NL_VEGETABLE_BENDFACTOR_RAND_DEFAULT;
	_vegetable->BendFactor.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// Color
	_vegetable->Color.NoiseValue.Abs = NL_VEGETABLE_COLOR_ABS_DEFAULT;
	_vegetable->Color.NoiseValue.Rand = NL_VEGETABLE_COLOR_RAND_DEFAULT;
	_vegetable->Color.NoiseValue.Frequency = NL_VEGETABLE_FREQ_DEFAULT;

	// Scale
	// ScaleXY
	_vegetable->Sxy.Abs = NL_VEGETABLE_SCALE_ABS_DEFAULT;
	_vegetable->Sxy.Rand = NL_VEGETABLE_SCALE_RAND_DEFAULT;
	_vegetable->Sxy.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// ScaleZ.
	_vegetable->Sz.Abs = NL_VEGETABLE_SCALE_ABS_DEFAULT;
	_vegetable->Sz.Rand = NL_VEGETABLE_SCALE_RAND_DEFAULT;
	_vegetable->Sz.Frequency = NL_VEGETABLE_FREQ_DEFAULT;

	// Rotate
	// RotateX
	_vegetable->Rx.Abs = NL_VEGETABLE_ROTATEX_ABS_DEFAULT;
	_vegetable->Rx.Rand = NL_VEGETABLE_ROTATEX_RAND_DEFAULT;
	_vegetable->Rx.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// RotateY
	_vegetable->Ry.Abs = NL_VEGETABLE_ROTATEY_ABS_DEFAULT;
	_vegetable->Ry.Rand = NL_VEGETABLE_ROTATEY_RAND_DEFAULT;
	_vegetable->Ry.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// RotateZ
	_vegetable->Rz.Abs = NL_VEGETABLE_ROTATEZ_ABS_DEFAULT;
	_vegetable->Rz.Rand = NL_VEGETABLE_ROTATEZ_RAND_DEFAULT;
	_vegetable->Rz.Frequency = NL_VEGETABLE_ROTATEZ_FREQ_DEFAULT;

}

void CVegetableNode::initVegetable(const NL3D::CVegetable &vegetable)
{
	_vegetable = new NL3D::CVegetable(vegetable);
	// update vegetableName according to Vegetable
	updateVegetableName();
}


void CVegetableNode::updateVegetableName()
{
	// Build the vegetable Name according to the ShapeName
	if(_vegetable->ShapeName == "")
	{
		_vegetableName = NL_DefaultVegetName;
	}
	else
	{
		std::string::size_type pos = _vegetable->ShapeName.find(".veget");
		_vegetableName = _vegetable->ShapeName.substr(0, pos);
		// And (to be clearer) append distance of creation.
		char	str[256];
		sprintf(str, " - %dm", (_vegetable->DistType + 1) * 10);
		_vegetableName += str;
		// NB: if you add info with other parameters, you must use updateCurSelVegetableName() if they change
	}
}

void CVegetableNode::deleteVegetable()
{
	delete _vegetable;
	_vegetable = NULL;
	_vegetableName = NL_DefaultVegetName;
}

} /* namespace NLQT */