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

#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/config_file.h>
#include <nel/3d/u_light.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_particle_system_instance.h>
#include <nel/3d/particle_system.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/ps_util.h>

#include "shapes_exporter.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

Settings::Settings()
{
	preview_width = 0;
	preview_height = 0;
	preview_quality = 0;

	output_steps_z = 0;
	output_steps_x = 0;
	output_width = 0;
	output_height = 0;
	output_antialiasing = 0;
	output_quality = 0;
	output_background = CRGBA::Black;

	light_ambiant = CRGBA::White;
	light_diffuse = CRGBA::White;
	light_specular = CRGBA::White;
	light_direction = CVector(0.f, 0.f, 0.f);
}

ShapesExporter::ShapesExporter():Driver(NULL), Scene(NULL)
{
}

ShapesExporter::~ShapesExporter()
{
	// delete the scene
	Driver->deleteScene(Scene);

	// release all textures and others elements
	Driver->release();

	// delete the driver
	delete Driver;
}

bool ShapesExporter::init()
{
	if (!settings.output_width) return false;

	// create OpenGL driver
	Driver = UDriver::createDriver();
	if (!Driver) return false;

	// create a window
	Driver->setDisplay(UDriver::CMode((uint16)(settings.output_width * settings.output_antialiasing),
		(uint16)(settings.output_height * settings.output_antialiasing), 32, true));

	// set the title
	Driver->setWindowTitle(ucstring("NeL images exporter"));

	// Create a scene
	Scene = Driver->createScene(true);
	if (!Scene) return false;

	return true;
}

bool ShapesExporter::parseConfigFile(const string &filename)
{
	CConfigFile cf;

	try
	{
		// load the config file
		cf.load(filename);
	}
	catch(const exception &e)
	{
		nlwarning("can't parse config file : %s", filename.c_str());
		nlwarning(e.what());
		return false;
	}

	// input path
	try
	{
		settings.input_path = CPath::standardizePath(cf.getVar("input_path").asString());
	}
	catch (const EUnknownVar &)
	{
	}

	// output path
	try
	{
		settings.output_path = CPath::standardizePath(cf.getVar("output_path").asString());
	}
	catch (const EUnknownVar &)
	{
	}

	// output format
	try
	{
		settings.output_format = cf.getVar("output_format").asString();
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_format = "jpg";
	}

	// add search pathes
	try
	{
		CConfigFile::CVar &search_pathes = cf.getVar("search_pathes");
		for (uint i=0; i < (uint)search_pathes.size(); ++i)
			CPath::addSearchPath(CPath::standardizePath(search_pathes.asString(i)));
	}
	catch(const EUnknownVar &)
	{
	}

	// add recusrive search pathes
	try
	{
		CConfigFile::CVar &recursive_search_pathes = cf.getVar("recursive_search_pathes");
		for (uint i=0; i< (uint)recursive_search_pathes.size(); ++i)
			CPath::addSearchPath(CPath::standardizePath(recursive_search_pathes.asString(i)), true, false);
	}
	catch(const EUnknownVar &)
	{
	}

	// add extension remapping
	try
	{
		CConfigFile::CVar &extensions_remapping = cf.getVar("extensions_remapping");

		if (extensions_remapping.size()%2 != 0)
		{
			nlwarning ("extensions_remapping must have a multiple of 2 entries (ex: extensions_remapping={\"dds\",\"tga\"};)");
		}
		else
		{
			for (uint i=0; i < (uint)extensions_remapping.size(); i+=2)
				CPath::remapExtension(extensions_remapping.asString(i), extensions_remapping.asString(i+1), true);
		}
	}
	catch (const EUnknownVar &)
	{
	}

	// preview format
	try
	{
		settings.preview_format = cf.getVar("preview_format").asString();
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.preview_format = "jpg";
	}

	// preview image width
	try
	{
		settings.preview_width = cf.getVar("preview_width").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.preview_width = 256;
	}

	// preview image height
	try
	{
		settings.preview_height = cf.getVar("preview_height").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.preview_height = 256;
	}

	// preview jpeg image quality
	try
	{
		settings.preview_quality = (uint8)cf.getVar("preview_quality").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.preview_quality = 90;
	}

	// output background color
	try
	{
		CConfigFile::CVar &var = cf.getVar("output_background");
		settings.output_background.R = (uint8)var.asInt(0);
		settings.output_background.G = (uint8)var.asInt(1);
		settings.output_background.B = (uint8)var.asInt(2);
	}
	catch (const EUnknownVar &)
	{
		settings.output_background = CRGBA::Black;
	}

	// light ambiant color
	try
	{
		CConfigFile::CVar &var = cf.getVar("light_ambiant");
		settings.light_ambiant.R = (uint8)var.asInt(0);
		settings.light_ambiant.G = (uint8)var.asInt(1);
		settings.light_ambiant.B = (uint8)var.asInt(2);
	}
	catch (const EUnknownVar &)
	{
		settings.light_ambiant = CRGBA::White;
	}

	// light diffuse color
	try
	{
		CConfigFile::CVar &var = cf.getVar("light_diffuse");
		settings.light_diffuse.R = (uint8)var.asInt(0);
		settings.light_diffuse.G = (uint8)var.asInt(1);
		settings.light_diffuse.B = (uint8)var.asInt(2);
	}
	catch (const EUnknownVar &)
	{
		settings.light_diffuse = CRGBA::White;
	}

	// light specular color
	try
	{
		CConfigFile::CVar &var = cf.getVar("light_specular");
		settings.light_specular.R = (uint8)var.asInt(0);
		settings.light_specular.G = (uint8)var.asInt(1);
		settings.light_specular.B = (uint8)var.asInt(2);
	}
	catch (const EUnknownVar &)
	{
		settings.light_specular = CRGBA::White;
	}

	// light direction
	try
	{
		CConfigFile::CVar &var = cf.getVar("light_direction");
		settings.light_direction = CVector(var.asFloat(0), var.asFloat(1), var.asFloat(2));
	}
	catch (const EUnknownVar &)
	{
		settings.light_direction = CVector(0.f, 1.f, 0.f);
	}

	// output steps z number
	try
	{
		settings.output_steps_z = cf.getVar("output_steps_z").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_steps_z = 10;
	}

	// output steps x number
	try
	{
		settings.output_steps_x = cf.getVar("output_steps_x").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_steps_x = 10;
	}

	// output image width
	try
	{
		settings.output_width = cf.getVar("output_width").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_width = 256;
	}

	// output image height
	try
	{
		settings.output_height = cf.getVar("output_height").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_height = 256;
	}

	// output image antialiasing
	try
	{
		settings.output_antialiasing = (uint8)cf.getVar("output_antialiasing").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_antialiasing = 2;
	}

	// output jpeg image quality
	try
	{
		settings.output_quality = (uint8)cf.getVar("output_quality").asInt();								
	}
	catch (const NLMISC::EUnknownVar &)
	{
		settings.output_quality = 90;
	}

	return true;
}

bool ShapesExporter::setupLight(const CVector &position, const CVector &direction)
{
	// create the light
	ULight *Light = ULight::createLight();
	if (!Light) return false;

	// set mode of the light
	Light->setMode(ULight::DirectionalLight);

	// set position of the light
//	Light->setupDirectional(settings.light_ambiant, settings.light_diffuse, settings.light_specular, settings.light_direction);
	Light->setupPointLight(settings.light_ambiant, settings.light_diffuse, settings.light_specular, position, direction + settings.light_direction);

	// set and enable the light
	Driver->setLight(0, *Light);
	Driver->enableLight(0);

	return true;
}

void ShapesExporter::setCamera(CAABBox &bbox, UTransform &entity, bool high_z)
{
	CVector pos(0.f, 0.f, 0.f);
	CQuat quat(0.f, 0.f, 0.f, 0.f);
	NL3D::UInstance inst;
	inst.cast(entity);
	if (!inst.empty())
	{
		inst.getDefaultPos(pos);
		inst.getDefaultRotQuat(quat);
/*
		if (quat.getAxis().isNull())
		{
			quat.set(0, 0, 0, 0);
			inst.setRotQuat(quat);
		}
*/
//		quat.set(1.f, 1.f, 0.f, 0.f);

//		inst.setRotQuat(quat);
//		inst.getRotQuat(quat);

		// check for presence of all textures from each sets
		bool allGood = true;

		for(uint s = 0; s < 5; ++s)
		{
			inst.selectTextureSet(s);

			uint numMat = inst.getNumMaterials();

			// by default, all textures are present
			allGood = true;

			for(uint i = 0; i < numMat; ++i)
			{
				UInstanceMaterial mat = inst.getMaterial(i);

				for(sint j = 0; j <= mat.getLastTextureStage(); ++j)
				{
					// if a texture is missing
					if (mat.isTextureFile(j) && mat.getTextureFileName(j) == "CTextureMultiFile:Dummy")
						allGood = false;
				}
			}

			// if all textures have been found for this set, skip other sets
			if (allGood)
				break;
		}
	}

	// fix scale (some shapes have a different value)
	entity.setScale(1.f, 1.f, 1.f);

	UCamera Camera = Scene->getCam();
	CVector max_radius = bbox.getHalfSize();

	CVector center = bbox.getCenter();
	entity.setPivot(center);
	center += pos;

	float fov = float(20.0*Pi/180.0);
	Camera.setPerspective (fov, 1.0f, 0.1f, 1000.0f);
	float radius = max(max(max_radius.x, max_radius.y), max_radius.z);
	if (radius == 0.f) radius = 1.f;
	float left, right, bottom, top, znear, zfar;
	Camera.getFrustum(left, right, bottom, top, znear, zfar);
	float dist = radius / (tan(fov/2));
	CVector eye(center);
/*	if (axis == CVector::I)
		eye.y -= dist+radius;
	else if (axis == CVector::J)
		eye.x += dist+radius;
*/
//	quat.normalize();

	CVector ax(quat.getAxis());

//	float angle = quat.getAngle();
/*
	if (ax.isNull())
	{
		if (int(angle*100.f) == int(NLMISC::Pi * 200.f))
		{
			ax = CVector::J;
		}
	}
	else 
*/
	if (ax.isNull() || ax == CVector::I)
	{
		ax = CVector::I;
	}
	else if (ax == -CVector::K)
	{
		ax = -CVector::J;
	}
/*	else if (ax.x < -0.9f && ax.y == 0.f && ax.z == 0.f)
	{
		ax = -CVector::J ;
	}
*/
//	ax.normalize();

	eye -= ax * (dist+radius);
	if (high_z)
		eye.z += max_radius.z/1.0f;
	Camera.lookAt(eye, center);

	setupLight(eye, center - eye);
}

bool ShapesExporter::exportShape(const string &filename, const string &output_path)
{
	// get scene camera
	if (Scene->getCam().empty())
	{
		nlwarning("can't get camera from scene");
		return false;
	}

	//CParticleSystem::forceDisplayBBox(true);

	// add an entity to the scene
	UInstance Entity = Scene->createInstance(filename);

	// if we can't create entity, skip it
	if (Entity.empty())
	{
		nlwarning("can't create instance from %s", filename.c_str());
		return false;
	}

	// get AABox of Entity
	CAABBox bbox;
	Entity.getShapeAABBox(bbox);
	setCamera(bbox , Entity);

	Scene->animate(1.0);
	Scene->render();

	if(CFile::getExtension(filename) == "ps")
	{
		UParticleSystemInstance *psi = static_cast<UParticleSystemInstance*>(&Entity);
		if(psi)
		{
			psi->getSystemBBox(bbox);
			setCamera(bbox, Entity);
		}

		// first pass to detect bbox & duration
		CAABBox bbox2;
		double duration = 0.0;
		renderPS(Entity, output_path, duration, bbox2);
		Scene->deleteInstance(Entity);
		// second pass to actually take screenshots
		Entity = Scene->createInstance(filename);
		setCamera(bbox, Entity, true);
		renderPS(Entity, output_path, duration, bbox2);
	}
	else
	{
		renderShape(Entity, output_path);
	}

	// delete entity
	Scene->deleteInstance(Entity);

	return true;
}

bool ShapesExporter::exportSkeleton(const string &skeleton, const vector<string> &parts, const string &output_path)
{
	// get scene camera
	UCamera Camera = Scene->getCam();
	if (Camera.empty())
	{
		nlwarning("can't get camera from scene");
		return false;
	}

	// add a skeleton to the scene
	USkeleton Skeleton = Scene->createSkeleton(skeleton);

	// if we can't create entity, skip it
	if (Skeleton.empty())
	{
		nlwarning("can't create skeleton from %s", skeleton.c_str());
		return false;
	}

	vector<UInstance> Entities(parts.size());

	for(size_t i = 0; i < parts.size(); ++i)
	{
		Entities[i] = Scene->createInstance(parts[i]);

		// if we can't create entity, skip it
		if (Entities[i].empty())
		{
			nlwarning("can't create instance from %s", parts[i].c_str());
			return false;
		}

		if (!Skeleton.bindSkin(Entities[i]))
		{
			nlwarning("can't bind %s to skeleton", parts[i].c_str());
			return false;
		}
	}

	// get AABox of Entity
	CAABBox bbox;
	Skeleton.computeCurrentBBox(bbox, NULL);
	setCamera(bbox, Skeleton);

	renderShape(Skeleton, output_path);

	// delete entities
	for(size_t i = 0; i < Entities.size(); ++i)
	{
		Skeleton.detachSkeletonSon(Entities[i]);
		Scene->deleteInstance(Entities[i]);
	}

	// delete skeleton
	Scene->deleteSkeleton(Skeleton);

	return true;
}

/*
bool ShapesExporter::exportAnimation(const std::string &animation, const std::string &skeleton, const std::vector<std::string> &parts, const std::string &output_path)
{
	UPlayListManager *PlayListManager = Scene->createPlayListManager();
	UAnimationSet *AnimSet = Driver->createAnimationSet();

//	uint anim_id = AnimSet->addAnimation("anim.anim", "anim_name", false);
//	uint weight_id = AnimSet->addSkeletonWeight("file.wgt", "skel_name"):

//	UAnimation *anim = AnimSet->getAnimation(anim_id);
//	anim->getEndTime();

//	UPlayList *playlist = playlist_manager->createPlayList(AnimSet);
//	playlist->registerTransform(Skeleton);

//	playlist->setAnimation(0, anim_id);
//	playlist->setTimeOrigin(newSlot, time);
//	playlist->setWeightSmoothness(newSlot, 1.0f);

	// get scene camera
	UCamera Camera = Scene->getCam();
	if (Camera.empty())
	{
		nlwarning("can't get camera from scene");
		return false;
	}

	// add a skeleton to the scene
	USkeleton Skeleton = Scene->createSkeleton(skeleton);

	// if we can't create entity, skip it
	if (Skeleton.empty())
	{
		nlwarning("can't create skeleton from %s", skeleton.c_str());
		return false;
	}

	std::vector<UInstance> Entities(parts.size());

	for(size_t i = 0; i < parts.size(); ++i)
	{
		Entities[i] = Scene->createInstance(parts[i]);

		// if we can't create entity, skip it
		if (Entities[i].empty())
		{
			nlwarning("can't create instance from %s", parts[i].c_str());
			return false;
		}

		if (!Skeleton.bindSkin(Entities[i]))
		{
			nlwarning("can't bind %s to skeleton", parts[i].c_str());
			return false;
		}
	}

	// get AABox of Entity
	CAABBox bbox;
	Skeleton.computeCurrentBBox(bbox, NULL);

	setCamera();

	// camera will look at skeleton
	Camera.lookAt(CVector(center.x + dist - radius, center.y, center.z), center);

	renderAllImages(Skeleton, CVector::J, output_path);

	// delete entities
	for(size_t i = 0; i < Entities.size(); ++i)
	{
		Skeleton.detachSkeletonSon(Entities[i]);
		Scene->deleteInstance(Entities[i]);
	}

	// delete skeleton
	Scene->deleteSkeleton(Skeleton);

	Scene->deletePlayListManager(PlayListManager);
	Driver->deleteAnimationSet(AnimSet);

//	m_playlist->emptyPlayList();
//	m_playlist->resetAllChannels();
//	m_playlistman->deletePlayList(m_playlist);

	return true;
}
*/

bool ShapesExporter::saveOneImage(const string &filename)
{
	CBitmap btm;
	Driver->getBuffer(btm);

	// resamble bitmap only if antialiasing is enabled
	if (settings.output_antialiasing > 1)
		btm.resample(settings.output_width, settings.output_height);

	COFile fs;

	if (fs.open(filename))
	{
		if (settings.output_format == "png")
		{
			if (!btm.writePNG(fs, 24))
			{
				nlwarning("can't save image to PNG");
				return false;
			}
		}
		else if (settings.output_format == "jpg")
		{
			if (!btm.writeJPG(fs, settings.output_quality))
			{
				nlwarning("can't save image to JPG");
				return false;
			}
		}
		else if (settings.output_format == "tga")
		{
			if (!btm.writeTGA(fs, 24))
			{
				nlwarning("can't save image to TGA");
				return false;
			}
		}
	}
	else
	{
		nlwarning("can't create %s", filename.c_str());
		return false;
	}

	return true;
}

bool ShapesExporter::renderShape(UTransform &entity, const string &output_path)
{
	CQuat quat(0.f, 0.f, 0.f, 0.f);

	CVector axis1 = CVector::J, axis2 = CVector::K;
	int orientation1 = -1, orientation2 = 1;

	NL3D::UInstance inst;
	inst.cast(entity);
	if (!inst.empty())
	{
//		inst.getDefaultRotQuat(quat);
		inst.getRotQuat(quat);
/*		if (!quat.getAxis().isNull())
		{
			CVector a = quat.getAxis();
			if (a.z != 0 && a.x == 0.f)
			{
				axis1 = CVector::J;
				orientation1 = -1;
			}
			if (a.y != 0.f)
			{
				axis2 = CVector::J;
			}
		}
*/
	}

	// main loop
	for (uint step_z = 0; step_z < settings.output_steps_z; ++step_z)
	{
		CQuat z(axis1, orientation1 * (float)step_z * ((float)NLMISC::Pi*2.f / (float)settings.output_steps_z));

		for (uint step_x = 0; step_x < settings.output_steps_x; ++step_x)
		{
			CQuat x(axis2, orientation2 * (float)step_x * ((float)NLMISC::Pi*2.f / (float)settings.output_steps_x));

			entity.setRotQuat(quat * z * x);

			string filename = CPath::standardizePath(output_path) + toString("%03d_%03d.%s", step_z, step_x, settings.output_format.c_str());

			// the background is black
			Driver->clearBuffers(settings.output_background);

			// render the scene
			Scene->render();

			if(!saveOneImage(filename))
				return false;
		}
	}

	return true;
}

bool ShapesExporter::renderPS(UInstance &entity, const string &output_path, double &duration, CAABBox &bbox)
{
	TGlobalAnimationTime  time = 0.0, startTime = 0.0f;
	uint step = 0;

	static uint NbFrame = 100;

	double deltaTime = 0.05f;
	if (duration > 0.0f)
		deltaTime = duration / NbFrame;

	// main loop
	while(true)
	{
		uint nbparticle = 0;

		time += deltaTime;
		Scene->animate(time);

		UParticleSystemInstance *psi = static_cast<UParticleSystemInstance*>(&entity);
		CTransformShape	*e = entity.getObjectPtr();
		CParticleSystemModel *psm = dynamic_cast<CParticleSystemModel*>(e);
		if(psm->getPS())
		{
			nbparticle = psm->getPS()->getCurrNumParticles();
			if(duration == 0.0f && time <= 10.0)
			{
				// after 10s we stop computing the bbox
				CAABBox bbox2;
				psm->getPS()->forceComputeBBox(bbox2);
				bbox = CAABBox::computeAABBoxUnion(bbox, bbox2);
				//bbox = bbox2;
			}
		}
		if(psi)
		{
			//psi->getSystemBBox(bbox);
			setCamera(bbox, entity, true);
		}

		// the background is black
		Driver->clearBuffers(settings.output_background);

		// render the scene
		Scene->render();

		if(nbparticle > 0)
		{
			if(startTime == 0.0f)
				startTime = time;
		}
		if(duration > 0.0f)
		{
			string filename = CPath::standardizePath(output_path) + toString("000_%03d.%s", step, settings.output_format.c_str());
			step++;
			if(!saveOneImage(filename))
				return false;
		}
		if(time >= 30.0 || (nbparticle == 0 && startTime > 0.0f) || (duration != 0.0f && time > duration))
			break;
	}

	if(duration > 0.0f)
	{
		if(step > 0)
		{
			FILE *fp = fopen(string(CPath::standardizePath(output_path)+"nb_steps.txt").c_str(), "w");
			if(fp) { fprintf(fp, "%d", step); fclose(fp); }
		}
		nlinfo("PS duration %f after %f with nothing with %d steps, dt %f", duration, startTime, step, deltaTime);
	}

	if(time >= 30.0)
		duration = 10.0f;	// for infinite duration animation, only take first 10s
	else
		duration = time;

	return true;
}

bool ShapesExporter::createThumbnail(const string &filename, const string &path)
{
	string output_path = path;

	// Select which frame we'll use to do the thumnail
	uint selectedFrame = 0;
	if(CFile::getExtension(filename) == "ps")
	{
		FILE *fp = fopen(string(CPath::standardizePath(output_path)+"nb_steps.txt").c_str(), "r");
		if(fp)
		{
			char str[100];
			if (!fgets(str, 100, fp))
				strcpy(str, "0");
			fclose(fp);
			NLMISC::fromString(std::string(str), selectedFrame);
			selectedFrame /= 2;
		}
	}

	CIFile in;

	// create a thumbail from first image
	if (in.open(output_path + toString("/000_%03d.", selectedFrame) + settings.output_format))
	{
		CBitmap bitmap;
		bitmap.load(in);
		bitmap.resample(settings.preview_width, settings.preview_height);

		output_path += "/thumb";

		// create directory for thumbnail
		if (!CFile::isExists(output_path) && !CFile::createDirectory(output_path))
		{
			nlwarning("can't create %s", output_path.c_str());
			return false;
		}

		COFile out;

		if (out.open(output_path + "/" + CFile::getFilenameWithoutExtension(filename) + "." + settings.preview_format))
		{
			if (settings.preview_format == "png")
			{
				if (!bitmap.writePNG(out, 24))
				{
					nlwarning("can't save image to PNG");
					return false;
				}
			}
			else if (settings.preview_format == "jpg")
			{
				if (!bitmap.writeJPG(out, settings.preview_quality))
				{
					nlwarning("can't save image to JPG");
					return false;
				}
			}
			else if (settings.preview_format == "tga")
			{
				if (!bitmap.writeTGA(out, 24))
				{
					nlwarning("can't save image to TGA");
					return false;
				}
			}
		}
		else
		{
			nlwarning("can't create %s", filename.c_str());
			return false;
		}
	}

	return true;
}

std::string ShapesExporter::findSkeleton(const std::string &shape)
{
	std::string baseFilename = CFile::getFilenameWithoutExtension(shape);

	// work in 60% of cases
	std::string skeleton = CPath::lookup(baseFilename + ".skel", false, false, false);

	if (!skeleton.empty())
		return skeleton;

	// remove last part
	std::string::size_type pos = baseFilename.rfind("_");

	if (pos != std::string::npos)
	{
		skeleton = CPath::lookup(baseFilename.substr(0, pos) + ".skel", false, false, false);

		if (!skeleton.empty())
			return skeleton;

		pos = baseFilename.find("_");

		std::vector<std::string> filenames;

		CPath::getFileListByName("skel", baseFilename.substr(pos), filenames);

		if (filenames.size() == 1)
		{
			skeleton = filenames[0];
			return skeleton;
		}

	}

	int gender = 0;

	if (baseFilename.find("_hom_") != std::string::npos)
	{
		gender = 1;
	}
	else if (baseFilename.find("_hof_") != std::string::npos)
	{
		gender = 2;
	}

	// bipeds
	if (gender > 0)
	{
		// karavan
		if (baseFilename.find("ca_") == 0)
			return gender == 1 ? "ca_hom_armor01.skel":"ca_hof_armor01.skel";

		return gender == 1 ? "fy_hom_skel.skel":"fy_hof_skel.skel";
	}

	nlwarning("can't find skeleton for %s", shape.c_str());
	// goo mobs
//	CPath::getFileListByName("max", "_hof_", filenames);

	return "";
}
