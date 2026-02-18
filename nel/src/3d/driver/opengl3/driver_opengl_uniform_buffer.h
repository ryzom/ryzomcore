// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_DRIVER_OPENGL_UNIFORM_BUFFER_H
#define NL_DRIVER_OPENGL_UNIFORM_BUFFER_H

#include <nel/misc/types_nl.h>

#include <nel/3d/uniform_buffer_format.h>
#include <nel/3d/uniform_buffer.h>

#define NL3D_GL3_UNIFORM_BUFFER_DEBUG 1

namespace NL3D {
namespace NLDRIVERGL3 {

// NOTE: It is completely safe to reorder these indices.
// When changing, update GLSLBuiltinHeader blocks.
// Always use the defines.
#define NL_BUILTIN_CAMERA_BINDING 0        // Builtin: camera/fog/clip state
#define NL_BUILTIN_LIGHT_TABLE_BINDING 1   // Builtin: shared light table
#define NL_BUILTIN_MODEL_BINDING 2         // Builtin: per-object matrices/lighting
#define NL_BUILTIN_MATERIAL_BINDING 3      // Builtin: per-material properties
#define NL_USER_VERTEX_PROGRAM_BINDING 4   // User VP UBO
#define NL_USER_PIXEL_PROGRAM_BINDING 5    // User PP UBO

// Driver-side GLSL headers prepended to shaders (after #version and preprocessor lines).
// Each header is inserted independently based on CProgramFeatures flags.
extern const char *GLSLLightTableHeader; // NlLightTable UBO block (UsesLightTableUBO)
extern const char *GLSLCameraHeader;     // NlCamera UBO block (UsesCameraUBO)
extern const char *GLSLObjectHeader;     // NlModel UBO block (UsesObjectUBO)
extern const char *GLSLMaterialHeader;   // NlMaterial UBO block (UsesMaterialUBO)

void generateUniformBufferGLSL(std::stringstream &ss, const CUniformBufferFormat &ubf, sint binding);

class CUBDrvInfosGL3 : public IUBDrvInfos
{
public:
	CUBDrvInfosGL3(IDriver *drv, ItUBDrvInfoPtrList it, CUniformBuffer *ub);
	virtual ~CUBDrvInfosGL3();

	GLuint getBufferId() const { return _BufferId; }
	sint getCapacity() const { return _Capacity; }
	void setCapacity(sint cap) { _Capacity = cap; }

private:
	GLuint _BufferId;
	sint _Capacity;
};

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL_UNIFORM_BUFFER_H

/* end of file */
