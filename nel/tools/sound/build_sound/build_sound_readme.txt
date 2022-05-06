
////////////////////////////////////////////////////////////////////////

build_sound  <leveldesign> <dfn> <source_samplebanks> <build_packed_sheets> <build_samplebanks>

leveldesign = R:\leveldesign\
dfn = R:\leveldesign\dfn\
source_samplebanks = L:\assets\sound\samplebanks\
build_packed_sheets = L:\build\sound_sheets\
build_samplebanks = L:\build\sound_samplebanks\
Where L:\ is your game content database.

- leveldesign
Contains your game data sheets.

- dfn
Contains the game data sheet type definitions.

- source_samplebanks
Contains a subdirectory for each sample bank.
Inside each directory, your 16-bit PCM .wav files must be placed.

- build_packed_sheets
The build tool places the .packed_sheets files here.

- build_samplebanks
The build tool places the .sample_bank files here.

////////////////////////////////////////////////////////////////////////
