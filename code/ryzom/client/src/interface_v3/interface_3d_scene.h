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



#ifndef RZ_INTERFACE_SCENE_3D_H
#define RZ_INTERFACE_SCENE_3D_H

#include "nel/gui/interface_group.h"
#include "nel/3d/u_point_light.h"
#include "nel/3d/u_particle_system_instance.h"

class CCharacter3D;

class CInterface3DCharacter;
class CInterface3DShape;
class CInterface3DIG;
class CInterface3DCamera;
class CInterface3DLight;
class CInterface3DFX;

namespace NL3D
{
	class UParticleSystemInstance;
	class UAnimationSet;
}

/**
 * class managing all 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DScene : public CInterfaceGroup
{

public:

	CInterface3DScene(const TCtorParam &param);
	virtual ~CInterface3DScene();

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	virtual void checkCoords();

	virtual void updateCoords ();

	virtual void draw ();

	virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

	virtual CInterfaceElement* getElement (const std::string &id);

	NL3D::UScene *getScene() { return _Scene; }

	std::string getCurrentCamera() const;
	void setCurrentCamera(const std::string &sCameraName);

	std::string getCurrentClusterSystem () const;
	void setCurrentClusterSystem(const std::string &sCameraName);

	float getRotFactor() const { return _RotZFactor; }
	void setRotFactor(float f) { _RotZFactor = f; }

	void setFlareContext(uint context) { _Scene->setFlareContext(context); }

	float getDistLimitMin() const { return _DistLimitMin;}
	void  setDistLimitMin(float limitMin) { _DistLimitMin = limitMin;}

	float getDistLimitMax() const { return _DistLimitMax;}
	void  setDistLimitMax(float limitMax) { _DistLimitMax = limitMax;}

	REFLECT_EXPORT_START(CInterface3DScene, CInterfaceGroup)
		REFLECT_STRING ("curcam", getCurrentCamera, setCurrentCamera);
		REFLECT_STRING ("curcs", getCurrentClusterSystem, setCurrentClusterSystem);
		REFLECT_FLOAT ("rotzfactor", getRotFactor, setRotFactor);
		REFLECT_FLOAT ("distlimitmin", getDistLimitMin, setDistLimitMin);
		REFLECT_FLOAT ("distlimitmax", getDistLimitMax, setDistLimitMax);
	REFLECT_EXPORT_END

	void remove(NL3D::UInstanceGroup *pIG);

	uint					getCharacter3DCount() const { return (uint)_Characters.size(); }
	CInterface3DCharacter	*getCharacter3D(uint index);

	CInterface3DCamera		*getCamera(uint index);

protected:

	// If this value is not NULL the current scene is just a view onto another one
	CInterface3DScene		*_Ref3DScene;

	// Parsed properties
	NL3D::UScene			*_Scene;		// The scene containing all the 3D elements
	uint					_CurrentCamera;
	uint					_CurrentCS;		// Current Cluster System
	// The AutoAnimSet (if some auto_anim)
	NL3D::UAnimationSet		*_AutoAnimSet;

	std::vector<CInterface3DCharacter*>	_Characters;
	std::vector<CInterface3DShape*>		_Shapes;
	std::vector<CInterface3DIG*>		_IGs;
	std::vector<CInterface3DCamera*>	_Cameras;
	std::vector<CInterface3DLight*>		_Lights;
	std::vector<CInterface3DFX*>		_FXs;

	// Mouse event handling
	bool _UserInteraction;
	float _RotZLimitMin, _RotZLimitMax;
	float _RotZFactor;
	float _RotYLimitMin, _RotYLimitMax;
	float _RotYFactor;

	float _DistLimitMin, _DistLimitMax;
	float _DistFactor;

	bool _MouseLDown, _MouseRDown;
	sint32 _MouseLDownX, _MouseRDownX;
	sint32 _MouseLDownY, _MouseRDownY;

	void mouseLMove (sint32 dx, sint32 dy);
	void mouseRMove (sint32 dx, sint32 dy);

};

/**
 * class managing character 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DCharacter : public CInterfaceElement
{
public:
	CInterface3DCharacter();
	virtual ~CInterface3DCharacter();

	virtual bool parse (xmlNodePtr cur, CInterface3DScene *parentGroup);

	virtual void checkCoords();

	void setClusterSystem (NL3D::UInstanceGroup *pIG);

	float getRotX () const;
	float getRotY () const;
	float getRotZ () const;

	void setRotX (float f);
	void setRotY (float f);
	void setRotZ (float f);

	float getPosX () const;
	float getPosY () const;
	float getPosZ () const;

	void setPosX (float f);
	void setPosY (float f);
	void setPosZ (float f);

	float getHeadX () const;
	float getHeadY () const;
	float getHeadZ () const;

	void setHeadX (float /* f */) {}
	void setHeadY (float /* f */) {}
	void setHeadZ (float /* f */) {}

	sint32 getAnim () const {return 0;}
	void setAnim (sint32 anim);

	CCharacter3D * getCharacter3D() { return _Char3D; }

	void			setPeople(const std::string & people);
	std::string		getPeople() const;

	void			setSex(bool male);
	bool			getSex() const;

	void setupCharacter3D(sint32 slot);
	int luaSetupCharacter3D(CLuaState &ls);

	// active/inactive LOD of skeleton
	int luaEnableLOD(CLuaState &ls);


	REFLECT_EXPORT_START(CInterface3DCharacter, CInterfaceElement)
		REFLECT_LUA_METHOD("setupCharacter3D", luaSetupCharacter3D);
		REFLECT_LUA_METHOD("enableLOD", luaEnableLOD);
		REFLECT_FLOAT ("headx", getHeadX, setHeadX);
		REFLECT_FLOAT ("heady", getHeadY, setHeadY);
		REFLECT_FLOAT ("headz", getHeadZ, setHeadZ);
		REFLECT_FLOAT ("posx", getPosX, setPosX);
		REFLECT_FLOAT ("posy", getPosY, setPosY);
		REFLECT_FLOAT ("posz", getPosZ, setPosZ);
		REFLECT_FLOAT ("rotx", getRotX, setRotX);
		REFLECT_FLOAT ("roty", getRotY, setRotY);
		REFLECT_FLOAT ("rotz", getRotZ, setRotZ);
		REFLECT_SINT32 ("anim", getAnim, setAnim);
		REFLECT_STRING ("people", getPeople, setPeople);
		REFLECT_BOOL   ("sex", getSex, setSex);
	REFLECT_EXPORT_END

protected:
	CCharacter3D *_Char3D;
	std::string _DBLink;
};

/**
 * class managing shape instance 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DShape : public CInterfaceElement
{
public:
	CInterface3DShape()
	{
		_Instance = NULL;
		_Pos = NLMISC::CVector(0,0,0);
		_Rot = NLMISC::CVector(0,0,0);
	}

	virtual ~CInterface3DShape();

	virtual bool parse (xmlNodePtr cur, CInterface3DScene *parentGroup);

	NL3D::UInstance getShape() { return _Instance; }

	float getPosX () const;
	float getPosY () const;
	float getPosZ () const;

	void setPosX (float f);
	void setPosY (float f);
	void setPosZ (float f);

	float getRotX () const;
	float getRotY () const;
	float getRotZ () const;

	void setRotX (float f);
	void setRotY (float f);
	void setRotZ (float f);

	std::string getName() const;
	void        setName (const std::string &ht);

	REFLECT_EXPORT_START(CInterface3DShape, CInterfaceElement)
		REFLECT_FLOAT ("posx", getPosX, setPosX);
		REFLECT_FLOAT ("posy", getPosY, setPosY);
		REFLECT_FLOAT ("posz", getPosZ, setPosZ);
		REFLECT_FLOAT ("rotx", getRotX, setRotX);
		REFLECT_FLOAT ("roty", getRotY, setRotY);
		REFLECT_FLOAT ("rotz", getRotZ, setRotZ);
		REFLECT_STRING ("name", getName, setName);
	REFLECT_EXPORT_END

protected:

	NL3D::UInstance _Instance;
	NLMISC::CVector _Pos;
	NLMISC::CVector _Rot;
	std::string _Name;
};

/**
 * class managing instance group 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DIG : public CInterfaceElement
{
public:
	CInterface3DIG()
	{
		_IG = NULL;
		_Pos = NLMISC::CVector(0,0,0);
		_Rot = NLMISC::CVector(0,0,0);
	}

	virtual ~CInterface3DIG();

	virtual bool parse (xmlNodePtr cur, CInterface3DScene *parentGroup);

	NL3D::UInstanceGroup *getIG() { return _IG; }

	float getPosX () const;
	float getPosY () const;
	float getPosZ () const;

	void setPosX (float f);
	void setPosY (float f);
	void setPosZ (float f);

	float getRotX () const;
	float getRotY () const;
	float getRotZ () const;

	void setRotX (float f);
	void setRotY (float f);
	void setRotZ (float f);

	std::string getName() const;
	void        setName (const std::string &ht);

	REFLECT_EXPORT_START(CInterface3DIG, CInterfaceElement)
		REFLECT_FLOAT ("posx", getPosX, setPosX);
		REFLECT_FLOAT ("posy", getPosY, setPosY);
		REFLECT_FLOAT ("posz", getPosZ, setPosZ);
		REFLECT_FLOAT ("rotx", getRotX, setRotX);
		REFLECT_FLOAT ("roty", getRotY, setRotY);
		REFLECT_FLOAT ("rotz", getRotZ, setRotZ);
		REFLECT_STRING ("name", getName, setName);
	REFLECT_EXPORT_END

protected:

	NL3D::UInstanceGroup *_IG;
	NLMISC::CVector _Pos;
	NLMISC::CVector _Rot;
	std::string _Name;
};

/**
 * class managing camera 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DCamera : public CInterfaceElement
{

public:

	CInterface3DCamera()
	{
		_Rot = NLMISC::CVector(0,0,0);
		_Pos = NLMISC::CVector(0,0,0);
		_Target = NLMISC::CVector(0,0,1);
		_FOV = 36.0f;
		_Roll = 0;
		_Dist = 0;
	}

	virtual bool parse (xmlNodePtr cur, CInterface3DScene *parentGroup);
	float getFOV()				{ return _FOV; }
	NLMISC::CVector getPos()	{ return _Pos; }
	NLMISC::CVector getTarget() { return _Target; }

	void setRoll(float f)	{ _Roll = f; }
	float getRoll()	const	{ return _Roll; }
	void setFOV(float f)	{ _FOV = f; }
	float getFOV()	const	{ return _FOV; }

	void setPosX(float f)	{ _Pos.x = f; _Dist = (_Pos-_Target).norm(); }
	void setPosY(float f)	{ _Pos.y = f; _Dist = (_Pos-_Target).norm(); }
	void setPosZ(float f)	{ _Pos.z = f; _Dist = (_Pos-_Target).norm(); }

	float getPosX()	const	{ return _Pos.x; }
	float getPosY()	const	{ return _Pos.y; }
	float getPosZ()	const	{ return _Pos.z; }

	void setTgtX(float f)	{ _Target.x = f; _Dist = (_Pos-_Target).norm(); }
	void setTgtY(float f)	{ _Target.y = f; _Dist = (_Pos-_Target).norm(); }
	void setTgtZ(float f)	{ _Target.z = f; _Dist = (_Pos-_Target).norm(); }

	float getTgtX()	const	{ return _Target.x; }
	float getTgtY()	const	{ return _Target.y; }
	float getTgtZ()	const	{ return _Target.z; }

	REFLECT_EXPORT_START(CInterface3DCamera, CInterfaceElement)
		REFLECT_FLOAT ("posx", getPosX, setPosX);
		REFLECT_FLOAT ("posy", getPosY, setPosY);
		REFLECT_FLOAT ("posz", getPosZ, setPosZ);
		REFLECT_FLOAT ("tgtx", getTgtX, setTgtX);
		REFLECT_FLOAT ("tgty", getTgtY, setTgtY);
		REFLECT_FLOAT ("tgtz", getTgtZ, setTgtZ);
		REFLECT_FLOAT ("fov", getFOV, setFOV);
		REFLECT_FLOAT ("roll", getRoll, setRoll);
	REFLECT_EXPORT_END

	float getRotZ()	const	{ return _Rot.z; }
	void setRotZ(float f)	{ _Rot.z = f; }

	float getRotY()	const	{ return _Rot.y; }
	void setRotY(float f)	{ _Rot.y = f; }

	float getDist()	const	{ return _Dist; }
	void setDist(float f)	{ _Dist = f; }

	void reset(); // Reset user interaction

protected:

	NLMISC::CVector _Rot;
	float			_Dist;

	NLMISC::CVector _Pos;
	NLMISC::CVector _Target;
	float _Roll;
	float _FOV;
};

/**
 * class managing light 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DLight : public CInterfaceElement
{
public:
	CInterface3DLight()
	{
		_Pos = NLMISC::CVector(0,0,0);
		_Near = 1.0f;
		_Far = 4.0f;
		_Color = NLMISC::CRGBA(255,255,255);
		_Light = NULL;
	}

	virtual ~CInterface3DLight();

	virtual bool parse (xmlNodePtr cur, CInterface3DScene *parentGroup);

	float getPosX() const	{ return _Pos.x; }
	float getPosY() const	{ return _Pos.y; }
	float getPosZ() const	{ return _Pos.z; }

	void setPosX(float f);
	void setPosY(float f);
	void setPosZ(float f);

	float getNear() const	{ return _Near; }
	float getFar() const	{ return _Far; }

	void setNear(float f);
	void setFar(float f);

	sint32 getColR() const	{ return _Color.R; }
	sint32 getColG() const	{ return _Color.G; }
	sint32 getColB() const	{ return _Color.B; }

	void setColR(sint32 f);
	void setColG(sint32 f);
	void setColB(sint32 f);

	REFLECT_EXPORT_START(CInterface3DLight, CInterfaceElement)
		REFLECT_FLOAT ("posx", getPosX, setPosX);
		REFLECT_FLOAT ("posy", getPosY, setPosY);
		REFLECT_FLOAT ("posz", getPosZ, setPosZ);
		REFLECT_FLOAT ("near", getNear, setNear);
		REFLECT_FLOAT ("far",  getFar, setFar);
		REFLECT_SINT32 ("colr", getColR, setColR);
		REFLECT_SINT32 ("colg", getColG, setColG);
		REFLECT_SINT32 ("colb", getColB, setColB);
	REFLECT_EXPORT_END

protected:

	NLMISC::CVector _Pos;
	float _Near, _Far;
	NLMISC::CRGBA _Color;

	NL3D::UPointLight _Light;
};


/**
 * class managing fx 3d elements
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterface3DFX : public CInterfaceElement
{
public:
	CInterface3DFX()
	{
		_Pos = NLMISC::CVector(0,0,0);
		_FX = NULL;
	}

	virtual ~CInterface3DFX();

	virtual bool parse (xmlNodePtr cur, CInterface3DScene *parentGroup);

	virtual void checkCoords();

	NL3D::UParticleSystemInstance getPS() { return _FX; }

	float getPosX () const;
	float getPosY () const;
	float getPosZ () const;

	void setPosX (float f);
	void setPosY (float f);
	void setPosZ (float f);

	float getRotX () const;
	float getRotY () const;
	float getRotZ () const;

	void setRotX (float f);
	void setRotY (float f);
	void setRotZ (float f);

	std::string getName() const;
	void        setName (const std::string &ht);

	bool getStarted() const;
	void setStarted (bool b);

	REFLECT_EXPORT_START(CInterface3DFX, CInterfaceElement)
		REFLECT_FLOAT ("posx", getPosX, setPosX);
		REFLECT_FLOAT ("posy", getPosY, setPosY);
		REFLECT_FLOAT ("posz", getPosZ, setPosZ);
		REFLECT_FLOAT ("rotx", getRotX, setRotX);
		REFLECT_FLOAT ("roty", getRotY, setRotY);
		REFLECT_FLOAT ("rotz", getRotZ, setRotZ);
		REFLECT_STRING ("name", getName, setName);
		REFLECT_BOOL ("started", getStarted, setStarted);
	REFLECT_EXPORT_END

protected:

	NL3D::UParticleSystemInstance _FX;

	NLMISC::CVector _Pos;
	NLMISC::CVector _Rot;
	std::string		_Name;
};


#endif // RZ_INTERFACE_SCENE_3D_H

/* end of interface_3d_scene.h */


