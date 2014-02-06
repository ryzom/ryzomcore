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

#ifndef CONFIG_H
#define CONFIG_H

#include <nel/misc/config_file.h>

/**
 @brief Wrapper for a Ryzom config file, allows setting and querying values.
*/
class CConfig
{
public:
	CConfig();
	~CConfig();

	/**
	 @brief  Loads a config file.
	 @param  fileName - The file to load
	 @return Returns true on success, returns false on failure.
	*/
	bool load( const char *fileName );

	/**
	 @brief  Reloads the contents of the config file
	 @return Return true on success, returns false on failure.
	*/
	bool reload();

	/**
	 @brief   Reverts the config file to the default
	 @details Reverts the config file to the default if possible.
	          If there is no default config, it reverts the current changes only.
	*/
	void revertToDefault();

	/**
	 @brief  Saves the configuration to the config file.
	 @return Returns true on success, returns false on failure.
	*/
	bool save();

	/**
	 @brief   Queries the value for the specified key.
	 @param   key  -  The key we are interested in
	 @return  Returns the value as a bool, returns false if the key doesn't exist.
	*/
	bool getBool( const char *key );

	/**
	 @brief  Queries the value for the specified key.
     @param  key  -  The key we are interested in
	 @return Returns the value as an integer, returns 0 if the key doesn't exist.
	*/
	sint32 getInt( const char *key );

	/**
	 @brief  Queries the value for the specified key.
	 @param  key  -  The key we are interested in
	 @return Returns the value as a float, returns 0.0f if the key doesn't exist.
	*/
	float getFloat( const char *key );

	/**
	 @brief Queries the value for the specified key.
	 @param key  -  The key we are interested in
	 @return Returns the value as a std::string, returns an empty string if the key doesn't exist.
	*/
	std::string getString( const char *key );

	/**
	 @brief Sets the specified key to the specified value.
	 @param key   -  the key we want to alter
	 @param value -  the value we want to set
	*/
	void setBool( const char *key, bool value );

	/**
	 @brief Sets the specified key to the specified value.
	 @param key   -  the key we want to alter
	 @param value -  the value we want to set
	*/
	void setInt( const char *key,  sint32 value );

	/**
	 @brief Sets the specified key to the specified value.
	 @param key   -  the key we want to alter
	 @param value -  the value we want to set
    */
	void setFloat( const char *key, float value );

	/**
	 @brief Sets the specified key to the specified value.
	 @param key    -  the key we want to alter
	 @param value  -  the value we want to set
    */
	void setString( const char *key, const std::string &value );

private:
	// config file
	NLMISC::CConfigFile cf;
	// default config file
	NLMISC::CConfigFile dcf;
};

#endif
