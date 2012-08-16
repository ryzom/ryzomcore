
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-doc-meta-data.h>
#include <gsf/gsf-msole-utils.h>
#include <glib/gi18n.h>
#include <string.h>
#include <cstdio>

#include "storage_stream.h"

static const char *filename = "/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max";
//static const char *filename = "/home/kaetemi/source/minimax/GE_Acc_MikotoBaniere.max";
static const char *streamname = "DllDirectory";

inline uint8 cleanChar(uint8 c)
{
	if (c >= 32 && c <= 126) return c;
	return 46;
}

static void dumpData(PIPELINE::CStorageStream *in, const std::string &pad)
{
	sint32 size = in->getChunkSize();
	std::vector<uint8> buffer;
	buffer.resize(size + 2);
	in->serialBuffer(&buffer[0], size);
	switch (in->getChunkId())
	{
	case 8249: // some dll description
	case 8247: // some dll name
		for (std::vector<uint8>::size_type i = 0; i < buffer.size() / 2; ++i)
			buffer[i] = cleanChar(buffer[i * 2]);
		buffer[(buffer.size()  / 2) - 1] = 0;
		printf("%sUTF16: %s\n", pad.c_str(), &buffer[0]);
		break;
	default:
		for (std::vector<uint8>::size_type i = 0; i < buffer.size(); ++i)
			buffer[i] = cleanChar(buffer[i]);
		buffer[buffer.size() - 2] = 0;
		printf("%sRAW: %s\n", pad.c_str(), &buffer[0]);
		break;
	}
}

static void dumpContainer(PIPELINE::CStorageStream *in, const std::string &pad)
{
	while (in->enterChunk())
	{
		printf("%sCHUNK ID: %i, SIZE: %i, CONTAINER: %i\n", pad.c_str(), (sint32)in->getChunkId(), (sint32)in->getChunkSize(), (sint32)in->isChunkContainer());
		if (in->isChunkContainer())
		{
			dumpContainer(in, pad + "\t");
		}
		else
		{
			dumpData(in, pad + "\t");
		}
		sint32 skipped = in->leaveChunk();
		printf("%sSKIPPED: %i\n", pad.c_str(), skipped);
	}
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char **argv)
{
	printf("Pipeline Max Dump (Temporary Tool)\n");

	char const *me = (argv[0] ? argv[0] : "pipeline_max_dump");
	g_set_prgname(me);
	gsf_init();

	GsfInfile *infile;
	GError *error = NULL;
	GsfInput *src;
	char *display_name;

	src = gsf_input_stdio_new(filename, &error);

	if (error)
	{
		display_name = g_filename_display_name(filename);
		g_printerr (_("%s: Failed to open %s: %s\n"),
			    g_get_prgname (),
			    display_name,
			    error->message);
		g_free(display_name);
		return 1;
	}

	infile = gsf_infile_msole_new(src, NULL);

	if (!infile)
	{
		display_name = g_filename_display_name(filename);
		g_printerr (_("%s: Failed to recognize %s as an archive\n"),
				g_get_prgname (),
				display_name);
		g_free (display_name);
		return 1;
	}

	display_name = g_filename_display_name(filename);
	g_print("%s\n", display_name);
	g_free(display_name);
	g_print("%s\n", streamname);

	GsfInput *input = gsf_infile_child_by_name(infile, streamname);
	//gsf_input_dump(input, 1); // just a regular hex dump of this input stream
	PIPELINE::CStorageStream *instream = new PIPELINE::CStorageStream(input);
	dumpContainer(instream, "");
	delete instream;

	g_object_unref(input);
	g_object_unref(infile);

	return 0;
}

