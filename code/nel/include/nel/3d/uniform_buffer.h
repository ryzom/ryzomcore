/*

Copyright (C) 2015  Jan Boon <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NL_UNIFORM_BUFFER_H
#define NL_UNIFORM_BUFFER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/vector.h>
#include <nel/misc/vector_2d.h>
#include <nel/misc/vector_h.h>
#include <nel/misc/matrix.h>

#include <nel/3d/uniform_buffer_format.h>

#define NL3D_UNIFORM_BUFFER_DEBUG 1

namespace NLMISC {
	class CMatrix;
}

namespace NL3D {

class CUniformBuffer;
class IUBDrvInfos;
typedef	std::list<IUBDrvInfos*> TUBDrvInfoPtrList;
typedef	TUBDrvInfoPtrList::iterator ItUBDrvInfoPtrList;

/*
**** IMPORTANT ********************
**** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
***********************************
*/

// Uniform buffer
class CUniformBuffer : public NLMISC::CRefCount
{
public:
	CUniformBuffer();
	~CUniformBuffer();

	void *lock();
	void unlock();

	inline void set(sint offset, float f) { reinterpret_cast<float &>(m_HostMemory[offset]) = f; }
	inline void set(sint offset, float f0, float f1) { float *f = reinterpret_cast<float *>(&m_HostMemory[offset]); f[0] = f0; f[1] = f1; }
	inline void set(sint offset, float f0, float f1, float f2) { float *f = reinterpret_cast<float *>(&m_HostMemory[offset]); f[0] = f0; f[1] = f1; f[2] = f2; }
	inline void set(sint offset, float f0, float f1, float f2, float f3) { float *f = reinterpret_cast<float *>(&m_HostMemory[offset]); f[0] = f0; f[1] = f1; f[2] = f2; f[3] = f3; }
	inline void set(sint offset, NLMISC::CVector2d vec2) { reinterpret_cast<NLMISC::CVector2d &>(m_HostMemory[offset]) = vec2; }
	inline void set(sint offset, NLMISC::CVector vec3) { reinterpret_cast<NLMISC::CVector &>(m_HostMemory[offset]) = vec3; }
	inline void set(sint offset, NLMISC::CVectorH vec4) { reinterpret_cast<NLMISC::CVectorH &>(m_HostMemory[offset]) = vec4; }
	inline void set(sint offset, NLMISC::CMatrix mat4)  { float *f = reinterpret_cast<float *>(&m_HostMemory[offset]); mat4.get(f); }

private:
	std::vector<char> m_HostMemory;
	
public:
	CUniformBufferFormat Format;

public: // Driver-only
	NLMISC::CRefPtr<IUBDrvInfos> DrvInfos;
	bool Touched;

#if NL3D_UNIFORM_BUFFER_DEBUG
	sint Locked;
#endif

};

// TODO: Investigate if more efficient or not to do like vertex_buffer and call into driver for locking (what are best practices for updating uniform buffers?)
class IUBDrvInfos : public NLMISC::CRefCount
{
protected:
	IDriver				*_Driver;
private:
	ItUBDrvInfoPtrList	_DriverIterator;

public:
	NLMISC::CRefPtr<CUniformBuffer>	UniformBufferPtr;

	IUBDrvInfos(IDriver	*drv, ItUBDrvInfoPtrList it, CUniformBuffer *ub) { _Driver = drv; _DriverIterator = it; UniformBufferPtr = ub; }

	virtual ~IUBDrvInfos();
};

} /* namespace NL3D */

#endif /* #ifndef NL_UNIFORM_BUFFER_H */

/* end of file */
