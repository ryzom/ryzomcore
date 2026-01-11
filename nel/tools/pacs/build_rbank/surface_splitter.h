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

#ifndef NL_SURFACE_SPLITTER_H
#define NL_SURFACE_SPLITTER_H

#include "nel/misc/types_nl.h"

#include "nel/pacs/local_retriever.h"
#include "nel/pacs/quad_grid.h"

#include <map>

/**
 * TODO Class description
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CSurfaceSplitter
{
public:

	///
	class CVector2s64;
	class CSurfaceId;
	class CChain;
	class CLoop;
	class CSurface;

	///
	class CFixed64
	{
	protected:
		sint64	_Value;
	public:
		CFixed64() : _Value(0) {}
		CFixed64(const sint64 &v) : _Value(v) {}
		CFixed64(const double &v) : _Value((sint64)(v*4294967296.0)) {}
		CFixed64(const CFixed64 &v) : _Value(v._Value) {}

		operator double(void) const							{ return ((double)_Value) / 4294967296.0; }
		operator float(void) const							{ return ((float)_Value) / 4294967296.0f; }
		operator sint64(void) const							{ return _Value; }

		CFixed64	operator + (const CFixed64 &f) const	{ return CFixed64(_Value + f._Value); }
		CFixed64	operator - (const CFixed64 &f) const	{ return CFixed64(_Value - f._Value); }
		CFixed64	operator - () const						{ return CFixed64(-_Value); }
		CFixed64	operator * (const CFixed64 &f) const
		{
			sint64	vh = _Value>>32;
			sint64	vl = _Value&0xffffffff;
			sint64	uh = f._Value>>32;
			sint64	ul = f._Value&0xffffffff;

			sint64	res = ((vh*uh)<<32)+(vh*ul+uh*vl)+((uint64)(vl*ul)>>32);
			double	fres = ((double)res) / 4294967296.0;
			double	cfres = (double)(*this) * (double)f;
			return CFixed64(res);
		}
		CFixed64	operator / (const CFixed64 &f) const
		{
			if (f._Value == 0)
				return CFixed64();

			sint	sign = 0;
			uint64	a = _Value;
			uint64	b = f._Value;

			if ((sint64)a < 0)
				a = -(sint64)a, sign ^= 1;
			if ((sint64)b < 0)
				b = -(sint64)b, sign ^= 1;

			uint64	lb = INT64_CONSTANT (0x8000000000000000);
			uint64	q = 0;
			uint64	nh = a>>32;
			uint64	nl = a<<32;

			while (lb != 0)
			{
				if (nh >= b)
				{
					q |= (lb+lb);
					nh -= b;
				}
				nh = (nh+nh) + (nl&lb ? 1 : 0);
				lb >>= 1;
			}

			// round final bit
			if (nh >= b)
				++q;

			CFixed64	result(sign ? -(sint64)q : +(sint64)q );
			return result;
		}

		CFixed64	&operator += (const CFixed64 &f) 		{ _Value+=f._Value; return *this; }
		CFixed64	&operator -= (const CFixed64 &f)		{ _Value-=f._Value; return *this; }
		CFixed64	&operator *= (const CFixed64 &f)		{ *this = *this*f; return *this; }
		CFixed64	&operator /= (const CFixed64 &f)		{ *this = *this/f; return *this; }

		bool		operator == (const CFixed64 &f) const	{ return _Value == f._Value; }
		bool		operator != (const CFixed64 &f) const	{ return _Value != f._Value; }
		bool		operator < (const CFixed64 &f) const	{ return _Value < f._Value; }
		bool		operator <= (const CFixed64 &f) const	{ return _Value <= f._Value; }
		bool		operator >= (const CFixed64 &f) const	{ return _Value >= f._Value; }
		bool		operator > (const CFixed64 &f) const	{ return _Value > f._Value; }

		double		sqnorm() const							{ return (double)(*this * *this); }
		double		norm() const							{ return sqrt(sqnorm()); }
	};

	///
	class CVector2s64
	{
	public:
		CFixed64	x;
		CFixed64	y;

		CVector2s64() : x(), y() {}
		CVector2s64(const sint64 &X, const sint64 &Y) : x(X), y(Y) {}
		CVector2s64(const CFixed64 &X, const CFixed64 &Y) : x(X), y(Y) {}
		CVector2s64(const NLMISC::CVector &v) : x(v.x), y(v.y) {}
		CVector2s64(const NLMISC::CVectorD &v) : x(v.x), y(v.y) {}
		CVector2s64(const NLPACS::CVector2s &v) : x(((sint64)v.x) << 25), y(((sint64)v.y) << 25) {}

		CVector2s64		operator + (const CVector2s64 &v) const	{ return CVector2s64(x+v.x, y+v.y); }
		CVector2s64		operator - (const CVector2s64 &v) const	{ return CVector2s64(x-v.x, y-v.y); }
		CVector2s64		operator * (sint64 s) const				{ return CVector2s64(x*CFixed64(s), y*CFixed64(s)); }
		CVector2s64		operator * (CFixed64 s) const			{ return CVector2s64(x*s, y*s); }
		CVector2s64		operator * (double s) const				{ return CVector2s64(x*CFixed64(s), y*CFixed64(s)); }
		CVector2s64		operator / (sint64 s) const				{ return CVector2s64(x*CFixed64(s), y*CFixed64(s)); }
		CVector2s64		operator / (CFixed64 s) const			{ return CVector2s64(x*s, y*s); }
		CVector2s64		operator / (double s) const				{ return CVector2s64(x*CFixed64(s), y*CFixed64(s)); }
		CFixed64		operator * (const CVector2s64 &v) const	{ return x*v.x + y*v.y; }
		CFixed64		operator ^ (const CVector2s64 &v) const	{ return x*v.y - y*v.x; }

		bool			operator == (const CVector2s64 &v) const	{ return x == v.x && y == v.y; }
		bool			operator != (const CVector2s64 &v) const	{ return x != v.x || y != v.y; }

		CVector2s64		&operator += (const CVector2s64 &v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}
		CVector2s64		&operator -= (const CVector2s64 &v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}
		CVector2s64		&operator *= (sint64 s)
		{
			CFixed64	ss(s);
			x *= ss;
			y *= ss;
			return *this;
		}
		CVector2s64		&operator *= (double s)
		{
			CFixed64	ss(s);
			x *= ss;
			y *= ss;
			return *this;
		}

		NLMISC::CVector	asVector() const
		{
			return NLMISC::CVector((float)x, (float)y, 0.0f);
		}
		NLMISC::CVectorD	asVectorD() const
		{
			return NLMISC::CVectorD((double)x, (double)y, 0.0);
		}
	};

	///
	class CSurfaceId
	{
	public:
		uint16	Instance;
		uint16	Surface;
		uint16	SubSurface;

		CSurfaceId(uint16 instance=65535, uint16 surface=65535, uint16 subsurface=0) : 
			Instance(instance),
			Surface(surface),
			SubSurface(subsurface) {}

		bool	operator == (const CSurfaceId &id) const	{ return Instance == id.Instance && Surface == id.Surface && SubSurface == id.SubSurface; }
		bool	operator != (const CSurfaceId &id) const	{ return !(*this == id); }

		bool	operator < (const CSurfaceId &id) const
		{
			if (Instance < id.Instance)
				return true;
			else if (Instance > id.Instance)
				return false;
			else if (Surface < id.Surface)
				return true;
			else if (Surface > id.Surface)
				return false;
			else
				return SubSurface < id.SubSurface;
		}

		bool	operator > (const CSurfaceId &id) const		{ return id < *this; }
		bool	operator <= (const CSurfaceId &id) const	{ return *this == id || *this < id; }
		bool	operator >= (const CSurfaceId &id) const	{ return *this == id || id < *this; }
	};

	///
	class CChainId
	{
	public:
		CChainId(uint32 id=0) : Id(id) {}
		uint32		Id;

		bool	operator == (const CChainId &id) const		{ return Id == id.Id; }
		bool	operator != (const CChainId &id) const		{ return !(*this == id); }
		bool	operator < (const CChainId &id) const		{ return Id < id.Id; }
		bool	operator > (const CChainId &id) const		{ return id < *this; }
		bool	operator <= (const CChainId &id) const	{ return *this == id || *this < id; }
		bool	operator >= (const CChainId &id) const	{ return *this == id || id < *this; }
	};


	///
	class CEdgeId
	{
	public:
		CEdgeId(const CChainId &chain=CChainId(), uint edge=0) : Chain(chain), Edge(edge) {}
		CChainId				Chain;
		uint					Edge;
	};

	typedef NLPACS::CQuadGrid<CEdgeId>	TEdgeGrid;


	///
	class CTipId
	{
	public:
		CTipId(uint32 id=0) : Id(id) {}
		uint32		Id;

		bool	operator == (const CTipId &id) const		{ return Id == id.Id; }
		bool	operator != (const CTipId &id) const		{ return !(*this == id); }
		bool	operator < (const CTipId &id) const		{ return Id < id.Id; }
		bool	operator > (const CTipId &id) const		{ return id < *this; }
		bool	operator <= (const CTipId &id) const	{ return *this == id || *this < id; }
		bool	operator >= (const CTipId &id) const	{ return *this == id || id < *this; }
	};




	///
	class CChain
	{
	public:
		CChainId							Id;
		CSurfaceId							Left;
		CSurfaceId							Right;
		CTipId								Start;
		CTipId								Stop;
		std::vector<CVector2s64>			Vertices;
		std::vector<TEdgeGrid::CIterator>	Iterators;
		bool								DontSplit;

		///
		CChain() : DontSplit(false) {}

		///
		void		dump(bool forward) const
		{
			NLMISC::InfoLog->displayRawNL("Chain:%d[Left=%d:%d:%d,Right=%d:%d:%d]", Id.Id, Left.Instance, Left.Surface, Left.SubSurface, Right.Instance, Right.Surface, Right.SubSurface);
			sint	i, j=0;
			if (forward)
			{
				for (i=0; i<(sint)Vertices.size(); ++i, ++j)
					NLMISC::InfoLog->displayRawNL("%d;%.3f;%.3f", j, (double)Vertices[i].x, (double)Vertices[i].y);
			}
			else
			{
				for (i=(sint)Vertices.size()-1; i>=0; --i, ++j)
					NLMISC::InfoLog->displayRawNL("%d;%.3f;%.3f", j, (double)Vertices[i].x, (double)Vertices[i].y);
			}
		}
	};



	///
	class CTip
	{
	public:
		CTipId						Id;
		CVector2s64					Tip;
		std::vector<CChainId>		Chains;
	};



	///
	class CLoop
	{
	public:
		CSurfaceId					Surface;
		std::vector<CChainId>		Chains;

		///
		void		dump(const CSurfaceSplitter &splitter) const
		{
			uint	i;
			for (i=0; i<Chains.size(); ++i)
			{
				const CChain	*chain = splitter.getChain(Chains[i]);
				if (chain != NULL)
					chain->dump(Surface == chain->Left);
			}
		}

		///
		void		removeChain(CChainId chain)
		{
			std::vector<CChainId>::iterator	it;
			for (it=Chains.begin(); it!=Chains.end(); )
			{
				if (*it == chain)
					it = Chains.erase(it);
				else
					++it;
			}
		}

		///
		class iterator
		{
		public:
			CSurfaceSplitter	*pSplitter;
			CLoop				*pLoop;
			sint				Chain;
			CChain				*pChain;
			sint				ChainVertex;
			sint8				Direction;
			sint8				ChainDirection;

			iterator(CSurfaceSplitter *splitter=NULL, CLoop *loop=NULL, bool forward=true) : pSplitter(splitter), pLoop(loop), Chain(0), pChain(NULL), ChainVertex(0), Direction(0), ChainDirection(0)
			{
				Direction = forward ? +1 : -1;
				if (splitter == NULL || pLoop == NULL)
					return;
				Chain = (Direction>0 ? 0 : (sint)pLoop->Chains.size()-1);
				resetChain();
			}
			iterator(const iterator &it)
			{
				*this = it;
			}

			iterator	&operator = (const iterator &it)
			{
				pSplitter = it.pSplitter;
				pLoop = it.pLoop;
				Chain = it.Chain;
				pChain = it.pChain;
				ChainVertex = it.ChainVertex;
				Direction = it.Direction;
				return *this;
			}
			bool		operator == (const iterator &it)
			{
				return	pSplitter == it.pSplitter &&
						pLoop == it.pLoop &&
						Chain == it.Chain &&
						ChainVertex == it.ChainVertex;
			}
			bool		operator != (const iterator &it)	{ return !(*this == it); }

			// ++it
			iterator	&operator ++ ()
			{
				ChainVertex += ChainDirection;
				if (ChainVertex == 0 || ChainVertex == (sint)pChain->Vertices.size()-1)
				{
					Chain += Direction;
					resetChain();
				}

				return *this;
			}

			// it++
			iterator	operator ++ (int)
			{
				iterator tmp(*this);

				ChainVertex += ChainDirection;
				if (ChainVertex == 0 || ChainVertex == (sint)pChain->Vertices.size()-1)
				{
					Chain += Direction;
					resetChain();
				}

				return tmp;
			}

			void	resetChain()
			{
				if (Chain < 0 || Chain == (sint)pLoop->Chains.size())
				{
					pSplitter = NULL;
					pLoop = NULL;
					Chain = 0;
					pChain = NULL;
					ChainVertex = 0;
					Direction = 0;
					ChainDirection = 0;
					return;
				}
				pChain = pSplitter->getChain(pLoop->Chains[Chain]);
				ChainDirection = (pChain->Left == pLoop->Surface ? Direction : -Direction);
				ChainVertex = (ChainDirection>0 ? 0 : (sint)pChain->Vertices.size()-1);
			}

			CVector2s64	operator * () const
			{
				return pChain->Vertices[ChainVertex];
			}
		};
	};



	///
	class CSurface
	{
	public:
		CSurfaceId					Id;
		std::vector<CLoop>			Loops;

		///
		void	dump(const CSurfaceSplitter &splitter) const
		{
			NLMISC::InfoLog->displayRawNL("---Surface:%d:%d:%d", Id.Instance, Id.Surface, Id.SubSurface);
			uint	i;
			for (i=0; i<Loops.size(); ++i)
			{
				NLMISC::InfoLog->displayRawNL("Loop:%d", i);
				Loops[i].dump(splitter);
			}
		}

		///
		void	removeChain(CChainId chain)
		{
			uint	i;
			for (i=0; i<Loops.size(); ++i)
				Loops[i].removeChain(chain);
		}
	};

protected:
	typedef std::map<CSurfaceId, CSurface>	TSurfaceMap;
	typedef std::map<CChainId, CChain>		TChainMap;
	typedef std::map<CTipId, CTip>			TTipMap;

	TSurfaceMap				_Surfaces;
	uint					_NumSurfaces;
	TChainMap				_Chains;
	uint					_NumChains;
	TTipMap					_Tips;
	uint					_NumTips;

	TEdgeGrid				_Edges;


public:

	/// Constructor
	CSurfaceSplitter();

	///
	void		build(NLPACS::CLocalRetriever &lr);

	///
	CSurface	*getSurface(const CSurfaceId &id)
	{
		TSurfaceMap::iterator	it = _Surfaces.find(id);
		return (it == _Surfaces.end() ? NULL : &((*it).second));
	}

	///
	CChain		*getChain(const CChainId &id)
	{
		TChainMap::iterator		it = _Chains.find(id);
		return (it == _Chains.end() ? NULL : &((*it).second));
	}

	///
	const CSurface	*getSurface(const CSurfaceId &id) const
	{
		TSurfaceMap::const_iterator	it = _Surfaces.find(id);
		return (it == _Surfaces.end() ? NULL : &((*it).second));
	}

	///
	const CChain	*getChain(const CChainId &id) const
	{
		TChainMap::const_iterator	it = _Chains.find(id);
		return (it == _Chains.end() ? NULL : &((*it).second));
	}



protected:

	///
	void	buildChain(NLPACS::CLocalRetriever &lr, uint chain);

	///
	void	buildSurface(NLPACS::CLocalRetriever &lr, uint surface);



	///
	void	initEdgeGrid();



	///
	void	splitChains();

	///
	void	splitChain(TChainMap::iterator it, uint &numInters);



	///
	void	dump() const
	{
		TSurfaceMap::const_iterator	it;

		for (it=_Surfaces.begin(); it!=_Surfaces.end(); ++it)
			(*it).second.dump(*this);
	}

	///
	void	dumpChain(CChainId chain) const
	{
		TChainMap::const_iterator	it = _Chains.find(chain);
		if (it == _Chains.end())
			return;

		(*it).second.dump(true);
	}



	///
	bool	intersect(const CVector2s64 &v0, const CVector2s64 &v1,
					  const CVector2s64 &c0, const CVector2s64 &c1,
					  CVector2s64 &intersect, CFixed64 &ndist);




	///
	CChainId	addChain(const CSurfaceId &left, const CSurfaceId &right, const std::vector<CVector2s64> &points, bool dontSplit=false);

	///
	void		removeChain(CChainId chain);

	///
	void		replaceChain(CChainId chain, const std::vector<CChainId> &chains);
};


#endif // NL_SURFACE_SPLITTER_H

/* End of surface_splitter.h */
