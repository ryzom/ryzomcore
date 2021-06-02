

#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include <vector>


using namespace NLMISC;
using namespace std;


void	usage()
{
	printf("Usage: tga_resize tgaFileIn tgaFileOut divideRatio.\n   \
    eg: tga_resize pipo.tga pipo.tga 2\n \
        => pipo.tga widht and height are divided by 2 \n\n");

}


int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		usage();
		return -1;
	}
	else
	{
		CBitmap		btmp;
		CIFile		inFile;
		COFile		outFile;

		uint	divideRatio;
		NLMISC::fromString(argv[3], divideRatio);
		if(divideRatio==0 || !isPowerOf2(divideRatio))
		{
			printf("divideRatio must be a powerOf2 (1, 2, 4, 8 ...) \n");
			return 0;
		}

		try
		{
			// read.
			if (!inFile.open(argv[1]))
			{
				printf("Can't open input file %s \n", argv[1]);
				return -1;
			}
			uint8	depth= btmp.load(inFile);
			if(depth==0)
				throw Exception("Bad File Format");
			inFile.close();

			// resize.
			btmp.resample(btmp.getWidth() / divideRatio, btmp.getHeight() / divideRatio);

			// output.
			if (!outFile.open(argv[2]))
			{
				printf("Can't open output file %s \n", argv[2]);
				return -1;
			}
			btmp.writeTGA(outFile, depth);
		}
		catch (const Exception &e)
		{
			printf("Error: %s\n", e.what());
				return -1;
		}

		return 0;
	}
}
