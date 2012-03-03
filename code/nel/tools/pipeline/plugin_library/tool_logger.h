/**
 * \file tool_logger.h
 * \brief CToolLogger
 * \date 2012-02-19 10:33GMT
 * \author Jan Boon (Kaetemi)
 * Tool logger is fully implemented in header so small tools do not 
 * need to link to this library unnecessarily.
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_TOOL_LOGGER_H
#define PIPELINE_TOOL_LOGGER_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>

// NeL includes
#include <nel/misc/time_nl.h>
#include <nel/misc/string_common.h>

// Project includes

namespace PIPELINE {

#ifdef ERROR
#undef ERROR
#endif
enum EError
{
	ERROR, 
	WARNING, 
	MESSAGE, 
};

namespace {

const std::string s_Error = "ERROR";
const std::string s_Warning = "WARNING";
const std::string s_Message = "MESSAGE";

const std::string s_ErrorHeader = "type\tpath\ttime\terror";
const std::string s_DependHeader = "output_file\tinput_file";

}

/**
 * \brief CToolLogger
 * \date 2012-02-19 10:33GMT
 * \author Jan Boon (Kaetemi)
 * CToolLogger
 */
class CToolLogger
{
private:
	FILE *m_ErrorLog;
	FILE *m_DependLog;

public:
	CToolLogger()
	{

	}

	virtual ~CToolLogger()
	{
		release();
	}

	void initError(const std::string &errorLog)
	{
		releaseError();

		m_ErrorLog = fopen(errorLog.c_str(), "wt");
		fwrite(s_ErrorHeader.c_str(), 1, s_ErrorHeader.length(), m_ErrorLog);
		fwrite("\n", 1, 1, m_ErrorLog);
		fflush(m_ErrorLog);

	}

	void initDepend(const std::string &dependLog)
	{
		releaseDepend();

		m_DependLog = fopen(dependLog.c_str(), "wt");
		fwrite(s_DependHeader.c_str(), 1, s_DependHeader.length(), m_DependLog);
		fwrite("\n", 1, 1, m_DependLog);
		fflush(m_DependLog);
	}

	void writeError(EError type, const std::string &path, const std::string &error)
	{
		if (m_ErrorLog)
		{
			switch (type)
			{
			case ERROR:
				fwrite("ERROR", 1, 5, m_ErrorLog);
				break;
			case WARNING:
				fwrite("WARNING", 1, 7, m_ErrorLog);
				break;
			case MESSAGE:
				fwrite("MESSAGE", 1, 7, m_ErrorLog);
				break;
			}
			fwrite("\t", 1, 1, m_ErrorLog);
			fwrite(path.c_str(), 1, path.length(), m_ErrorLog);
			fwrite("\t", 1, 1, m_ErrorLog);
			std::string time = NLMISC::toString(NLMISC::CTime::getSecondsSince1970());
			fwrite(time.c_str(), 1, time.length(), m_ErrorLog);
			fwrite("\t", 1, 1, m_ErrorLog);
			fwrite(error.c_str(), 1, error.length(), m_ErrorLog);
			fwrite("\n", 1, 1, m_ErrorLog);
			fflush(m_ErrorLog);
		}
	}

	/// inputFile can only be file. May be not-yet-existing file for expected input. Directories are handled on process level. You should call this before calling writeError on inputFile, so the error is also linked from the outputFile.
	void writeDepend(const std::string &outputFile, const std::string &inputFile)
	{
		if (m_DependLog)
		{
			fwrite(outputFile.c_str(), 1, outputFile.length(), m_DependLog);
			fwrite("\t", 1, 1, m_DependLog);
			fwrite(inputFile.c_str(), 1, inputFile.length(), m_DependLog);
			fwrite("\n", 1, 1, m_DependLog);
			fflush(m_DependLog);
		}
	}

	void releaseError()
	{
		if (m_ErrorLog)
		{
			fflush(m_ErrorLog);
			fclose(m_ErrorLog);
			m_ErrorLog = NULL;
		}
	}

	void releaseDepend()
	{
		if (m_DependLog)
		{
			fflush(m_DependLog);
			fclose(m_DependLog);
			m_DependLog = NULL;
		}
	}

	void release()
	{
		releaseError();
		releaseDepend();
	}
}; /* class CToolLogger */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_TOOL_LOGGER_H */

/* end of file */
