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


#ifndef I_PROGRAM_OBJECT_H
#define I_PROGRAM_OBJECT_H

namespace NL3D
{

	class IProgram;

	class IProgramObject
	{
	public:

		enum EUniform
		{
			MVPMatrix,
			MVMatrix,
			TexMatrix0,
			TexMatrix1,
			TexMatrix2,
			TexMatrix3,
			Constant0,
			Constant1,
			Constant2,
			Constant3,
			Diffuse,
			Color,
			Sampler0,
			Sampler1,
			Sampler2,
			Sampler3,
			AlphaTreshold,
			NUM_UNIFORMS
		};

		IProgramObject(){}

		virtual ~IProgramObject(){}

		virtual bool attachVertexProgram( IProgram *shader ) = 0;

		virtual bool attachPixelProgram( IProgram *shader ) = 0;

		virtual bool detachVertexProgram( IProgram *shader ) = 0;

		virtual bool detachPixelProgram( IProgram *shader ) = 0;

		unsigned int getProgramId() const{ return programId; }

		virtual bool link( std::string &log ) = 0;

		virtual bool validate( std::string &log ) = 0;

		virtual void cacheUniformIndices() = 0;

		virtual int getUniformIndex( EUniform uniform ) = 0;

		bool isLinked() const{ return linked; }

	protected:
		unsigned int programId;
		bool linked;
	};

}

#endif

