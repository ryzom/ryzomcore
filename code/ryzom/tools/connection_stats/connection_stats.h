

#ifndef CONNECTION_STATS_H
#define CONNECTION_STATS_H

#include <nel/misc/types_nl.h>
#include <vector>
#include <string>


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CONNECTION_STATS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CONNECTION_STATS_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef LOG_ANALYSER_PLUGIN_EXPORTS
#define LOG_ANALYSER_PLUGIN_API __declspec(dllexport)
#else
#define LOG_ANALYSER_PLUGIN_API __declspec(dllimport)
#endif


LOG_ANALYSER_PLUGIN_API std::string getInfoString();
LOG_ANALYSER_PLUGIN_API bool doAnalyse( const std::vector<const char *>& vec, std::string& res, std::string& log );

#endif