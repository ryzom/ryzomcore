title Ryzom Core: 0_setup.py (EXECUTABLES)
python 0_setup.py --noconf -ipj common/gamedev common/exedll common/cfg common/data_common
title Ryzom Core: 1_export.py (EXECUTABLES)
python 1_export.py -ipj common/gamedev common/exedll common/cfg common/data_common
title Ryzom Core: 2_build.py (EXECUTABLES)
python 2_build.py -ipj common/gamedev common/exedll common/cfg common/data_common
title Ryzom Core: 3_install.py (EXECUTABLES)
python 3_install.py -ipj common/gamedev common/exedll common/cfg common/data_common
title Ryzom Core: b1_client_dev.py
python b1_client_dev.py
title Ryzom Core: b2_shard_data.py
python b2_shard_data.py
title Ryzom Core: b3_shard_dev.py
python b3_shard_dev.py
title Ryzom Core: Ready
