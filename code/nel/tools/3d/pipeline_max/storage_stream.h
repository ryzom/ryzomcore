/**
 * \file storage_stream.h
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * CStorageStream
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_STORAGE_STREAM_H
#define PIPELINE_STORAGE_STREAM_H
#include <nel/misc/types_nl.h>

// STL includes
#include <map>

// 3rd Party includes
#include <gsf/gsf-infile.h>
#include <gsf/gsf-outfile.h>

// NeL includes
#include <nel/misc/stream.h>

// Project includes

namespace PIPELINE {
namespace MAX {

/**
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * CStorageStream
 */
class CStorageStream : public NLMISC::IStream
{
public:
	CStorageStream(GsfInput *input);
	CStorageStream(GsfOutput *output);
	virtual ~CStorageStream();

	virtual bool seek(sint32 offset, TSeekOrigin origin) const;
	virtual sint32 getPos() const;
	// virtual std::string getStreamName() const; // char const   *      gsf_input_name                      (GsfInput *input);
	virtual void serialBuffer(uint8 *buf, uint len);
	virtual void serialBit(bool &bit);

	sint32 size();
	bool eof();

private:
	GsfInput *m_Input;
	GsfOutput *m_Output;

/* there exist compressed max files, so maybe we will need this at some point
GsfInput *          gsf_input_uncompress                (GsfInput *src);
*/

}; /* class CStorageStream */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_STREAM_H */

/* end of file */
