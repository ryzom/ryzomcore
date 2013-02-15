****************
* Master Tools *
****************

3 Main directories : Continents / Backup / Trash
------------------------------------------------

A World (like Continents or Backup) is a group of directories.
Each directory is a continent. And a continent is made by a
file continent.cfg which contains basics information about the
continent. A continent contains directories which represents regions

Example of directories

+ Continents
	+ Fyros
		+ near_the_city
		+ oasis
			oasis.flora
			oasisflora.prim
			fyros.tribe
			respawn.prim
		+ near_holes
		continent.cfg
		fyros.continent
	+ Matis
	+ Tryker
+ Backup
+ Trash


Backup/clean : keep the most recent version of all inside the backup
including continent and tag (tags are all the Continents world at a certain date)



Other launchable tools
----------------------

This tools regroup the 3 other tools in a common workspace.
- WorldEditor : Easily generates landscapes and primitives
- LogicEditor : Condition and trigger tools
- Georges : Edit the monsters, the primitives description and so on

With the master tool you can export vegetable through the landscape.
Simply create a new form herited from a vegetable.dfn and edit it.
The fields are
 + Include_patats A set of patats in which we will plant the "Plants"
 + Exclude_patats A set of patats in which the "Plants" will never appear
 + Plants         A structure of plants
    + Name            The name of the plant
    + Shape           The .shape associated
    + Shadow          The shape that will generate the shadows on the landscape 
    + CollisionRadius The radius of the base of the plant (serve to plant it on the ground)
    + BundingRadius   The minimum radius that between the center of the plant and another plant
 + Jitter_Pos     The "randomness" of the position of the plants in the patat (from 0.0 (order) to 1.0 (random))
 + Scale_Min      Minimum scaling for all plants ( normal scale is 1.0)
 + Scale_Max      Maximum scale for all plants
 + Put_On_Water   Do we have to put the plant on the water ?
 + Water_Height   If we put the plant on water at which height is the water.
 + Random_Seed    Number between 0 and 4,294,967,295 that initialize the random generation



WorldEditor
-----------

WorldEditor needs some directories to functionnate :
ZoneBitmaps	(contains the .TGA from the max plugin)
ZoneLigos	(contains the .LIGOZONE from the max plugin)
In the integrated mode (when launched from the master tool) you cannot paint zones.
This functionnality is accessible when you execute the stand alone program which is just
a DLL loader. You have access to the .prim edition of the zone and can link your primitives
to contents (vegetable plants, construction, IA zones, etc.) with the georges tool.



LogicEditor
-----------

This is a stand alone tool. It does not need any system data.



Georges
-------

Georges needs a root directory with a subdirectory called "DFN" (This directory regroups
the .DFN definition of a type class).



RESTRICTIONS
************

Do not use prim.dfn, land.dfn nor logic.dfn in georges, else if you
instanciate those you will get xxx.logic or something like that which
could be not easy to differenciate from a worldeditor logic file.