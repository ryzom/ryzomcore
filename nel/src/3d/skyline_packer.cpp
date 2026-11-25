// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "std3d.h"
#include "nel/3d/skyline_packer.h"

#include <algorithm>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

// ***************************************************************************
CSkylinePacker::CSkylinePacker()
    : m_Width(0)
    , m_Height(0)
{
}

// ***************************************************************************
void CSkylinePacker::reset(uint width, uint height)
{
	m_Width = width;
	m_Height = height;
	m_Steps.clear();
	m_Gaps.clear();
	if (width)
	{
		CStep ground;
		ground.X = 0;
		ground.Y = 0;
		ground.Width = width;
		m_Steps.push_back(ground);
	}
}

// ***************************************************************************
bool CSkylinePacker::fitOnStep(uint step, uint width, uint &y) const
{
	// the caller has checked that the rectangle is no wider than the area
	const uint x = m_Steps[step].X;
	if (x > m_Width - width)
		return false;
	// the rectangle stands on the highest of the steps under it
	const uint right = x + width;
	y = 0;
	for (uint i = step; i < m_Steps.size() && m_Steps[i].X < right; ++i)
		y = std::max(y, m_Steps[i].Y);
	return true;
}

// ***************************************************************************
void CSkylinePacker::addGap(uint x, uint y, uint width, uint height)
{
	if (!width || !height)
		return;
	CGap gap;
	gap.X = x;
	gap.Y = y;
	gap.Width = width;
	gap.Height = height;
	m_Gaps.push_back(gap);
}

// ***************************************************************************
bool CSkylinePacker::placeInGap(uint width, uint height, uint &x, uint &y)
{
	// the gap with the least room that the rectangle fits in, the first of these
	const uint none = (uint)m_Gaps.size();
	uint best = none;
	uint64 bestRoom = 0;
	for (uint i = 0; i < m_Gaps.size(); ++i)
	{
		const CGap &gap = m_Gaps[i];
		if (width > gap.Width || height > gap.Height)
			continue;
		const uint64 room = (uint64)gap.Width * gap.Height;
		if (best == none || room < bestRoom)
		{
			best = i;
			bestRoom = room;
		}
	}
	if (best == none)
		return false;

	const CGap gap = m_Gaps[best];
	m_Gaps.erase(m_Gaps.begin() + best);
	x = gap.X;
	y = gap.Y;

	// The rectangle stands in the corner of the gap. What is left beside it and over it stays
	// as two gaps, cut so that the larger of the two sides left over stays in one piece.
	const uint restWidth = gap.Width - width;
	const uint restHeight = gap.Height - height;
	if (restWidth <= restHeight)
	{
		addGap(gap.X + width, gap.Y, restWidth, height);
		addGap(gap.X, gap.Y + height, gap.Width, restHeight);
	}
	else
	{
		addGap(gap.X + width, gap.Y, restWidth, gap.Height);
		addGap(gap.X, gap.Y + height, width, restHeight);
	}
	return true;
}

// ***************************************************************************
bool CSkylinePacker::place(uint width, uint height, uint &x, uint &y)
{
	if (!width || !height || width > m_Width || height > m_Height)
		return false;

	// room under the outline first: it is of no other use
	if (placeInGap(width, height, x, y))
		return true;

	// the step where the rectangle comes lowest, the leftmost of these
	const uint none = (uint)m_Steps.size();
	uint best = none;
	uint bestY = 0;
	for (uint i = 0; i < m_Steps.size(); ++i)
	{
		uint stepY;
		if (!fitOnStep(i, width, stepY))
			break; // the steps further right have less width behind them
		if (stepY > m_Height - height)
			continue;
		if (best == none || stepY < bestY)
		{
			best = i;
			bestY = stepY;
		}
	}
	if (best == none)
		return false;

	x = m_Steps[best].X;
	y = bestY;

	// The outline gets the top of the rectangle as a step, in place of the steps under the
	// rectangle; a step that is under it in part keeps the part to the right of it
	CStep top;
	top.X = x;
	top.Y = bestY + height;
	top.Width = width;
	const uint right = x + width;
	uint end = best;
	while (end < m_Steps.size() && m_Steps[end].X + m_Steps[end].Width <= right)
	{
		// the room between a lower step and the rectangle is a gap from here on
		addGap(m_Steps[end].X, m_Steps[end].Y, m_Steps[end].Width, bestY - m_Steps[end].Y);
		++end;
	}
	if (end < m_Steps.size() && m_Steps[end].X < right)
	{
		addGap(m_Steps[end].X, m_Steps[end].Y, right - m_Steps[end].X, bestY - m_Steps[end].Y);
		m_Steps[end].Width -= right - m_Steps[end].X;
		m_Steps[end].X = right;
	}
	m_Steps.erase(m_Steps.begin() + best, m_Steps.begin() + end);
	m_Steps.insert(m_Steps.begin() + best, top);

	// neighbours of one height are one step
	if (best + 1 < m_Steps.size() && m_Steps[best + 1].Y == m_Steps[best].Y)
	{
		m_Steps[best].Width += m_Steps[best + 1].Width;
		m_Steps.erase(m_Steps.begin() + best + 1);
	}
	if (best > 0 && m_Steps[best - 1].Y == m_Steps[best].Y)
	{
		m_Steps[best - 1].Width += m_Steps[best].Width;
		m_Steps.erase(m_Steps.begin() + best);
	}
	return true;
}

namespace {

// A rectangle of the list at the size it is tried at
struct CSizedRect
{
	uint Index;
	uint Width, Height;
};

// An order the rectangles are placed in: the tallest first, of these the widest, of these by priority
struct CTallestFirst
{
	bool operator()(const CSizedRect &a, const CSizedRect &b) const
	{
		if (a.Height != b.Height) return a.Height > b.Height;
		if (a.Width != b.Width) return a.Width > b.Width;
		return a.Index < b.Index;
	}
};

// The other order: the widest first, of these the tallest, of these by priority
struct CWidestFirst
{
	bool operator()(const CSizedRect &a, const CSizedRect &b) const
	{
		if (a.Width != b.Width) return a.Width > b.Width;
		if (a.Height != b.Height) return a.Height > b.Height;
		return a.Index < b.Index;
	}
};

// Places the rectangles at the sizes given in the order given, each in the first bin that has room for it.
// Returns false when one has no room; what is in places means nothing then.
bool packInOrder(const std::vector<CSizedRect> &sized, uint binWidth, uint binHeight, sint maxBins,
    std::vector<CSkylinePacker> &bins, std::vector<CPackRect> &places, uint &numBins)
{
	numBins = 0;
	for (uint i = 0; i < sized.size(); ++i)
	{
		CPackRect &place = places[sized[i].Index];
		place.Width = sized[i].Width;
		place.Height = sized[i].Height;
		uint bin = 0;
		while (bin < numBins && !bins[bin].place(place.Width, place.Height, place.X, place.Y))
			++bin;
		if (bin == numBins)
		{
			if (maxBins >= 0 && numBins >= (uint)maxBins)
				return false;
			if (bins.size() <= bin)
				bins.resize(bin + 1);
			bins[bin].reset(binWidth, binHeight);
			++numBins;
			// what has no room in an empty bin has no room at all
			if (!bins[bin].place(place.Width, place.Height, place.X, place.Y))
				return false;
		}
		place.Bin = bin;
		place.Placed = true;
	}
	return true;
}

// Places the rectangles at the sizes given: the tallest first, and where that has no room, the widest first.
// Returns false when neither has room for all; what is in places means nothing then.
bool packSized(std::vector<CSizedRect> sized, uint binWidth, uint binHeight, sint maxBins,
    std::vector<CSkylinePacker> &bins, std::vector<CPackRect> &places, uint &numBins)
{
	std::sort(sized.begin(), sized.end(), CTallestFirst());
	if (packInOrder(sized, binWidth, binHeight, maxBins, bins, places, numBins))
		return true;
	std::sort(sized.begin(), sized.end(), CWidestFirst());
	return packInOrder(sized, binWidth, binHeight, maxBins, bins, places, numBins);
}

} // namespace

// ***************************************************************************
uint packRectsByPriority(std::vector<CPackRect> &rects, uint binWidth, uint binHeight, sint maxBins, uint minSize)
{
	const uint count = (uint)rects.size();
	std::vector<CSizedRect> sized(count);
	for (uint i = 0; i < count; ++i)
	{
		sized[i].Index = i;
		sized[i].Width = std::min(rects[i].Width, binWidth);
		sized[i].Height = std::min(rects[i].Height, binHeight);
	}

	std::vector<CSkylinePacker> bins;
	std::vector<CPackRect> attempt(count);
	uint numBins = 0;
	if (packSized(sized, binWidth, binHeight, maxBins, bins, attempt, numBins))
	{
		rects.swap(attempt);
		return numBins;
	}

	// The bins are full. By priority, each rectangle gets the largest size at which it has room
	// together with the ones before it; one that has no room at its smallest size is left out.
	std::vector<CPackRect> placed(count);
	std::vector<CSizedRect> kept;
	kept.reserve(count);
	numBins = 0;
	for (uint i = 0; i < count; ++i)
	{
		CSizedRect rect = sized[i];
		const uint minWidth = std::min(rect.Width, minSize);
		const uint minHeight = std::min(rect.Height, minSize);
		for (;;)
		{
			kept.push_back(rect);
			attempt.assign(count, CPackRect());
			uint attemptBins;
			if (packSized(kept, binWidth, binHeight, maxBins, bins, attempt, attemptBins))
			{
				placed.swap(attempt);
				numBins = attemptBins;
				break;
			}
			kept.pop_back();
			if (rect.Width <= minWidth && rect.Height <= minHeight)
				break;
			rect.Width = std::max(minWidth, rect.Width / 2);
			rect.Height = std::max(minHeight, rect.Height / 2);
		}
	}
	for (uint i = 0; i < count; ++i)
	{
		if (!placed[i].Placed)
		{
			// the size it had no room at
			placed[i].Width = std::min(sized[i].Width, minSize);
			placed[i].Height = std::min(sized[i].Height, minSize);
		}
	}
	rects.swap(placed);
	return numBins;
}

} // NL3D
