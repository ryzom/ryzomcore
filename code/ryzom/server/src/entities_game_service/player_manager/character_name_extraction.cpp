
#include "stdpch.h"
#include "character_name_extraction.h"

using namespace std;
using namespace NLMISC;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


#define PERSISTENT_CLASS CEntityBaseNameExtraction
#define PERSISTENT_NO_STORE

#define PERSISTENT_DATA\
	PROP(string,Name)\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


#define PERSISTENT_CLASS CCharacterNameExtraction
#define PERSISTENT_NO_STORE

#define PERSISTENT_DATA\
 
#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA
