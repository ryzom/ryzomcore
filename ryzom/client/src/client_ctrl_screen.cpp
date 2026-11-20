// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
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

// The screen methods of the control endpoint: screen.grab, screen.filmstrip, screen.diff, screen.stats.
// The pictures come from the frame the client renders, read from the back buffer just before the swap
// (or before the interface is drawn, for a picture without it): neither the window's place nor what
// covers it matters. A grab is not answered in the call: the call stays open until the frames it
// needs have been shown. Image pixels are the pixels of input.mouse, scaled by the scale given back.
//
// A window that shows no frame (minimized, or a Direct3D device that is lost) gives none, and a wait
// that pumps without drawing (a far teleport, a wait for the server) shows none: a call that sees no
// frame for a few seconds ends, as does a series past its timeout_ms, with the frames it has when they
// are enough (incomplete says so) or as a failure that names the state.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/common.h"
#include "nel/misc/mem_stream.h"

#include "client_cfg.h"
#include "far_tp.h"
#include "global.h"
#include "misc.h"

#include <cmath>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

// *** defined in client_ctrl_input.cpp; the declaration belongs in client_ctrl_methods.h ***

ICtrlMethod *newMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly, CCtrlFunctionMethod::TFunction callFunction,
    CCtrlFunctionMethod::TFunction pollFunction, CCtrlFunctionMethod::TFunction cancelFunction);

namespace {

const uint s_DefaultMaxSize = 1024;
const uint s_MaxSize = 16384;
const uint s_MaxFrames = 120;
// a series spans this many frames at most: a minute at 60 frames a second
const uint s_MaxSpan = 3600;
// the calls that wait for frames at once, of all connections: each can hold pictures of many megabytes
const uint s_MaxPending = 8;
// a call that sees no frame for this long ends
const TTime s_StallMs = 5000;
const sint64 s_DefaultSeriesTimeoutMs = 30000;
const sint64 s_MaxSeriesTimeoutMs = 300000;
// whether the build writes JPEG (NeL with USE_JPEG); png otherwise
bool s_HasJpeg = false;

/// The part of the window a grab takes, and the size it is scaled to
struct CGrabArea
{
	uint X, Y, Width, Height;
	uint OutWidth, OutHeight;
	double Scale;
};

/// An integer member of the region, false with the call failed when it is there and not an integer
bool regionMember(CCtrlCall &call, const CJsonValue &region, const char *name, sint64 defaultValue, sint64 &value)
{
	const CJsonValue &v = region.get(name);
	if (v.isNull())
	{
		value = defaultValue;
		return true;
	}
	if (!v.isNumber() || !isValidDouble(v.getNumber()) || v.getNumber() != std::floor(v.getNumber()))
	{
		call.fail(toString("region.%s is an integer, in window pixels", name));
		return false;
	}
	value = v.getInt();
	return true;
}

bool parseArea(CCtrlCall &call, uint maxSizeDefault, uint hardMax, CGrabArea &area)
{
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	if (width == 0 || height == 0)
	{
		call.fail("the window has no area (minimized): there is no picture");
		return false;
	}
	const CJsonValue &args = call.getArgs();
	const CJsonValue &region = args.get("region");
	area.X = 0;
	area.Y = 0;
	area.Width = width;
	area.Height = height;
	if (region.isObject())
	{
		const CJsonValue::TMembers &members = region.members();
		for (uint i = 0; i < members.size(); ++i)
		{
			const std::string &name = members[i].first;
			if (name != "x" && name != "y" && name != "width" && name != "height")
			{
				call.fail(toString("region has x, y, width and height, not '%s'", name.c_str()));
				return false;
			}
		}
		sint64 x, y, w, h;
		if (!regionMember(call, region, "x", 0, x) || !regionMember(call, region, "y", 0, y) || !regionMember(call, region, "width", (sint64)width - x, w)
		    || !regionMember(call, region, "height", (sint64)height - y, h))
			return false;
		if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > (sint64)width || y + h > (sint64)height)
		{
			call.fail(toString("the region is outside the window, which is %ux%u", width, height));
			return false;
		}
		area.X = (uint)x;
		area.Y = (uint)y;
		area.Width = (uint)w;
		area.Height = (uint)h;
	}
	// a size a model reads, unless asked otherwise; never larger than the frame
	double scale;
	uint longest = std::max(area.Width, area.Height);
	if (!args.get("scale").isNull())
	{
		scale = args.get("scale").getNumber();
		if (!(scale > 0.0 && scale <= 1.0))
		{
			call.fail("scale is above 0 and at most 1 (1 for full pixels)");
			return false;
		}
	}
	else
	{
		sint64 maxSize = maxSizeDefault;
		if (!args.get("max_size").isNull())
		{
			maxSize = args.get("max_size").getInt();
			if (maxSize < 16 || maxSize > (sint64)s_MaxSize)
			{
				call.fail(toString("max_size is 16 to %u", s_MaxSize));
				return false;
			}
		}
		scale = longest > (uint)maxSize ? (double)maxSize / (double)longest : 1.0;
	}
	// what a series of pictures may take at most, whatever was asked: the result gives the scale
	if (hardMax > 0 && longest * scale > hardMax)
		scale = (double)hardMax / (double)longest;
	area.Scale = scale;
	area.OutWidth = std::max(1u, (uint)std::floor(area.Width * scale + 0.5));
	area.OutHeight = std::max(1u, (uint)std::floor(area.Height * scale + 0.5));
	return true;
}

/// The area of a frame, cropped and scaled, as RGBA
void cropScale(const CBitmap &frame, const CGrabArea &area, CBitmap &out)
{
	out.resize(area.Width, area.Height, CBitmap::RGBA);
	const CObjectVector<uint8> &src = frame.getPixels();
	CObjectVector<uint8> &dst = out.getPixels();
	uint frameWidth = frame.getWidth();
	for (uint y = 0; y < area.Height; ++y)
		memcpy(&dst[(size_t)y * area.Width * 4], &src[((size_t)(area.Y + y) * frameWidth + area.X) * 4], (size_t)area.Width * 4);
	if (area.OutWidth != area.Width || area.OutHeight != area.Height)
		out.resample((sint32)area.OutWidth, (sint32)area.OutHeight);
}

/// PNG or JPEG bytes of a picture; the frame's alpha is dropped
bool encode(CBitmap &bitmap, const std::string &format, uint quality, std::string &bytes, std::string &type)
{
	CMemStream stream(false);
	bool ok;
	if (format == "png")
	{
		ok = bitmap.writePNG(stream, 24);
		type = "image/png";
	}
	else
	{
		ok = bitmap.writeJPG(stream, (uint8)quality);
		type = "image/jpeg";
	}
	if (!ok)
		return false;
	bytes.assign((const char *)stream.buffer(), stream.length());
	return true;
}

/// Whether the build writes JPEG: NeL without USE_JPEG refuses it, with a warning
bool probeJpeg()
{
	CBitmap bitmap;
	bitmap.resize(1, 1, CBitmap::RGBA);
	CMemStream stream(false);
	return bitmap.writeJPG(stream, 50);
}

/// What a grab call waits for, and what it gathered
class CGrabData : public ICtrlCallData
{
public:
	enum TKind
	{
		Grab,
		Filmstrip,
		Diff,
		Stats
	};
	TKind Kind;
	bool WithoutInterface;
	CGrabArea Area;
	std::string Format;
	uint Quality;
	uint Frames;
	uint Every;
	uint64 NextFrame;
	// the last frame taken, or the start of the call; the end of a series, 0 for none
	TTime LastFrame;
	TTime Deadline;
	uint TimeoutMs;
	// why a series ends before its frames
	std::string Incomplete;
	std::vector<CBitmap> Pictures;
	std::vector<uint64> Times;
	std::vector<uint64> FrameNumbers;
	// the difference: the previous picture, the summed change per pixel, the change per frame
	CBitmap Previous;
	std::vector<float> Heat;
	std::vector<double> Series;
	bool Depth;
};

std::vector<CCtrlCall *> s_Pending;

void removePending(CCtrlCall &call)
{
	std::vector<CCtrlCall *>::iterator it = std::find(s_Pending.begin(), s_Pending.end(), &call);
	if (it != s_Pending.end())
		s_Pending.erase(it);
}

void cancelGrab(CCtrlCall &call)
{
	removePending(call);
}

CGrabData *startGrab(CCtrlCall &call, CGrabData::TKind kind, uint maxSize, uint hardMax = 0)
{
	if (s_Pending.size() >= s_MaxPending)
	{
		call.fail(toString("%u screen calls wait for frames already: wait for their answers", (uint)s_Pending.size()));
		return nullptr;
	}
	// the depth is the scene's, read where the scene is drawn: with bloom or FXAA the window's depth buffer at the
	// swap is not the scene's
	bool depth = call.getArgs().get("depth").getBool(false);
	bool withoutInterface = !call.getArgs().get("interface").getBool(true) || depth;
	// the scene alone is taken in the main loop, the only one that draws the interface over a scene
	if (withoutInterface && !requireInGame(call))
		return nullptr;
	std::string format = call.getArgs().get("format").getString(s_HasJpeg ? "jpeg" : "png");
	if (format == "jpeg" && !s_HasJpeg)
	{
		call.fail("this build writes no JPEG (NeL without USE_JPEG): format png");
		return nullptr;
	}
	sint64 quality = call.getArgs().get("quality").getInt(88);
	if (quality < 1 || quality > 100)
	{
		call.fail("quality is 1 to 100");
		return nullptr;
	}
	CGrabData *data = new CGrabData();
	data->Kind = kind;
	data->WithoutInterface = withoutInterface;
	data->Depth = depth;
	data->Format = format;
	data->Quality = (uint)quality;
	if (!parseArea(call, maxSize, hardMax, data->Area))
	{
		delete data;
		return nullptr;
	}
	data->Frames = 1;
	data->Every = 1;
	data->NextFrame = 0;
	data->LastFrame = CTime::getLocalTime();
	data->Deadline = 0;
	data->TimeoutMs = 0;
	call.setData(data);
	s_Pending.push_back(&call);
	return data;
}

CJsonValue areaJson(const CGrabArea &area)
{
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	CJsonValue r = CJsonValue::object();
	r.set("window", CJsonValue::object().set("width", width).set("height", height));
	r.set("region", CJsonValue::object().set("x", area.X).set("y", area.Y).set("width", area.Width).set("height", area.Height));
	r.set("scale", area.Scale);
	r.set("image", CJsonValue::object().set("width", area.OutWidth).set("height", area.OutHeight));
	r.set("coordinates", "image pixel / scale + region origin = the x, y of input.mouse");
	return r;
}

/// A series that ended before its frames says why
void setIncomplete(CJsonValue &r, const CGrabData &data)
{
	if (data.Incomplete.empty())
		return;
	r.set("incomplete", true);
	r.set("reason", data.Incomplete);
	r.set("frames_asked", data.Frames);
}

void finishGrab(CCtrlCall &call, CGrabData &data)
{
	CJsonValue r = areaJson(data.Area);
	std::string bytes, type;
	if (!encode(data.Pictures[0], data.Format, data.Quality, bytes, type))
	{
		call.fail("the picture could not be encoded");
		return;
	}
	r.set("picture", call.attach(type, bytes));
	r.set("format", data.Format);
	r.set("frame", data.FrameNumbers[0]);
	r.set("time", data.Times[0]);
	r.set("interface", !data.WithoutInterface);
	// Direct3D reads the back buffer, while bloom and FXAA draw the scene in a target of their own until the swap
	if (data.WithoutInterface && ClientCfg.Driver3D == CClientConfig::Direct3D && (ClientCfg.Bloom || ClientCfg.FXAA))
		r.set("warning", "with Direct3D and bloom or FXAA, the scene alone is read from the back buffer, which may hold the previous frame");
	if (data.Depth)
	{
		// the depth of the same frame, near white and far black, where the driver gives it
		std::vector<float> z;
		Driver->getZBuffer(z);
		uint32 width = 0, height = 0;
		Driver->getWindowSize(width, height);
		if (z.size() == (size_t)width * height)
		{
			CBitmap depth;
			depth.resize(width, height, CBitmap::RGBA);
			CObjectVector<uint8> &p = depth.getPixels();
			float zMin = 1.f, zMax = 0.f;
			for (size_t i = 0; i < z.size(); ++i)
			{
				if (z[i] < 1.f)
				{
					zMin = std::min(zMin, z[i]);
					zMax = std::max(zMax, z[i]);
				}
			}
			float range = zMax > zMin ? zMax - zMin : 1.f;
			for (size_t i = 0; i < z.size(); ++i)
			{
				uint8 v = z[i] >= 1.f ? 0 : (uint8)(255.f - 230.f * (z[i] - zMin) / range);
				p[i * 4] = p[i * 4 + 1] = p[i * 4 + 2] = v;
				p[i * 4 + 3] = 255;
			}
			CBitmap out;
			cropScale(depth, data.Area, out);
			if (encode(out, "png", 0, bytes, type))
				r.set("depth", call.attach(type, bytes));
		}
		else
		{
			r.set("depth", "the driver gave no depth for this frame");
		}
	}
	call.reply(r);
}

void finishFilmstrip(CCtrlCall &call, CGrabData &data)
{
	uint count = (uint)data.Pictures.size();
	// the columns asked for, fewer when the series ended early
	uint columns = (uint)std::max((sint64)1, std::min((sint64)count, call.getArgs().get("columns").getInt((sint64)std::ceil(std::sqrt((double)data.Frames)))));
	uint rows = (count + columns - 1) / columns;
	uint w = data.Area.OutWidth, h = data.Area.OutHeight;
	const uint gap = 2;
	CBitmap grid;
	grid.resize(columns * w + (columns - 1) * gap, rows * h + (rows - 1) * gap, CBitmap::RGBA);
	CObjectVector<uint8> &g = grid.getPixels();
	for (size_t i = 0; i < g.size(); i += 4)
	{
		g[i] = g[i + 1] = g[i + 2] = 32;
		g[i + 3] = 255;
	}
	CJsonValue frames = CJsonValue::array();
	for (uint i = 0; i < count; ++i)
	{
		grid.blit(&data.Pictures[i], (sint32)((i % columns) * (w + gap)), (sint32)((i / columns) * (h + gap)));
		frames.push(CJsonValue::object().set("cell", i).set("frame", data.FrameNumbers[i]).set("time", data.Times[i]));
	}
	CJsonValue r = areaJson(data.Area);
	std::string bytes, type;
	if (!encode(grid, data.Format, data.Quality, bytes, type))
	{
		call.fail("the picture could not be encoded");
		return;
	}
	r.set("grid", call.attach(type, bytes));
	r.set("format", data.Format);
	r.set("columns", columns);
	r.set("rows", rows);
	r.set("cells", "left to right, top to bottom, in the order of the frames");
	r.set("frames", frames);
	setIncomplete(r, data);
	call.reply(r);
}

/// Luminance of an RGBA pixel, 0..255
inline float luminance(const uint8 *p)
{
	return 0.2126f * p[0] + 0.7152f * p[1] + 0.0722f * p[2];
}

void addDiffFrame(CGrabData &data, const CBitmap &picture)
{
	uint w = data.Area.OutWidth, h = data.Area.OutHeight;
	if (data.Heat.empty())
		data.Heat.resize((size_t)w * h, 0.f);
	if (data.Previous.getWidth() == w && data.Previous.getHeight() == h)
	{
		const CObjectVector<uint8> &a = data.Previous.getPixels();
		const CObjectVector<uint8> &b = picture.getPixels();
		double sum = 0;
		for (size_t i = 0; i < (size_t)w * h; ++i)
		{
			float d = fabsf(luminance(&a[i * 4]) - luminance(&b[i * 4]));
			data.Heat[i] += d;
			sum += d;
		}
		data.Series.push_back(sum / ((double)w * h));
	}
	data.Previous = picture;
}

void finishDiff(CCtrlCall &call, CGrabData &data)
{
	uint w = data.Area.OutWidth, h = data.Area.OutHeight;
	float peak = 0.f;
	double total = 0;
	for (size_t i = 0; i < data.Heat.size(); ++i)
	{
		peak = std::max(peak, data.Heat[i]);
		total += data.Heat[i];
	}
	// the heat map: no change black, the most change white through red and yellow, over the last frame in gray
	CBitmap heat;
	heat.resize(w, h, CBitmap::RGBA);
	CObjectVector<uint8> &p = heat.getPixels();
	const CObjectVector<uint8> &last = data.Previous.getPixels();
	for (size_t i = 0; i < (size_t)w * h; ++i)
	{
		float t = peak > 0.f ? data.Heat[i] / peak : 0.f;
		float gray = luminance(&last[i * 4]) * 0.25f;
		float r = std::min(1.f, t * 3.f), g = std::min(1.f, std::max(0.f, t * 3.f - 1.f)), b = std::max(0.f, t * 3.f - 2.f);
		p[i * 4] = (uint8)std::min(255.f, gray + r * 255.f);
		p[i * 4 + 1] = (uint8)std::min(255.f, gray + g * 255.f);
		p[i * 4 + 2] = (uint8)std::min(255.f, gray + b * 255.f);
		p[i * 4 + 3] = 255;
	}
	// the change per cell of a grid over the area, in the area's pixels
	uint cells = (uint)call.getArgs().get("grid").getInt(4);
	CJsonValue regions = CJsonValue::array();
	uint changes = data.Series.empty() ? 1 : (uint)data.Series.size();
	for (uint cy = 0; cy < cells; ++cy)
	{
		for (uint cx = 0; cx < cells; ++cx)
		{
			uint x0 = cx * w / cells, x1 = (cx + 1) * w / cells, y0 = cy * h / cells, y1 = (cy + 1) * h / cells;
			double sum = 0;
			for (uint y = y0; y < y1; ++y)
				for (uint x = x0; x < x1; ++x)
					sum += data.Heat[(size_t)y * w + x];
			double area = std::max(1.0, (double)(x1 - x0) * (y1 - y0));
			CJsonValue cell = CJsonValue::object();
			cell.set("x", data.Area.X + (uint)(x0 / data.Area.Scale));
			cell.set("y", data.Area.Y + (uint)(y0 / data.Area.Scale));
			cell.set("width", (uint)((x1 - x0) / data.Area.Scale));
			cell.set("height", (uint)((y1 - y0) / data.Area.Scale));
			cell.set("change", std::floor(sum / area / changes * 100.0 + 0.5) / 100.0);
			regions.push(cell);
		}
	}
	CJsonValue series = CJsonValue::array();
	for (uint i = 0; i < data.Series.size(); ++i)
		series.push(CJsonValue(std::floor(data.Series[i] * 100.0 + 0.5) / 100.0));
	CJsonValue r = areaJson(data.Area);
	std::string bytes, type;
	if (!encode(heat, "png", 0, bytes, type))
	{
		call.fail("the picture could not be encoded");
		return;
	}
	r.set("heat_map", call.attach(type, bytes));
	r.set("frames", (uint32)data.FrameNumbers.size());
	r.set("first_frame", data.FrameNumbers.front());
	r.set("last_frame", data.FrameNumbers.back());
	r.set("duration_ms", (sint64)(data.Times.back() - data.Times.front()));
	r.set("change_per_frame", series);
	r.set("mean_change", std::floor(total / std::max(1.0, (double)w * h) / changes * 100.0 + 0.5) / 100.0);
	r.set("units", "mean absolute change of luminance per pixel and per frame step, 0 to 255");
	r.set("regions", regions);
	setIncomplete(r, data);
	call.reply(r);
}

void finishStats(CCtrlCall &call, CGrabData &data)
{
	const CBitmap &picture = data.Pictures[0];
	const CObjectVector<uint8> &p = picture.getPixels();
	size_t n = (size_t)picture.getWidth() * picture.getHeight();
	double r = 0, g = 0, b = 0, l = 0, l2 = 0;
	for (size_t i = 0; i < n; ++i)
	{
		r += p[i * 4];
		g += p[i * 4 + 1];
		b += p[i * 4 + 2];
		float lum = luminance(&p[i * 4]);
		l += lum;
		l2 += (double)lum * lum;
	}
	double count = (double)std::max((size_t)1, n);
	double mean = l / count;
	CJsonValue res = areaJson(data.Area);
	res.set("mean", CJsonValue::object().set("r", std::floor(r / count * 10 + 0.5) / 10).set("g", std::floor(g / count * 10 + 0.5) / 10).set("b", std::floor(b / count * 10 + 0.5) / 10));
	res.set("luminance", std::floor(mean * 10 + 0.5) / 10);
	res.set("luminance_deviation", std::floor(std::sqrt(std::max(0.0, l2 / count - mean * mean)) * 10 + 0.5) / 10);
	res.set("frame", data.FrameNumbers[0]);
	call.reply(res);
}

void finishCall(CCtrlCall &call, CGrabData &data)
{
	switch (data.Kind)
	{
	case CGrabData::Grab: finishGrab(call, data); break;
	case CGrabData::Filmstrip: finishFilmstrip(call, data); break;
	case CGrabData::Diff: finishDiff(call, data); break;
	case CGrabData::Stats: finishStats(call, data); break;
	}
}

/// What the client is in, for a call whose frames do not come
std::string stateText()
{
	std::string text = "the client is in the phase " + getPhase();
	if (isWindowMinimizedOrWithoutArea())
		text += ", with its window minimized";
	else if (Driver->isLost())
		text += ", with its Direct3D device lost";
	if (FarTP.isFarTPInProgress())
		text += ", in a far teleport";
	if (!isInterfaceAvailable())
		text += ", in a loading or a wait that pumps without the interface";
	return text;
}

/// Ends a call that cannot have all its frames: with those it has, when they make an answer (a filmstrip one, a
/// difference two), and why; or as a failure
void endEarly(CCtrlCall &call, CGrabData &data, const std::string &reason)
{
	removePending(call);
	bool enough = (data.Kind == CGrabData::Filmstrip && !data.FrameNumbers.empty()) || (data.Kind == CGrabData::Diff && data.FrameNumbers.size() >= 2);
	if (enough)
	{
		data.Incomplete = reason;
		finishCall(call, data);
		return;
	}
	call.fail(reason, CJsonValue::object().set("phase", getPhase()).set("frames", (uint32)data.FrameNumbers.size()));
}

void pollGrab(CCtrlCall &call)
{
	CGrabData &data = *static_cast<CGrabData *>(call.getData());
	TTime now = CTime::getLocalTime();
	if (now - data.LastFrame >= s_StallMs)
		endEarly(call, data, toString("no frame was shown for %u s: %s", (uint)(s_StallMs / 1000), stateText().c_str()));
	else if (data.Deadline != 0 && now >= data.Deadline)
		endEarly(call, data, toString("the series took longer than timeout_ms (%u ms)", data.TimeoutMs));
}

void takeFrame(CCtrlCall &call, CGrabData &data, CBitmap &frame, bool &haveFrame, uint64 frameNumber);

/// Takes the frame for the calls that want one at this point of it
void takeFrames(bool withoutInterface)
{
	if (s_Pending.empty())
		return;
	// a window that shows no frame gives none to read: the calls wait, and pollGrab ends them if it lasts
	if (isWindowMinimizedOrWithoutArea() || Driver->isLost())
		return;
	CBitmap frame;
	bool haveFrame = false;
	std::vector<CCtrlCall *> pending = s_Pending;
	for (uint i = 0; i < pending.size(); ++i)
	{
		CCtrlCall &call = *pending[i];
		CGrabData &data = *static_cast<CGrabData *>(call.getData());
		if (data.WithoutInterface != withoutInterface)
			continue;
		// before the interface, the frame being drawn is the one after the last shown
		uint64 frameNumber = (getEndpoint() != nullptr ? getEndpoint()->getFrame() : 0) + (withoutInterface ? 1 : 0);
		if (frameNumber < data.NextFrame)
			continue;
		takeFrame(call, data, frame, haveFrame, frameNumber);
	}
}

/// One frame for one call; the call is answered when it has its frames
void takeFrame(CCtrlCall &call, CGrabData &data, CBitmap &frame, bool &haveFrame, uint64 frameNumber)
{
	if (!haveFrame)
	{
		Driver->getBuffer(frame);
		haveFrame = true;
	}
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	if (frame.getWidth() != width || frame.getHeight() != height || frame.getWidth() < data.Area.X + data.Area.Width || frame.getHeight() < data.Area.Y + data.Area.Height)
	{
		endEarly(call, data, toString("the frame is %ux%u, not the %ux%u of the window it was asked of: the window changed size", frame.getWidth(), frame.getHeight(), width, height));
		return;
	}
	CBitmap picture;
	cropScale(frame, data.Area, picture);
	data.FrameNumbers.push_back(frameNumber);
	data.Times.push_back(getCtrlTimeMs());
	data.LastFrame = CTime::getLocalTime();
	if (data.Kind == CGrabData::Diff)
		addDiffFrame(data, picture);
	else
		data.Pictures.push_back(picture);
	data.NextFrame = frameNumber + data.Every;
	if (data.FrameNumbers.size() < data.Frames)
		return;
	removePending(call);
	finishCall(call, data);
}

void methodGrab(CCtrlCall &call)
{
	startGrab(call, CGrabData::Grab, s_DefaultMaxSize);
}

void methodStats(CCtrlCall &call)
{
	CGrabData *data = startGrab(call, CGrabData::Stats, 100000);
	if (data != nullptr)
	{
		// the statistics of the full pixels of the region
		data->Area.Scale = 1.0;
		data->Area.OutWidth = data->Area.Width;
		data->Area.OutHeight = data->Area.Height;
	}
}

/// The frame count, spacing and time bound of a series of frames; false with the call failed when they are out of range
bool parseSeries(CCtrlCall &call, uint defaultFrames, uint &frames, uint &every, uint &timeoutMs)
{
	const CJsonValue &args = call.getArgs();
	sint64 f = args.get("frames").getInt(defaultFrames);
	sint64 e = args.get("every").getInt(1);
	sint64 t = args.get("timeout_ms").getInt(s_DefaultSeriesTimeoutMs);
	if (f < 2 || f > (sint64)s_MaxFrames)
	{
		call.fail(toString("frames is 2 to %u", s_MaxFrames));
		return false;
	}
	if (e < 1 || e > 600)
	{
		call.fail("every is 1 to 600");
		return false;
	}
	if ((f - 1) * e + 1 > (sint64)s_MaxSpan)
	{
		call.fail(toString("frames and every span %u frames; a series spans at most %u (about a minute at 60 frames a second)", (uint)((f - 1) * e + 1), s_MaxSpan));
		return false;
	}
	if (t < 1 || t > s_MaxSeriesTimeoutMs)
	{
		call.fail(toString("timeout_ms is 1 to %u", (uint)s_MaxSeriesTimeoutMs));
		return false;
	}
	frames = (uint)f;
	every = (uint)e;
	timeoutMs = (uint)t;
	return true;
}

void startSeries(CGrabData &data, uint frames, uint every, uint timeoutMs)
{
	data.Frames = frames;
	data.Every = every;
	data.TimeoutMs = timeoutMs;
	data.Deadline = CTime::getLocalTime() + timeoutMs;
}

void methodFilmstrip(CCtrlCall &call)
{
	uint frames, every, timeoutMs;
	if (!parseSeries(call, 9, frames, every, timeoutMs))
		return;
	// the cells fit in one picture of a size a model reads
	sint64 columns = call.getArgs().get("columns").getInt((sint64)std::ceil(std::sqrt((double)frames)));
	if (columns < 1 || columns > (sint64)frames)
	{
		call.fail(toString("columns is 1 to frames (%u)", frames));
		return;
	}
	uint rows = (frames + (uint)columns - 1) / (uint)columns;
	// the grid fits in a picture of 2048 pixels a side, whatever max_size asks: frames times a full window would not
	uint cellMax = std::max(16u, 2048u / std::max((uint)columns, rows));
	CGrabData *data = startGrab(call, CGrabData::Filmstrip, std::min(cellMax, s_DefaultMaxSize * 3 / 2 / (uint)columns), cellMax);
	if (data != nullptr)
		startSeries(*data, frames, every, timeoutMs);
}

void methodDiff(CCtrlCall &call)
{
	uint frames, every, timeoutMs;
	if (!parseSeries(call, 30, frames, every, timeoutMs))
		return;
	sint64 grid = call.getArgs().get("grid").getInt(4);
	if (grid < 1 || grid > 16)
	{
		call.fail("grid is 1 to 16");
		return;
	}
	CGrabData *data = startGrab(call, CGrabData::Diff, 320, 640);
	if (data != nullptr)
		startSeries(*data, frames, every, timeoutMs);
}

} // anonymous namespace

void screenFrameShown()
{
	takeFrames(false);
	// a picture of the scene alone comes only in the game: a call that waits for one fails once the game is left
	for (uint i = 0; i < s_Pending.size();)
	{
		CCtrlCall &call = *s_Pending[i];
		if (static_cast<CGrabData *>(call.getData())->WithoutInterface && getPhase() != "in_game")
		{
			s_Pending.erase(s_Pending.begin() + i);
			call.fail("the game was left before the scene could be taken alone");
		}
		else
		{
			++i;
		}
	}
}

void screenSceneRendered()
{
	takeFrames(true);
}

void releaseScreenMethods()
{
	s_Pending.clear();
}

void registerScreenMethods(CCtrlEndpoint &endpoint)
{
	s_HasJpeg = probeJpeg();
	const char *formats = "jpeg (the default) or png; png is the default in a build without a JPEG writer (the result says which).";
	endpoint.addMethod(newMethod("screen.grab",
	    "A picture of the frame the client shows, from its back buffer: the window's place and what covers it do not matter. By default the whole client area, scaled down to 1024 pixels on its longest side; "
	    "region crops it (in the pixels of input.mouse), scale or max_size sizes it. interface false takes the frame before the interface is drawn (in the game only). depth adds the depth of the frame where the driver gives it. "
	    "The result gives the scale: image pixel / scale + region origin = the x, y of input.mouse. It fails when no frame is shown for 5 s (a minimized window, a wait that does not draw).",
	    CCtrlSchema()
	        .object("region", "x, y, width, height in window pixels (integers).")
	        .number("scale", "The scale of the picture, above 0 and at most 1 (1 for full pixels).")
	        .integer("max_size", "The longest side of the picture when no scale is given, 16 to 16384 (1024 by default).")
	        .choice("format", "jpeg|png", formats)
	        .integer("quality", "JPEG quality, 1 to 100 (88 by default).")
	        .boolean("interface", "With the interface (the default), or the scene alone.")
	        .boolean("depth", "Also the depth of the frame, as a gray picture."),
	    true, methodGrab, pollGrab, cancelGrab));
	endpoint.addMethod(newMethod("screen.filmstrip",
	    "Several frames as one picture of thumbnails, left to right and top to bottom, each with its frame number and time: frames of them, every so many frames apart, spanning at most "
	    "3600 frames. To see an animation or a movement at a glance. When the frames stop coming (5 s without one) or timeout_ms passes, it answers with the thumbnails it has and "
	    "incomplete with the reason.",
	    CCtrlSchema()
	        .integer("frames", "How many frames, 2 to 120 (9 by default).")
	        .integer("every", "Frames between two thumbnails, 1 to 600 (1 by default).")
	        .integer("columns", "Thumbnails per row, 1 to frames (about the square root of frames by default).")
	        .integer("timeout_ms", "The longest the series may take, 1 to 300000 ms (30000 by default).")
	        .object("region", "x, y, width, height in window pixels (integers).")
	        .integer("max_size", "The longest side of a thumbnail, 16 to 16384.")
	        .choice("format", "jpeg|png", formats)
	        .integer("quality", "JPEG quality, 1 to 100 (88 by default).")
	        .boolean("interface", "With the interface (the default), or the scene alone."),
	    true, methodFilmstrip, pollGrab, cancelGrab));
	endpoint.addMethod(newMethod("screen.diff",
	    "Whether and where something moves over a series of frames, without reading the pictures: a heat map of the summed change of every pixel over the last frame, the change per frame step, "
	    "and the change per cell of a grid over the area. 0 means still; a moving character or camera gives tens. When the frames stop coming (5 s without one) or timeout_ms passes, it "
	    "answers with the frames it has (two at least) and incomplete with the reason.",
	    CCtrlSchema()
	        .integer("frames", "How many frames, 2 to 120 (30 by default).")
	        .integer("every", "Frames between two samples, 1 to 600 (1 by default).")
	        .integer("timeout_ms", "The longest the series may take, 1 to 300000 ms (30000 by default).")
	        .object("region", "x, y, width, height in window pixels (integers).")
	        .integer("max_size", "The working size of the samples, 16 to 16384 (320 by default).")
	        .integer("grid", "Cells per side of the grid of figures, 1 to 16 (4 by default).")
	        .boolean("interface", "With the interface (the default), or the scene alone."),
	    true, methodDiff, pollGrab, cancelGrab));
	endpoint.addMethod(newMethod("screen.stats",
	    "The mean colour, luminance and its deviation of a region of the next frame, at full pixels: for lighting and fading checks without a picture.",
	    CCtrlSchema().object("region", "x, y, width, height in window pixels, integers (the whole client area by default).").boolean("interface", "With the interface (the default), or the scene alone."),
	    true, methodStats, pollGrab, cancelGrab));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_screen.cpp */
