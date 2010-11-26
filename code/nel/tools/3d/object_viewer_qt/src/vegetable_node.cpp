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
	Vegetable = NULL;
	VegetableName = NL_DefaultVegetName;
	Visible = true;
}

void CVegetableNode::initDefaultVegetable()
{
	Vegetable= new NL3D::CVegetable;
	// update vegetableName according to Vegetable
	updateVegetableName();

	// init Vegetable with some good default values.

	// General/Density.
	// Density.
	Vegetable->Density.Abs = NL_VEGETABLE_DENSITY_ABS_DEFAULT;
	Vegetable->Density.Rand = NL_VEGETABLE_DENSITY_RAND_DEFAULT;
	Vegetable->Density.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// disable MaxDensity
	Vegetable->MaxDensity = -1;
	// Leave ShapeName to ""
	// Default DistType is always 0.
	Vegetable->DistType = 0;


	// Apperance
	// BendPhase
	Vegetable->BendPhase.Abs = NL_VEGETABLE_BENDPHASE_ABS_DEFAULT;
	Vegetable->BendPhase.Rand = NL_VEGETABLE_BENDPHASE_RAND_DEFAULT;
	Vegetable->BendPhase.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// BendFactor
	Vegetable->BendFactor.Abs = NL_VEGETABLE_BENDFACTOR_ABS_DEFAULT;
	Vegetable->BendFactor.Rand = NL_VEGETABLE_BENDFACTOR_RAND_DEFAULT;
	Vegetable->BendFactor.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// Color
	Vegetable->Color.NoiseValue.Abs = NL_VEGETABLE_COLOR_ABS_DEFAULT;
	Vegetable->Color.NoiseValue.Rand = NL_VEGETABLE_COLOR_RAND_DEFAULT;
	Vegetable->Color.NoiseValue.Frequency = NL_VEGETABLE_FREQ_DEFAULT;

	// Scale
	// ScaleXY
	Vegetable->Sxy.Abs = NL_VEGETABLE_SCALE_ABS_DEFAULT;
	Vegetable->Sxy.Rand = NL_VEGETABLE_SCALE_RAND_DEFAULT;
	Vegetable->Sxy.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// ScaleZ.
	Vegetable->Sz.Abs = NL_VEGETABLE_SCALE_ABS_DEFAULT;
	Vegetable->Sz.Rand = NL_VEGETABLE_SCALE_RAND_DEFAULT;
	Vegetable->Sz.Frequency = NL_VEGETABLE_FREQ_DEFAULT;

	// Rotate
	// RotateX
	Vegetable->Rx.Abs = NL_VEGETABLE_ROTATEX_ABS_DEFAULT;
	Vegetable->Rx.Rand = NL_VEGETABLE_ROTATEX_RAND_DEFAULT;
	Vegetable->Rx.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// RotateY
	Vegetable->Ry.Abs = NL_VEGETABLE_ROTATEY_ABS_DEFAULT;
	Vegetable->Ry.Rand = NL_VEGETABLE_ROTATEY_RAND_DEFAULT;
	Vegetable->Ry.Frequency = NL_VEGETABLE_FREQ_DEFAULT;
	// RotateZ
	Vegetable->Rz.Abs = NL_VEGETABLE_ROTATEZ_ABS_DEFAULT;
	Vegetable->Rz.Rand = NL_VEGETABLE_ROTATEZ_RAND_DEFAULT;
	Vegetable->Rz.Frequency = NL_VEGETABLE_ROTATEZ_FREQ_DEFAULT;

}

void CVegetableNode::initVegetable(const NL3D::CVegetable &vegetable)
{
	Vegetable = new NL3D::CVegetable(vegetable);
	// update vegetableName according to Vegetable
	updateVegetableName();
}


void CVegetableNode::updateVegetableName()
{
	// Build the vegetable Name according to the ShapeName
	if(Vegetable->ShapeName == "")
	{
		VegetableName = NL_DefaultVegetName;
	}
	else
	{
		std::string::size_type pos = Vegetable->ShapeName.find(".veget");
		VegetableName= Vegetable->ShapeName.substr(0, pos);
		// And (to be clearer) append distance of creation.
		char	str[256];
		sprintf(str, " - %dm", (Vegetable->DistType + 1) * 10);
		VegetableName += str;
		// NB: if you add info with other parameters, you must use updateCurSelVegetableName() if they change
	}
}

void CVegetableNode::deleteVegetable()
{
	delete Vegetable;
	Vegetable = NULL;
	VegetableName = NL_DefaultVegetName;
}

} /* namespace NLQT */