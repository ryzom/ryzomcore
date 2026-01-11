// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#include "stdpch.h"
#include "nel/gui/proc.h"

#include "nel/misc/algo.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

#define	PROC_PARAM_IDENT	'@'

	// ***************************************************************************
	void CProcAction::buildParamBlock( const std::string &params )
	{
		buildBlocks( params, ParamBlocks );
	}

	void CProcAction::buildParams( const std::vector< std::string > &paramList, std::string &params ) const
	{
		eval( paramList, ParamBlocks, params );
	}

	void CProcAction::buildCondBlock( const std::string &params )
	{
		buildBlocks( params, CondBlocks );
	}

	void CProcAction::buildCond( const std::vector< std::string > &paramList, std::string &params ) const
	{
		eval( paramList, CondBlocks, params );
	}

	// ***************************************************************************
	void CProcAction::buildBlocks( const std::string &in, std::vector< CParamBlock > &out )
	{
		out.clear();

		if(in.empty())
			return;

		std::string lastString;
		std::string::size_type curPos= 0;
		std::string::size_type lastPos= 0;

		//if it has some @ then solve proc value
		while( (curPos=in.find(PROC_PARAM_IDENT, curPos)) != std::string::npos)
		{
			// If it is end of line
			if(curPos==in.size()-1)
			{
				// then skip
				curPos= in.size();
			}
			else
			{
				// Skip all @
				uint countNbIdent = 0;
				while (curPos<in.size() && in[curPos]==PROC_PARAM_IDENT)
				{
					curPos++;
					countNbIdent++;
				}

				// get the id pos
				uint countNbDigit = 0;
				uint startIdPos= (uint)curPos;
				while (curPos<in.size() && in[curPos]>='0' && in[curPos]<='9')
				{
					curPos++;
					countNbDigit++;
				}

				if (curPos == startIdPos)
				{
					// No digit so it is a normal db entry
					lastString+= in.substr (lastPos, curPos-(countNbIdent-1)-lastPos);
					// all @ are skipped
				}
				else
				{
					// There is some digit it is an argument

					// copy the last not param sub string.
					sint nbToCopy = (sint)(curPos-countNbIdent-countNbDigit-lastPos);
					if (nbToCopy > 0)
						lastString += in.substr(lastPos, nbToCopy);

					// if not empty, add to the param block
					if (!lastString.empty())
					{
						CParamBlock pb;
						pb.String = lastString;
						out.push_back(pb);
						// clear it
						lastString.clear();
					}

					// get the param id
					sint paramId;
					fromString(in.substr(startIdPos, curPos-startIdPos), paramId);
					// Add it to the param block
					CParamBlock	 pb;
					pb.NumParam = paramId;
					out.push_back(pb);
				}

				// valid pos is current pos
				lastPos= curPos;
			}
		}
		// concat last part
		lastString+= in.substr(lastPos, in.size()-lastPos);
		if(!lastString.empty())
		{
			CParamBlock pb;
			pb.String = lastString;
			out.push_back(pb);
		}
	}

	// ***************************************************************************
	void CProcAction::eval( const std::vector<std::string> &inArgs, const std::vector< CParamBlock > &inBlocks, std::string &out )
	{
		// clear the ret string
		out.clear();

		// for all block
		for (uint i=0; i < inBlocks.size(); i++)
		{
			const CParamBlock &pb = inBlocks[i];
			// if the block is a raw string
			if (pb.NumParam < 0)
			{
				// concat with the block
				out += pb.String;
			}
			// else get from paramList
			else
			{
				// add 1, because paramList[0] is the name of the procedure
				sint idInList = pb.NumParam+1;
				// if param exist
				if (idInList < (sint)inArgs.size())
					// concat with the params
					out += inArgs[idInList];
				// else skip (should fail)
			}
		}
	}
}

