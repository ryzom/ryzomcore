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

#include "std3d.h"
#include <nel/3d/uniform_buffer.h>

#include <nel/misc/debug.h>

namespace NL3D {

CUniformBuffer::CUniformBuffer()
{
	nlctassert(sizeof(float) == 4);
	nlctassert(sizeof(NLMISC::CVector2f) == 8);
	nlctassert(sizeof(NLMISC::CVector) == 12);
	nlctassert(sizeof(NLMISC::CVectorH) == 16);

	// ...
}

CUniformBuffer::~CUniformBuffer()
{
	/* ***********************************************
	 *  WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *  It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	if (DrvInfos)
		DrvInfos->UniformBufferPtr = NULL; // Tell the driver info to not restaure memory when it will die

	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

void *CUniformBuffer::lock()
{
	m_HostMemory.reserve(Format.size());
	return &m_HostMemory[0];

#if NL3D_UNIFORM_BUFFER_DEBUG
	++Locked;
#endif
}

void CUniformBuffer::unlock()
{
#if NL3D_UNIFORM_BUFFER_DEBUG
	--Locked;
	nlassert(Locked >= 0);
#endif

	Touched = true;
}

IUBDrvInfos::~IUBDrvInfos()
{
	// TODO: _Driver->removeUBDrvInfoPtr(_DriverIterator);
}

} /* namespace NL3D */

/* end of file */
