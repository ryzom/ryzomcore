/**
 * \file multipass_camera_effect_info.h
 * \brief CMultipassCameraEffectInfo
 * \date 2013-06-16 17:27GMT
 * \author Jan Boon (Kaetemi)
 * CMultipassCameraEffectInfo
 */

/* 
 * Copyright (C) 2013  by authors
 * 
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with NEVRAX NEL.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NL3D_MULTIPASS_CAMERA_EFFECT_INFO_H
#define NL3D_MULTIPASS_CAMERA_EFFECT_INFO_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace NL3D {
	class IMultipassCameraEffect;

enum TMultipassCameraType
{
	MULTIPASS_CAMERA_UNKNOWN,
	/// A multi pass effect used to support stereo displays.
	MULTIPASS_CAMERA_STEREO,
	/// A multi pass effect which renders at different animation deltas to create a motion blur (experimental).
	MULTIPASS_CAMERA_MOTION_BLUR,
	/// A multi pass effect which renders with modified camera settings to create depth of field (experimental).
	MULTIPASS_CAMERA_DEPTH_OF_FIELD,
	// etc.
};

/**
 * \brief CMultipassCameraEffectInfo
 * \date 2013-06-16 17:27GMT
 * \author Jan Boon (Kaetemi)
 * Information on a multi pass camera effect.
 */
struct CMultipassCameraEffectInfo
{
public:
	CMultipassCameraEffectInfo();
	virtual ~CMultipassCameraEffectInfo();
	
	/// Name used in configs etc. Use i18n for storing display name and descriptions.
	std::string Name;

	/// Type of multipass. Useful for filtering which ones the user can pick.
	TMultipassCameraType Type;

}; /* class CMultipassCameraEffectInfo */

/// Inherit from this class with a class which fills the public 
/// information fields and that overrides the create() method.
/// Driver has list of installed IMultipassCameraEffectInfoPriv.
struct IMultipassCameraEffectInfoPriv : public CMultipassCameraEffectInfo
{
public:
	IMultipassCameraEffectInfoPriv();
	virtual ~IMultipassCameraEffectInfoPriv();

	virtual IMultipassCameraEffect *create() const = 0;
};

} /* namespace NL3D */

#endif /* #ifndef NL3D_MULTIPASS_CAMERA_EFFECT_INFO_H */

/* end of file */
