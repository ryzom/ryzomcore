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

// A program with a control endpoint, as a NeL application has it: nl_sample_ctrl_endpoint [<name>]
// It listens under the name (sample by default) until its quit method is called, with a main loop
// that serves the calls once per frame, a state, events, and three methods: one answered at once,
// one that answers a few frames later, and one that attaches an image. Drive it with nel_mcp.

#include "nel/misc/types_nl.h"

#include "nel/misc/bitmap.h"
#include "nel/misc/command.h"
#include "nel/misc/ctrl_endpoint.h"
#include "nel/misc/debug.h"
#include "nel/misc/json.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/time_nl.h"

using namespace NLMISC;

namespace {

bool s_Quit = false;
uint64 s_Frame = 0;
sint s_Counter = 0;

void methodAdd(CCtrlCall &call)
{
	s_Counter += (sint)call.getArgs().get("value").getInt(1);
	nlinfo("counter is %d", s_Counter);
	call.reply(CJsonValue::object().set("counter", s_Counter));
}

/// Answers after the number of frames asked for, as a method that waits for something in the program
class CFramesData : public ICtrlCallData
{
public:
	uint64 Until;
};

void methodFrames(CCtrlCall &call)
{
	CFramesData *data = new CFramesData();
	data->Until = s_Frame + (uint64)call.getArgs().get("frames").getInt(10);
	call.setData(data);
}

void pollFrames(CCtrlCall &call)
{
	CFramesData *data = static_cast<CFramesData *>(call.getData());
	if (s_Frame >= data->Until)
		call.reply(CJsonValue::object().set("frame", s_Frame).set("elapsed_ms", (sint64)call.getElapsed()));
}

void methodImage(CCtrlCall &call)
{
	// a gradient, as a grab would give one
	uint size = (uint)call.getArgs().get("size").getInt(64);
	CBitmap bitmap;
	bitmap.resize(size, size, CBitmap::RGBA);
	CObjectVector<uint8> &pixels = bitmap.getPixels();
	for (uint y = 0; y < size; ++y)
		for (uint x = 0; x < size; ++x)
		{
			uint8 *p = &pixels[(y * size + x) * 4];
			p[0] = (uint8)(x * 255 / size);
			p[1] = (uint8)(y * 255 / size);
			p[2] = 128;
			p[3] = 255;
		}
	CMemStream stream(false);
	bitmap.writePNG(stream, 24);
	std::string bytes((const char *)stream.buffer(), stream.length());
	call.reply(CJsonValue::object().set("image", call.attach("image/png", bytes)).set("size", size));
}

void methodQuit(CCtrlCall &call)
{
	s_Quit = true;
	call.reply(CJsonValue::object().set("quitting", true));
}

} // anonymous namespace

NLMISC_COMMAND(sampleSay, "Shows a text", "<text>")
{
	if (args.size() != 1)
		return false;
	log.displayNL("said: %s", args[0].c_str());
	return true;
}

int main(int argc, char **argv)
{
	CApplicationContext context;
	createDebug();

	std::string name = argc > 1 ? argv[1] : "sample";
	CCtrlEndpoint endpoint;
	CJsonValue identity = CJsonValue::object();
	identity.set("kind", "nl_sample_ctrl_endpoint");
	std::string error;
	if (!endpoint.start(name, identity, error))
	{
		nlwarning("the endpoint did not start: %s", error.c_str());
		return 1;
	}
	endpoint.addMethod(new CCtrlFunctionMethod("sample.add", "Adds a value to the counter.", CCtrlSchema().integer("value", "What to add (1 by default)."), false, methodAdd));
	endpoint.addMethod(new CCtrlFunctionMethod("sample.frames", "Answers after a number of frames.", CCtrlSchema().integer("frames", "How many (10 by default)."), true, methodFrames, pollFrames));
	endpoint.addMethod(new CCtrlFunctionMethod("sample.image", "A gradient as a PNG image.", CCtrlSchema().integer("size", "Its width and height (64 by default)."), true, methodImage));
	endpoint.addMethod(new CCtrlFunctionMethod("sample.quit", "Quits the program.", CCtrlSchema(), false, methodQuit));

	// the main loop: a frame every 16 ms, the calls served once per frame
	TTime start = CTime::getLocalTime();
	while (!s_Quit)
	{
		++s_Frame;
		endpoint.setFrame(s_Frame);
		endpoint.setState((CTime::getLocalTime() - start) < 2000 ? "starting" : "running", CJsonValue::object().set("counter", s_Counter));
		endpoint.update();
		if (s_Frame % 300 == 0)
			endpoint.addEvent("tick", CJsonValue::object().set("frame", s_Frame));
		nlSleep(16);
	}
	endpoint.stop("the quit method was called");
	return 0;
}
