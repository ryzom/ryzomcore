title Ryzom Core: 0_setup.py (RING LANDSCAPE)
python 0_setup.py --noconf -ipj continents/r2_lakes
title Ryzom Core: 1_export.py (RING LANDSCAPE)
python 1_export.py -ipj continents/r2_lakes
title Ryzom Core: 2_build.py (RING LANDSCAPE)
python 2_build.py -ipj continents/r2_lakes
title Ryzom Core: 3_install.py (RING LANDSCAPE)
python 3_install.py -ipj continents/r2_lakes
title Ryzom Core: a1_worldedit_data.py (RING LANDSCAPE)
python a1_worldedit_data.py
title Ryzom Core: b1_client_dev.py (RING LANDSCAPE)
python b1_client_dev.py
title Ryzom Core: b2_shard_data.py (RING LANDSCAPE)
python b2_shard_data.py
title Ryzom Core: Ready (RING LANDSCAPE)
dir *.py
