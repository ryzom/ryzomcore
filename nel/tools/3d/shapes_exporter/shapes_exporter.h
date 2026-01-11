// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef SHAPES_EXPORTER_H
#define SHAPES_EXPORTER_H

#include <nel/misc/common.h>
#include <nel/misc/aabbox.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_driver.h>

struct Settings
{
	Settings();

	std::string input_path;
	std::string output_path;

	std::string preview_format;
	uint preview_width;
	uint preview_height;
	uint8 preview_quality;

	uint output_steps_z;
	uint output_steps_x;
	std::string output_format;
	uint output_width;
	uint output_height;
	uint8 output_antialiasing;
	uint8 output_quality;
	NLMISC::CRGBA output_background;

	NLMISC::CRGBA light_ambiant;
	NLMISC::CRGBA light_diffuse;
	NLMISC::CRGBA light_specular;
	NLMISC::CVector light_direction;
};

class ShapesExporter
{
public:
	ShapesExporter();
	virtual ~ShapesExporter();

	bool init();
	bool parseConfigFile(const std::string &filename);
	bool setupLight(const NLMISC::CVector &position, const NLMISC::CVector &direction);

	bool exportShape(const std::string &filename, const std::string &output_path);
	bool exportSkeleton(const std::string &skeleton, const std::vector<std::string> &parts, const std::string &output_path);
	void setCamera(NLMISC::CAABBox &bbox, NL3D::UTransform &entity, bool high_z=false);

	bool saveOneImage(const std::string &output_path);
	bool renderShape(NL3D::UTransform &entity, const std::string &output_path);
	bool renderPS(NL3D::UInstance &entity, const std::string &output_path, double &duration, NLMISC::CAABBox &bbox);
	bool createThumbnail(const std::string &filename, const std::string &path);

	static std::string findSkeleton(const std::string &shape);

	Settings settings;
	NL3D::UDriver* Driver;
	NL3D::UScene* Scene;
};

#endif
