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

#ifndef NL_PRIM_CHECKER_H
#define NL_PRIM_CHECKER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

#include <nel/misc/polygon.h>

namespace NLMISC
{
	class CVectorD;
};

namespace NLLIGO
{
	class IPrimitive;
	class CPrimZone;
};

/**
 * A class that reads .primitive files from ligo and that allows
 * to define flags over pacs surfaces with CPrimZone
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CPrimChecker
{
public:

	/// Constructor
	CPrimChecker();


	/// Bits value for landscape
	enum
	{
		Include = 1,
		Exclude = 2,
		ClusterHint = 4,
		Water = 8,
		Cliff = 16
	};


	/// Init CPrimChecker
	bool	build(const std::string &primitivesPath, const std::string &igLandPath, const std::string &igVillagePath, const std::string &outputDirectory = "./", bool forceRebuild = false);

	/// load CPrimChecker state
	bool	load(const std::string &outputDirectory = "./");

	/// Reads bits value at a given position
	uint8	get(uint x, uint y)	const	{ return _Grid.get(x, y); }

	/// Reads bits value at a given position
	uint16	index(uint x, uint y) const	{ return _Grid.index(x, y); }

	/// Gete water height
	float	waterHeight(uint index, bool &exists) const
	{
		if (index >= _WaterHeight.size())
		{
			exists = false;
			return 0.0;
		}
		exists = true;
		return _WaterHeight[index];
	}

	/// Render a CPolygon of bit value
	void	renderBits(const NLMISC::CPolygon &poly, uint8 bits);

private:

	/// \name Grid management
	//@{

	/**
	 * A class that allows to store 65536x65536 elements with minimum allocation and dynamic allocation of elements
	 */
	class CGrid
	{
	public:

		/// Constructor
		CGrid()		{ clear(); }

		/// Set bits in grid
		void	set(uint x, uint y, uint8 bits)
		{
			CGridCell	*cell = _Grid[((x&0xff00)>>8) + (y&0xff00)];
			if (cell == NULL)
			{
				cell = new CGridCell();
				_Grid[((x&0xff00)>>8) + (y&0xff00)] = cell;
			}
			cell->set(x, y, bits);
		}

		/// Get bits in grid
		uint8	get(uint x, uint y) const
		{
			CGridCell	*cell = _Grid[((x&0xff00)>>8) + (y&0xff00)];
			return cell != NULL ? cell->get(x, y) : (uint8)0;
		}

		/// Set bits in grid
		void	index(uint x, uint y, uint16 idx)
		{
			CGridCell	*cell = _Grid[((x&0xff00)>>8) + (y&0xff00)];
			if (cell == NULL)
			{
				cell = new CGridCell();
				_Grid[((x&0xff00)>>8) + (y&0xff00)] = cell;
			}
			cell->index(x, y, idx);
		}

		/// Get bits in grid
		uint16	index(uint x, uint y) const
		{
			CGridCell	*cell = _Grid[((x&0xff00)>>8) + (y&0xff00)];
			return cell != NULL ? cell->index(x, y) : (uint16)0;
		}


		/// Clear grid
		void	clear()
		{
			for (uint i=0; i<256*256; ++i)
				if (_Grid != NULL)
				{
					delete _Grid[i];
					_Grid[i] = NULL;
				}
		}

		/// Serializes
		void	serial(NLMISC::IStream &f)
		{
			f.serialCheck(NELID("PCHK"));
			f.serialVersion(0);

			if (f.isReading())
				clear();

			for (uint i=0; i<256*256; ++i)
			{
				bool	present = (_Grid[i] != NULL);
				f.serial(present);

				if (present)
				{
					if (_Grid[i] == NULL)
						_Grid[i] = new CGridCell();
					_Grid[i]->serial(f);
				}
			}
		}

	private:

		/**
		 * A class that allows to store 256x256 elements
		 */
		class CGridCell
		{
			class CGridElm
			{
				uint16		_Value;

			public:

				CGridElm() : _Value(0)	{}

				uint8		flags() const						{ return (uint8)((_Value >> 11) & 0x1f); }
				void		flags(uint8 bits)					{ _Value |= (((uint16)bits) << 11); }
				uint16		index() const						{ return _Value & 0x07ff; }
				void		index(uint16 idx)					{ _Value = ((idx & 0x07ff) | (_Value & 0xf800)); }

				void		serial(NLMISC::IStream &f)			{ f.serial(_Value); }
			};

		public:

			/// Constructor
			CGridCell()	{}

			/// Set bits in grid
			void	set(uint x, uint y, uint8 bits)		{ _Grid[(x&0xff) + ((y&0xff)<<8)].flags(bits); }

			/// Get bits in grid
			uint8	get(uint x, uint y) const			{ return _Grid[(x&0xff) + ((y&0xff)<<8)].flags(); }


			///
			uint16	index(uint x, uint y)				{ return _Grid[(x&0xff) + ((y&0xff)<<8)].index(); }
			///
			void	index(uint x, uint y, uint idx)		{ _Grid[(x&0xff) + ((y&0xff)<<8)].index(idx); }


			/// Serializes
			void	serial(NLMISC::IStream &f)			{ for (uint i=0; i<256*256; ++i) f.serial(_Grid[i]); }

		private:
			CGridElm	_Grid[256*256];
		};

		CGridCell		*_Grid[256*256];
	};

	CGrid				_Grid;

	std::vector<float>	_WaterHeight;

	//@}



	/// Reads a primitive file and renders contained primitives into grid
	void	readFile(const std::string &filename);

	/// Reads a primitive and its sub primitives
	void	readPrimitive(NLLIGO::IPrimitive *primitive);

	/// Renders a primitive
	void	render(NLLIGO::CPrimZone *zone, uint8 bits);

	/// Render a water shape, as a CPolygon
	void	render(const NLMISC::CPolygon &poly, uint16 value);
};


#endif // NL_PRIM_CHECKER_H

/* End of prim_checker.h */
