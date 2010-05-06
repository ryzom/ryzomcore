// animation_set_builder.cpp : Defines the entry point for the console application.
//

#include <nel/misc/file.h>
#include <nel/3d/animation_set.h>
#include <nel/3d/register_3d.h>
#include "anim_utility.h"

using namespace NLMISC;
using namespace NL3D;


int main(int argc, char* argv[])
{
	NL3D::registerSerial3d();

	// Check num of args
	if (argc!=3)
	{
		// Output an help
		printf  (
					"animation_set_builder [animation_set.animset] [animation.anim]\n"
					"\tThis command line add or replace the animation.anim animation into the animation_set.animset animation set\n"
				);
	}
	else
	{
		try
		{
			// Tha animation set
			CAnimationSet animSet;

			// Try to open the animation set file
			CIFile inFile;
			if (inFile.open (argv[1]))
			{
				// Ok, read it
				animSet.serial (inFile);

				// Close it
				inFile.close ();
			}
			else
			{
				// Info message
				printf ("Info: can't open %s for reading. File created.\n", argv[1]);
			}

			// Animation
			CAnimation	*anim= new CAnimation;

			// Read the input animation
			if (inFile.open (argv[2]))
			{
				// Ok, read it
				anim->serial (inFile);

				// Close it
				inFile.close ();

				// *** Build/replace the animation set

				// build animation name.
				std::string		animName= getName(argv[2]);


				// animation found in the animation set?
				uint	animId=	animSet.getAnimationIdByName(animName);
				if( animId == CAnimation::NotFound)
				{
					// No, add it.
					animId= animSet.addAnimation(animName.c_str(),  anim);
				}
				else
				{
					// Copy the animation
					*animSet.getAnimation (animId)= *anim;
				}


				// Build the animation set
				animSet.build ();


				// *** Save the animation Set

				// Open a output file
				COFile outFile;
				if (outFile.open (argv[1]))
				{
					// Write it
					animSet.serial (outFile);
				}
				else
				{
					// Error message
					fprintf (stderr, "Error: can't open %s for writing.\n", argv[1]);
					
					// Error code
					return -1;
				}

				// That's it.
			}
			else
			{
				// Error message
				fprintf (stderr, "Error: can't open %s for reading.\n", argv[2]);
				
				// Error code
				return -1;
			}
		}
		catch (Exception& e)
		{
			// Error message
			fprintf (stderr, "Error: %s\n", e.what());
			
			// Error code
			return -1;
		}
	}
	return 0;
}
