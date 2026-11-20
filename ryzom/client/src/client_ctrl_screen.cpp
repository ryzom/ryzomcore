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

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/mem_stream.h"

#include "client_cfg.h"
#include "global.h"

#include <cmath>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

const uint s_DefaultMaxSize = 1024;
const uint s_MaxFrames = 120;

/// The part of the window a grab takes, and the size it is scaled to
struct CGrabArea
{
	uint X, Y, Width, Height;
	uint OutWidth, OutHeight;
	double Scale;
};

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
		sint64 x = region.get("x").getInt(0), y = region.get("y").getInt(0);
		sint64 w = region.get("width").getInt((sint64)width - x), h = region.get("height").getInt((sint64)height - y);
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
	double scale = args.get("scale").getNumber(0.0);
	if (scale <= 0.0)
	{
		uint maxSize = (uint)std::max((sint64)16, args.get("max_size").getInt(maxSizeDefault));
		uint longest = std::max(area.Width, area.Height);
		scale = longest > maxSize ? (double)maxSize / (double)longest : 1.0;
	}
	if (scale > 1.0)
		scale = 1.0;
	// what a series of pictures may take at most, whatever was asked
	if (hardMax > 0 && std::max(area.Width, area.Height) * scale > hardMax)
		scale = (double)hardMax / (double)std::max(area.Width, area.Height);
	area.Scale = scale;
	area.OutWidth = std::max(1u, (uint)floor(area.Width * scale + 0.5));
	area.OutHeight = std::max(1u, (uint)floor(area.Height * scale + 0.5));
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
		ok = bitmap.writeJPG(stream, (uint8)std::min(100u, std::max(1u, quality)));
		type = "image/jpeg";
	}
	if (!ok)
		return false;
	bytes.assign((const char *)stream.buffer(), stream.length());
	return true;
}

std::string formatOf(const CCtrlCall &call, const char *defaultFormat)
{
	std::string format = call.getArgs().get("format").getString(defaultFormat);
	return format == "png" ? "png" : "jpeg";
}

uint qualityOf(const CCtrlCall &call)
{
	return (uint)std::max((sint64)1, std::min((sint64)100, call.getArgs().get("quality").getInt(88)));
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
	uint Frames;
	uint Every;
	uint64 NextFrame;
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
	// the depth is the scene's, read where the scene is drawn: with bloom or FXAA the window's depth buffer at the
	// swap is not the scene's
	bool depth = call.getArgs().get("depth").getBool(false);
	bool withoutInterface = !call.getArgs().get("interface").getBool(true) || depth;
	// the scene alone is taken in the main loop, the only one that draws the interface over a scene
	if (withoutInterface && !requireInGame(call))
		return nullptr;
	CGrabData *data = new CGrabData();
	data->Kind = kind;
	data->WithoutInterface = withoutInterface;
	data->Depth = depth;
	if (!parseArea(call, maxSize, hardMax, data->Area))
	{
		delete data;
		return nullptr;
	}
	data->Frames = 1;
	data->Every = 1;
	data->NextFrame = 0;
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

void finishGrab(CCtrlCall &call, CGrabData &data)
{
	CJsonValue r = areaJson(data.Area);
	std::string format = formatOf(call, "jpeg");
	std::string bytes, type;
	if (!encode(data.Pictures[0], format, qualityOf(call), bytes, type))
	{
		call.fail("the picture could not be encoded");
		return;
	}
	r.set("picture", call.attach(type, bytes));
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
	uint columns = (uint)std::max((sint64)1, std::min((sint64)count, call.getArgs().get("columns").getInt((sint64)ceil(sqrt((double)count)))));
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
	if (!encode(grid, formatOf(call, "jpeg"), qualityOf(call), bytes, type))
	{
		call.fail("the picture could not be encoded");
		return;
	}
	r.set("grid", call.attach(type, bytes));
	r.set("columns", columns);
	r.set("rows", rows);
	r.set("cells", "left to right, top to bottom, in the order of the frames");
	r.set("frames", frames);
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
	uint cells = (uint)std::max((sint64)1, std::min((sint64)16, call.getArgs().get("grid").getInt(4)));
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
			cell.set("change", floor(sum / area / changes * 100.0 + 0.5) / 100.0);
			regions.push(cell);
		}
	}
	CJsonValue series = CJsonValue::array();
	for (uint i = 0; i < data.Series.size(); ++i)
		series.push(CJsonValue(floor(data.Series[i] * 100.0 + 0.5) / 100.0));
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
	r.set("mean_change", floor(total / std::max(1.0, (double)w * h) / changes * 100.0 + 0.5) / 100.0);
	r.set("units", "mean absolute change of luminance per pixel and per frame step, 0 to 255");
	r.set("regions", regions);
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
	double count = std::max((size_t)1, n);
	double mean = l / count;
	CJsonValue res = areaJson(data.Area);
	res.set("mean", CJsonValue::object().set("r", floor(r / count * 10 + 0.5) / 10).set("g", floor(g / count * 10 + 0.5) / 10).set("b", floor(b / count * 10 + 0.5) / 10));
	res.set("luminance", floor(mean * 10 + 0.5) / 10);
	res.set("luminance_deviation", floor(sqrt(std::max(0.0, l2 / count - mean * mean)) * 10 + 0.5) / 10);
	res.set("frame", data.FrameNumbers[0]);
	call.reply(res);
}

void takeFrame(CCtrlCall &call, CGrabData &data, CBitmap &frame, bool &haveFrame, uint64 frameNumber);

/// Takes the frame for the calls that want one at this point of it
void takeFrames(bool withoutInterface)
{
	if (s_Pending.empty())
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
		try
		{
			takeFrame(call, data, frame, haveFrame, frameNumber);
		}
		catch (const std::exception &e)
		{
			// the main loop is not a method's: the call fails, the frame goes on
			removePending(call);
			call.fail(toString("the grab failed: %s", e.what()));
		}
	}
}

/// One frame for one call; the call is answered when it has its frames
void takeFrame(CCtrlCall &call, CGrabData &data, CBitmap &frame, bool &haveFrame, uint64 frameNumber)
{
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
			removePending(call);
			call.fail(toString("the frame is %ux%u, not the %ux%u of the window: the window changed size during the grab", frame.getWidth(), frame.getHeight(), width, height));
			return;
		}
		CBitmap picture;
		cropScale(frame, data.Area, picture);
		data.FrameNumbers.push_back(frameNumber);
		data.Times.push_back(getCtrlTimeMs());
		if (data.Kind == CGrabData::Diff)
			addDiffFrame(data, picture);
		else
			data.Pictures.push_back(picture);
		data.NextFrame = frameNumber + data.Every;
		if (data.FrameNumbers.size() < data.Frames)
			return;
		removePending(call);
		switch (data.Kind)
		{
		case CGrabData::Grab: finishGrab(call, data); break;
		case CGrabData::Filmstrip: finishFilmstrip(call, data); break;
		case CGrabData::Diff: finishDiff(call, data); break;
		case CGrabData::Stats: finishStats(call, data); break;
		}
	}
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

/// The frame count and spacing of a series of frames
bool seriesOf(CCtrlCall &call, CGrabData &data, uint defaultFrames)
{
	data.Frames = (uint)std::max((sint64)2, std::min((sint64)s_MaxFrames, call.getArgs().get("frames").getInt(defaultFrames)));
	data.Every = (uint)std::max((sint64)1, std::min((sint64)600, call.getArgs().get("every").getInt(1)));
	return true;
}

void methodFilmstrip(CCtrlCall &call)
{
	uint frames = (uint)std::max((sint64)2, std::min((sint64)s_MaxFrames, call.getArgs().get("frames").getInt(9)));
	// the cells fit in one picture of a size a model reads
	uint columns = (uint)std::max((sint64)1, std::min((sint64)frames, call.getArgs().get("columns").getInt((sint64)ceil(sqrt((double)frames)))));
	uint rows = (frames + columns - 1) / columns;
	// the grid fits in a picture of 2048 pixels a side, whatever max_size asks: frames times a full window would not
	uint cellMax = std::max(16u, 2048u / std::max(columns, rows));
	CGrabData *data = startGrab(call, CGrabData::Filmstrip, std::min(cellMax, s_DefaultMaxSize * 3 / 2 / columns), cellMax);
	if (data != nullptr)
		seriesOf(call, *data, 9);
}

void methodDiff(CCtrlCall &call)
{
	CGrabData *data = startGrab(call, CGrabData::Diff, 320, 640);
	if (data != nullptr)
		seriesOf(call, *data, 30);
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
	endpoint.addMethod(new CCtrlFunctionMethod("screen.grab",
	    "A picture of the frame the client shows, from its back buffer: the window's place and what covers it do not matter. By default the whole client area, scaled down to 1024 pixels on its longest side; "
	    "region crops it (in the pixels of input.mouse), scale or max_size sizes it. interface false takes the frame before the interface is drawn (in the game only). depth adds the depth of the frame where the driver gives it. "
	    "The result gives the scale: image pixel / scale + region origin = the x, y of input.mouse.",
	    CCtrlSchema()
	        .object("region", "x, y, width, height in window pixels.")
	        .number("scale", "The scale of the picture, 1 for full pixels.")
	        .integer("max_size", "The longest side of the picture when no scale is given (1024 by default).")
	        .choice("format", "jpeg|png", "jpeg (the default) or png.")
	        .integer("quality", "JPEG quality, 1 to 100 (88 by default).")
	        .boolean("interface", "With the interface (the default), or the scene alone.")
	        .boolean("depth", "Also the depth of the frame, as a gray picture."),
	    true, methodGrab, nullptr, cancelGrab));
	endpoint.addMethod(new CCtrlFunctionMethod("screen.filmstrip",
	    "Several frames as one picture of thumbnails, left to right and top to bottom, each with its frame number and time: frames of them, every so many frames apart. To see an animation or a movement at a glance.",
	    CCtrlSchema()
	        .integer("frames", "How many frames (9 by default, 120 at most).")
	        .integer("every", "Frames between two thumbnails (1 by default).")
	        .integer("columns", "Thumbnails per row (about the square root of frames by default).")
	        .object("region", "x, y, width, height in window pixels.")
	        .integer("max_size", "The longest side of a thumbnail.")
	        .choice("format", "jpeg|png", "jpeg (the default) or png.")
	        .boolean("interface", "With the interface (the default), or the scene alone."),
	    true, methodFilmstrip, nullptr, cancelGrab));
	endpoint.addMethod(new CCtrlFunctionMethod("screen.diff",
	    "Whether and where something moves over a series of frames, without reading the pictures: a heat map of the summed change of every pixel over the last frame, the change per frame step, "
	    "and the change per cell of a grid over the area. 0 means still; a moving character or camera gives tens.",
	    CCtrlSchema()
	        .integer("frames", "How many frames (30 by default, 120 at most).")
	        .integer("every", "Frames between two samples (1 by default).")
	        .object("region", "x, y, width, height in window pixels.")
	        .integer("max_size", "The working size of the samples (320 by default).")
	        .integer("grid", "Cells per side of the grid of figures (4 by default).")
	        .boolean("interface", "With the interface (the default), or the scene alone."),
	    true, methodDiff, nullptr, cancelGrab));
	endpoint.addMethod(new CCtrlFunctionMethod("screen.stats",
	    "The mean colour, luminance and its deviation of a region of the next frame, at full pixels: for lighting and fading checks without a picture.",
	    CCtrlSchema().object("region", "x, y, width, height in window pixels (the whole client area by default).").boolean("interface", "With the interface (the default), or the scene alone."),
	    true, methodStats, nullptr, cancelGrab));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_screen.cpp */
