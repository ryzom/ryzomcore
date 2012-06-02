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




#ifndef CL_SKY_MATERIAL_SETUP
#define CL_SKY_MATERIAL_SETUP



namespace NL3D
{
	class UInstance;
}

// setup of material for night or day
struct CSkyMaterialSetup
{
	struct CTexInfo
	{
		uint		 MatNum;
		std::string  TexName;
	};
	std::vector<CTexInfo> Setup;

	/** Build a setup of texture for the given stage.
	  * \param instance The instance from which setup must be built
	  * \param stage stage at which the setup must be taken. Must be 0 or 1
	  */
	void buildFromInstance(NL3D::UInstance instance, uint stage);

	/** Apply a setup of texture at the given stage
	  * You can also ask to skip first material, as it is used for the skydome
	  */
	void applyToInstance(NL3D::UInstance instance, uint stage, bool skipFirstMaterial = false);
};


#endif

