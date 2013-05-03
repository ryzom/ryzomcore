// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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


#include "stdpch.h"
#include <nel/misc/file.h>

extern "C"
{

	/* Library Includes */
#include "wwwsys.h"
#include "WWWUtil.h"
#include "WWWCore.h"
#include "WWWDir.h"
#include "WWWTrans.h"
#include "HTReqMan.h"
#include "HTBind.h"
#include "HTMulti.h"
#include "HTNetMan.h"
#include "HTChannl.h"
#include "nel/gui/libwww_nel_stream.h"		/* Implemented here */
}

using namespace std;
using namespace NLMISC;

extern "C"
{

	/* Final states have negative value */
typedef enum _FileState {
	FS_RETRY		= -4,
	FS_ERROR		= -3,
	FS_NO_DATA		= -2,
	FS_GOT_DATA		= -1,
	FS_BEGIN		= 0,
	FS_PENDING,
	FS_DO_CN,
	FS_NEED_OPEN_FILE,
	FS_NEED_BODY,
	FS_PARSE_DIR,
	FS_TRY_FTP
} FileState;

/* This is the context structure for the this module */
typedef struct _file_info {
	FileState	state;		  /* Current state of the connection */
	char *		local;		/* Local representation of file name */
	struct stat		stat_info;	      /* Contains actual file chosen */
	HTNet *		net;
	HTTimer *	timer;
} file_info;

struct _HTStream {
	const HTStreamClass *	isa;
};

struct _HTInputStream {
	const HTInputStreamClass *	isa;
	HTChannel *			ch;
	HTHost *			host;
	char *			write;			/* Last byte written */
	char *			read;			   /* Last byte read */
	int				b_read;
	char			data [INPUT_BUFFER_SIZE];	   /* buffer */
};

PRIVATE int FileCleanup (HTRequest *req, int status)
{
	HTNet * net = HTRequest_net(req);
	file_info * file = (file_info *) HTNet_context(net);
	HTStream * input = HTRequest_inputStream(req);

	/* Free stream with data TO Local file system */
	if (input)
	{
		if (status == HT_INTERRUPTED)
			(*input->isa->abort)(input, NULL);
		else
			(*input->isa->_free)(input);
		HTRequest_setInputStream(req, NULL);
	}

	/*
	**  Remove if we have registered a timer function as a callback
	*/
	if (file->timer)
	{
		HTTimer_delete(file->timer);
		file->timer = NULL;
	}

	if (file)
	{
		HT_FREE(file->local);
		HT_FREE(file);
	}

	HTNet_delete(net, status);
	return YES;
}


PRIVATE int FileEvent (SOCKET soc, void * pVoid, HTEventType type);

PUBLIC int HTLoadNeLFile (SOCKET soc, HTRequest * request)
{
	file_info *file;			      /* Specific access information */
	HTNet * net = HTRequest_net(request);
	HTParentAnchor * anchor = HTRequest_anchor(request);

	HTTRACE(PROT_TRACE, "HTLoadFile.. Looking for `%s\'\n" _
				HTAnchor_physical(anchor));
	if ((file = (file_info *) HT_CALLOC(1, sizeof(file_info))) == NULL)
	HT_OUTOFMEM((char*)"HTLoadFILE");
	file->state = FS_BEGIN;
	file->net = net;
	HTNet_setContext(net, file);
	HTNet_setEventCallback(net, FileEvent);
	HTNet_setEventParam(net, file);  /* callbacks get http* */

	return FileEvent(soc, file, HTEvent_BEGIN);	    /* get it started - ops is ignored */
}

PRIVATE int ReturnEvent (HTTimer * timer, void * param, HTEventType /* type */)
{
	file_info * file = (file_info *) param;
	if (timer != file->timer)
	HTDEBUGBREAK((char*)"File timer %p not in sync\n" _ timer);
	HTTRACE(PROT_TRACE, "HTLoadFile.. Continuing %p with timer %p\n" _ file _ timer);

	/*
	**  Delete the timer
	*/
	HTTimer_delete(file->timer);
	file->timer = NULL;

	/*
	**  Now call the event again
	*/
	return FileEvent(INVSOC, file, HTEvent_READ);
}

PUBLIC int HTNeLFileOpen (HTNet * net, char * local, HTLocalMode /* mode */)
{
	HTRequest * request = HTNet_request(net);
	HTHost * host = HTNet_host(net);
	CIFile* fp = new CIFile;

	if (!fp->open (local))
	{
		HTRequest_addSystemError(request, ERR_FATAL, errno, NO, (char*)"CIFile::open");
		return HT_ERROR;
	}

	HTHost_setChannel(host, HTChannel_new(INVSOC, (FILE*)fp, YES));

	HTHost_getInput(host, HTNet_transport(net), NULL, 0);
	HTHost_getOutput(host, HTNet_transport(net), NULL, 0);
	return HT_OK;
}

PRIVATE int FileEvent (SOCKET /* soc */, void * pVoid, HTEventType type)
{
	file_info *file = (file_info *)pVoid;			      /* Specific access information */
	int status = HT_ERROR;
	HTNet * net = file->net;
	HTRequest * request = HTNet_request(net);
	HTParentAnchor * anchor = HTRequest_anchor(request);

	if (type == HTEvent_CLOSE)
	{
		/* Interrupted */
		HTRequest_addError(request, ERR_FATAL, NO, HTERR_INTERRUPTED,
			NULL, 0, (char*)"HTLoadFile");
		FileCleanup(request, HT_INTERRUPTED);
		return HT_OK;
	}


	/* Now jump into the machine. We know the state from the previous run */
	for(;;)
	{
	switch (file->state)
	{
	case FS_BEGIN:

		/* We only support safe (GET, HEAD, etc) methods for the moment */
		if (!HTMethod_isSafe(HTRequest_method(request))) {
		HTRequest_addError(request, ERR_FATAL, NO, HTERR_NOT_ALLOWED,
				   NULL, 0, (char*)"HTLoadFile");
		file->state = FS_ERROR;
		break;
		}

		/* Check whether we have access to local disk at all */
		if (HTLib_secure())
		{
			HTTRACE(PROT_TRACE, "LoadFile.... No access to local file system\n");
			file->state = FS_TRY_FTP;
			break;
		}

		/*file->local = HTWWWToLocal(HTAnchor_physical(anchor), "",
					   HTRequest_userProfile(request));*/
		{
			string tmp = HTAnchor_physical(anchor);
			if (strlwr(tmp).find("file:/") == 0)
			{
				tmp = tmp.substr(6, tmp.size()-6);
			}
			StrAllocCopy(file->local, tmp.c_str());
		}

		if (!file->local)
		{
			file->state = FS_TRY_FTP;
			break;
		}

		/* Create a new host object and link it to the net object */
		{
		HTHost * host = NULL;
		if ((host = HTHost_new((char*)"localhost", 0)) == NULL) return HT_ERROR;
		HTNet_setHost(net, host);
		if (HTHost_addNet(host, net) == HT_PENDING) {
			HTTRACE(PROT_TRACE, "HTLoadFile.. Pending...\n");
			/* move to the hack state */
			file->state = FS_PENDING;
			return HT_OK;
		}
		}
		file->state = FS_DO_CN;
		break;

	case FS_PENDING:
		{
		HTHost * host = NULL;
		if ((host = HTHost_new((char*)"localhost", 0)) == NULL) return HT_ERROR;
		HTNet_setHost(net, host);
		if (HTHost_addNet(host, net) == HT_PENDING) {
			HTTRACE(PROT_TRACE, "HTLoadFile.. Pending...\n");
			file->state = FS_PENDING;
			return HT_OK;
		}
		}
		file->state = FS_DO_CN;
		break;

	case FS_DO_CN:
		if (HTRequest_negotiation(request) &&
		HTMethod_isSafe(HTRequest_method(request))) {

		HTAnchor_setPhysical(anchor, file->local);
		HTTRACE(PROT_TRACE, "Load File... Found `%s\'\n" _ file->local);

		} else {
		if (HT_STAT(file->local, &file->stat_info) == -1) {
			HTTRACE(PROT_TRACE, "Load File... Not found `%s\'\n" _ file->local);
			HTRequest_addError(request, ERR_FATAL, NO, HTERR_NOT_FOUND,
					   NULL, 0, (char*)"HTLoadFile");
			file->state = FS_ERROR;
			break;
		}
		}

		if (((file->stat_info.st_mode) & S_IFMT) == S_IFDIR) {
		if (HTRequest_method(request) == METHOD_GET)
			file->state = FS_PARSE_DIR;
		else {
			HTRequest_addError(request, ERR_INFO, NO, HTERR_NO_CONTENT,
					   NULL, 0, (char*)"HTLoadFile");
			file->state = FS_NO_DATA;
		}
		break;
		}

		{
		BOOL editable = FALSE;
		HTBind_getAnchorBindings(anchor);
		if (editable) HTAnchor_appendAllow(anchor, METHOD_PUT);

		/* Set the file size */
		CIFile nelFile;
		if (nelFile.open (file->local))
		{
			file->stat_info.st_size = nelFile.getFileSize();
		}
		nelFile.close();

		if (file->stat_info.st_size)
			HTAnchor_setLength(anchor, file->stat_info.st_size);

		/* Set the file last modified time stamp */
		if (file->stat_info.st_mtime > 0)
			HTAnchor_setLastModified(anchor, file->stat_info.st_mtime);

		/* Check to see if we can edit it */
		if (!editable && !file->stat_info.st_size) {
			HTRequest_addError(request, ERR_INFO, NO, HTERR_NO_CONTENT,
					   NULL, 0, (char*)"HTLoadFile");
			file->state = FS_NO_DATA;
		} else {
			file->state = (HTRequest_method(request)==METHOD_GET) ?
			FS_NEED_OPEN_FILE : FS_GOT_DATA;
		}
		}
		break;

	  case FS_NEED_OPEN_FILE:
		status = HTNeLFileOpen(net, file->local, HT_FB_RDONLY);
		if (status == HT_OK) {
		{
			HTStream * rstream = HTStreamStack(HTAnchor_format(anchor),
							   HTRequest_outputFormat(request),
							   HTRequest_outputStream(request),
							   request, YES);
			HTNet_setReadStream(net, rstream);
			HTRequest_setOutputConnected(request, YES);
		}

		{
			HTOutputStream * output = HTNet_getOutput(net, NULL, 0);
			HTRequest_setInputStream(request, (HTStream *) output);
		}

		if (HTRequest_isSource(request) && !HTRequest_destinationsReady(request))
			return HT_OK;
		HTRequest_addError(request, ERR_INFO, NO, HTERR_OK, NULL, 0,
				   (char*)"HTLoadFile");
		file->state = FS_NEED_BODY;

		if (HTEvent_isCallbacksRegistered()) {
			if (!HTRequest_preemptive(request)) {
			if (!HTNet_preemptive(net)) {
				HTTRACE(PROT_TRACE, "HTLoadFile.. Returning\n");
				HTHost_register(HTNet_host(net), net, HTEvent_READ);
			} else if (!file->timer) {
				HTTRACE(PROT_TRACE, "HTLoadFile.. Returning\n");
				file->timer =
				HTTimer_new(NULL, ReturnEvent, file, 1, YES, NO);
			}
			return HT_OK;
			}
		}
		} else if (status == HT_WOULD_BLOCK || status == HT_PENDING)
		return HT_OK;
		else {
		HTRequest_addError(request, ERR_INFO, NO, HTERR_INTERNAL,
				   NULL, 0, (char*)"HTLoadFile");
		file->state = FS_ERROR;		       /* Error or interrupt */
		}
		break;

	  case FS_NEED_BODY:
		status = HTHost_read(HTNet_host(net), net);
		if (status == HT_WOULD_BLOCK)
		return HT_OK;
		else if (status == HT_LOADED || status == HT_CLOSED) {
		file->state = FS_GOT_DATA;
		} else {
		HTRequest_addError(request, ERR_INFO, NO, HTERR_FORBIDDEN,
				   NULL, 0, (char*)"HTLoadFile");
		file->state = FS_ERROR;
		}
		break;

	  case FS_TRY_FTP:
		{
		char *url = HTAnchor_physical(anchor);
		HTAnchor *anchor;
		char *newname = NULL;
		StrAllocCopy(newname, "ftp:");
		if (!strncmp(url, "file:", 5))
			StrAllocCat(newname, url+5);
		else
			StrAllocCat(newname, url);
		anchor = HTAnchor_findAddress(newname);
		HTRequest_setAnchor(request, anchor);
		HT_FREE(newname);
		FileCleanup(request, HT_IGNORE);
		return HTLoad(request, YES);
		}
		break;

	  case FS_GOT_DATA:
		FileCleanup(request, HT_LOADED);
		return HT_OK;
		break;

	  case FS_NO_DATA:
		FileCleanup(request, HT_NO_DATA);
		return HT_OK;
		break;

	  case FS_RETRY:
		FileCleanup(request, HT_RETRY);
		return HT_OK;
		break;

	  case FS_ERROR:
		FileCleanup(request, HT_ERROR);
		return HT_OK;
		break;

	  default:
		break;
	}
	} /* End of while(1) */
}

// *************************************************************************
// HTNeLReader
// *************************************************************************

size_t nel_fread (void *buffer, uint size, FILE *fp)
{
	CIFile *file = (CIFile *)fp;
	int toRead = std::min ((int)(file->getFileSize () - file->getPos ()), (int)size);
	file->serialBuffer((uint8*)buffer, toRead);
	return toRead;
}

PRIVATE int HTNeLReader_read (HTInputStream * me)
{
	FILE * fp = HTChannel_file(me->ch);
	HTNet * net = HTHost_getReadNet(me->host);
	int status;

	/* Read the file desriptor */
	while (fp)
	{
		if ((me->b_read = (int)nel_fread(me->data, FILE_BUFFER_SIZE, fp)) == 0)
		{
			HTAlertCallback *cbf = HTAlert_find(HT_PROG_DONE);
			// HTTRACE(PROT_TRACE, "ANSI read... Finished loading file %p\n" _ fp);
			if (cbf)
				(*cbf)(net->request, HT_PROG_DONE, HT_MSG_NULL,NULL,NULL,NULL);
			return HT_CLOSED;
		}

		/* Remember how much we have read from the input socket */
		HTTRACEDATA(me->data, me->b_read, "HTANSIReader_read me->data:");
		me->write = me->data;
		me->read = me->data + me->b_read;

		{
			HTAlertCallback * cbf = HTAlert_find(HT_PROG_READ);
			HTNet_addBytesRead(net, me->b_read);
			if (cbf) {
				int tr = HTNet_bytesRead(net);
				(*cbf)(net->request, HT_PROG_READ, HT_MSG_NULL, NULL, &tr, NULL);
			}
		}

		if (!net->readStream)
			return HT_ERROR;

		/* Now push the data down the stream */
		if ((status = (*net->readStream->isa->put_block)
			(net->readStream, me->data, me->b_read)) != HT_OK) {
			if (status == HT_WOULD_BLOCK) {
				HTTRACE(PROT_TRACE, "ANSI read... Target WOULD BLOCK\n");
				return HT_WOULD_BLOCK;
			} else if (status == HT_PAUSE) {
				HTTRACE(PROT_TRACE, "ANSI read... Target PAUSED\n");
				return HT_PAUSE;
			} else if (status > 0) {	      /* Stream specific return code */
				HTTRACE(PROT_TRACE, "ANSI read... Target returns %d\n" _ status);
				me->write = me->data + me->b_read;
				return status;
			} else {				     /* We have a real error */
				HTTRACE(PROT_TRACE, "ANSI read... Target ERROR\n");
				return status;
			}
		}
		me->write = me->data + me->b_read;
	}
	HTTRACE(PROT_TRACE, "ANSI read... File descriptor is NULL...\n");
	return HT_ERROR;
}

PRIVATE int HTNeLReader_close (HTInputStream * me)
{
	CIFile *file = (CIFile *)HTChannel_file(me->ch);
	if (file)
	{
		file->close();
	}

	int status = HT_OK;
	HTNet * net = HTHost_getReadNet(me->host);


	if (net && net->readStream) {
	if ((status = (*net->readStream->isa->_free)(net->readStream))==HT_WOULD_BLOCK)
		return HT_WOULD_BLOCK;
	net->readStream = NULL;
	}
	HTTRACE(STREAM_TRACE, "Socket read. FREEING....\n");
	HT_FREE(me);
	return status;
}

PUBLIC int HTNeLReader_consumed (HTInputStream * me, size_t bytes)
{
	me->write += bytes;
	me->b_read -= (int)bytes;
	HTHost_setRemainingRead(me->host, me->b_read);
	return HT_OK;
}

PRIVATE int HTNeLReader_flush (HTInputStream * me)
{
	HTNet * net = HTHost_getReadNet(me->host);
	return net && net->readStream ? (*net->readStream->isa->flush)(net->readStream) : HT_OK;
}

PRIVATE int HTNeLReader_free (HTInputStream * me)
{
	CIFile *file = (CIFile *)HTChannel_file(me->ch);
	if (file)
	{
		delete file;
		HTChannel_setFile (me->ch, NULL);
	}

	HTNet * net = HTHost_getReadNet(me->host);
	if (net && net->readStream) {
		int status = (*net->readStream->isa->_free)(net->readStream);
		if (status == HT_OK) net->readStream = NULL;
		return status;
	}
	return HT_OK;
}

PRIVATE int HTNeLReader_abort (HTInputStream * me, HTList * /* e */)
{
	HTNet * net = HTHost_getReadNet(me->host);
	if (net && net->readStream) {
		int status = (*net->readStream->isa->abort)(net->readStream, NULL);
		if (status != HT_IGNORE) net->readStream = NULL;
	}
	return HT_ERROR;
}

PRIVATE const HTInputStreamClass HTNeLReader =
{
	(char*)"SocketReader",
	HTNeLReader_flush,
	HTNeLReader_free,
	HTNeLReader_abort,
	HTNeLReader_read,
	HTNeLReader_close,
	HTNeLReader_consumed
};

PUBLIC HTInputStream * HTNeLReader_new (HTHost * host, HTChannel * ch,
					 void * /* param */, int /* mode */)
{
	if (host && ch) {
	HTInputStream * me = HTChannel_input(ch);
	if (me == NULL) {
		if ((me=(HTInputStream *) HT_CALLOC(1, sizeof(HTInputStream))) == NULL)
		HT_OUTOFMEM((char*)"HTNeLReader_new");
		me->isa = &HTNeLReader;
		me->ch = ch;
		me->host = host;
		HTTRACE(STREAM_TRACE, "Reader...... Created reader stream %p\n" _ me);
	}
	return me;
	}
	return NULL;
}

//PUBLIC unsigned int WWW_TraceFlag = 0;

} // extern "C"


