
#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-doc-meta-data.h>
#include <gsf/gsf-msole-utils.h>
#include <glib/gi18n.h>
#include <string.h>
#include <cstdio>

#include <vector>
#include <utility>

#include "storage_stream.h"

//static const char *filename = "/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max";
//static const char *filename = "/home/kaetemi/source/minimax/GE_Acc_MikotoBaniere.max";
static const char *filename = "/home/kaetemi/3dsMax/scenes/test2008.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/teapot_test_scene.max";
static const char *streamname = "Scene";

inline uint8 cleanChar(uint8 c)
{
	if (c >= 32 && c <= 126) return c;
	return 46;
}

namespace PIPELINE {
namespace MAX {

struct EStorage : public NLMISC::Exception
{
	EStorage() : NLMISC::Exception("PIPELINE::MAX::EStorage") { }
	virtual ~EStorage() throw() { }
};

// IStorageObject : exposes serial(CStorageStream *stream) and dump(const std::string &pad)
class IStorageObject
{
public:
	virtual void serial(CStorageStream *stream) = 0;
	virtual void dump(const std::string &pad) = 0;
};

class IStorageStreamable : public NLMISC::IStreamable, public IStorageObject
{
public:
	virtual void serial(CStorageStream *stream);
	virtual void serial(NLMISC::IStream &stream) = 0;
};
void IStorageStreamable::serial(CStorageStream *stream)
{
	serial(*((NLMISC::IStream *)stream));
}

// CStorageContainer : serializes a container chunk
class CStorageContainer : public std::vector<std::pair<uint16, IStorageObject *> >, public IStorageObject
{
public:
	virtual void serial(CStorageStream *stream);
	virtual void dump(const std::string &pad);

	// override in subclasses, call parent if not handled
	virtual IStorageObject *serialChunk(CStorageStream *stream);
};

// CStorageRaw : serializes raw data, use for unknown data
class CStorageRaw : public std::vector<uint8>, public IStorageObject
{
public:
	virtual void serial(CStorageStream *stream);
	virtual void dump(const std::string &pad);
};

// CStorageUCString : serializes an ucstring chunk
class CStorageUCString : public ucstring, public IStorageObject
{
public:
	virtual void serial(CStorageStream *stream);
	virtual void dump(const std::string &pad);
};

// CStorageString : serializes a string chunk
class CStorageString : public std::string, public IStorageObject
{
public:
	virtual void serial(CStorageStream *stream);
	virtual void dump(const std::string &pad);
};

template<typename T>
class CStorageValue : public IStorageObject
{
public:
	T Value;
	virtual void serial(CStorageStream *stream);
	virtual void dump(const std::string &pad);
};

struct CClass_ID : public NLMISC::IStreamable
{
	uint32 A;
	uint32 B;

	virtual void serial(NLMISC::IStream &stream);
	virtual std::string getClassName();
};
void CClass_ID::serial(NLMISC::IStream &stream)
{
	stream.serial(A);
	stream.serial(B);
}
std::string CClass_ID::getClassName()
{
	return "Class_ID";
}

struct CClassDirectoryHeader : public IStorageStreamable
{
	uint32 DllIndex;
	CClass_ID ClassID;
	uint32 SuperClassID;
	virtual void serial(NLMISC::IStream &stream);
	virtual void dump(const std::string &pad);
	virtual std::string getClassName();
};
void CClassDirectoryHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(DllIndex);
	stream.serial(ClassID);
	stream.serial(SuperClassID);
}
void CClassDirectoryHeader::dump(const std::string &pad)
{
	printf("CClassDirectoryHeader - DllIndex: %u, ClassID: (A: 0x%X, B: 0x%X), SuperClassID: 0x%X\n", DllIndex, ClassID.A, ClassID.B, SuperClassID);
}
std::string CClassDirectoryHeader::getClassName()
{
	return "ClassDirectoryHeader";
}

void CStorageContainer::serial(CStorageStream *stream)
{
	if (stream->isReading())
	{
		while (stream->enterChunk())
		{
			uint16 id = stream->getChunkId();
			IStorageObject *storageObject = serialChunk(stream);
			push_back(std::pair<uint16, IStorageObject *>(id, storageObject));
			if (stream->leaveChunk()) // bytes were skipped while reading
				throw EStorage();
		}
	}
	else
	{
		throw EStorage();
	}
}
IStorageObject *CStorageContainer::serialChunk(CStorageStream *stream)
{
	IStorageObject *storageObject = NULL;
	if (stream->isChunkContainer())
	{
		switch (stream->getChunkId())
		{
		case 0x2040: // ClassEntry: container with dll index, class id, superclass id and class name
		case 0x2038: // DllEntry: container with dll desc and name
		default:
			storageObject = new CStorageContainer();
			break;
		}
	}
	else
	{
		switch (stream->getChunkId())
		{
		case 0x2060:
				storageObject = new CClassDirectoryHeader();
				break;
		case 0x21C0: // FileVersion: 4 byte in the dlldir thing, exists in 2010 but not in 3.x
				storageObject = new CStorageValue<uint32>();
				break;
		case 0x2039: // DllDescription: dll description in the DllEntry
		case 0x2037: // DllFilename: dll name in the DllEntry
		case 0x2042: // ClassName: name of class in ClassEntry
				storageObject = new CStorageUCString();
				break;
		default:
				storageObject = new CStorageRaw();
				break;
		}
	}
	storageObject->serial(stream);
	return storageObject;
}
void CStorageContainer::dump(const std::string &pad)
{
	printf("CStorageContainer - items: %i\n", (sint32)size());
	for (iterator it = this->begin(), end = this->end(); it != end; ++it)
	{
		std::string padpad = pad + "\t";
		printf("%s[0x%X] ", padpad.c_str(), (sint32)(it->first));
		(it->second)->dump(padpad);
	}
}

void CStorageRaw::serial(CStorageStream *stream)
{
	if (stream->isReading())
	{
		resize(stream->getChunkSize());
		stream->serialBuffer(&(*this)[0], size());
	}
	else
	{
		throw EStorage();
	}
}
void CStorageRaw::dump(const std::string &pad)
{
	std::vector<uint8> buffer;
	buffer.resize(size() + 1);
	for (std::vector<uint8>::size_type i = 0; i < size(); ++i)
		buffer[i] = cleanChar((*this)[i]);
	buffer[buffer.size() - 1] = 0;
	printf("CStorageRaw - bytes: %i\n", (sint32)size());
	printf("%s%s\n", pad.c_str(), &buffer[0]);
}

void CStorageUCString::serial(CStorageStream *stream)
{
	if (stream->isReading())
	{
		sint32 size = stream->getChunkSize();
		resize(size / 2);
		stream->serialBuffer((uint8 *)&(*this)[0], size);
	}
	else
	{
		throw EStorage();
	}
}
void CStorageUCString::dump(const std::string &pad)
{
	std::vector<uint8> buffer;
	buffer.resize(size() + 1);
	for (size_type i = 0; i < size(); ++i)
		buffer[i] = (*this)[i] > 255 ? 46 : cleanChar((*this)[i]);
	buffer[buffer.size() - 1] = 0;
	printf("CStorageUCString - length: %i\n", (sint32)size());
	printf("%s%s\n", pad.c_str(), &buffer[0]);
}

void CStorageString::serial(CStorageStream *stream)
{
	if (stream->isReading())
	{
		resize(stream->getChunkSize());
		stream->serialBuffer((uint8 *)(&(*this)[0]), size());
	}
	else
	{
		throw EStorage();
	}
}
void CStorageString::dump(const std::string &pad)
{
	std::vector<uint8> buffer;
	buffer.resize(size() + 1);
	for (std::vector<uint8>::size_type i = 0; i < size(); ++i)
		buffer[i] = cleanChar((*this)[i]);
	buffer[buffer.size() - 1] = 0;
	printf("CStorageString - length: %i\n", (sint32)size());
	printf("%s%s\n", pad.c_str(), &buffer[0]);
}

template <typename T>
void CStorageValue<T>::serial(CStorageStream *stream)
{
	stream->serial(Value);
}
template <typename T>
void CStorageValue<T>::dump(const std::string &pad)
{
	printf("CStorageValue - bytes: %i\n", (sint32)(sizeof(T)));
	std::string valstr = NLMISC::toString(Value);
	printf("%s%s\n", pad.c_str(), valstr.c_str());
}

}
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
	//PIPELINE::MAX::CStorageContainer ctr;
	//ctr.serial(instream);
	//ctr.dump("");
	delete instream;

	g_object_unref(input);
	g_object_unref(infile);

	return 0;
}

