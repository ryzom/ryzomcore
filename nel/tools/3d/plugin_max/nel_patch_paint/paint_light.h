#include "nel/3d/mesh.h"
#include "../nel_mesh_lib/calc_lm.h"

/*-------------------------------------------------------------------*/

namespace NL3D
{
class CLandscape;
class CScene;
}

class CPaintLight
{
public:

	// Setup the lights
	void setup (NL3D::CLandscape &landscape, NL3D::CScene &scene);

	// Build the lights
	void build (Interface &ip, INode *node = NULL);

private:
	// Array of light
	std::vector<SLightBuild>	_Lights;
};

/*-------------------------------------------------------------------*/
