# Files check request (`needcheck.flag`)

When the client cannot find a file it needs, the installation is incomplete or corrupted. The client then asks the launcher (ryztart) to rescan the files.

## How it works

- `NLMISC::CPath::requestFileCheck(missingFile)` (`nel/include/nel/misc/path.h`) writes a warning with the missing file name, then creates an empty file `needcheck.flag` in `getLogDirectory()`.
- The flag is created once per run. Each call still logs its own warning.
- The file is empty: it only means "rescan". The launcher rescans all the files with their hash, and removes the flag itself.
- Call it only for a file that must exist. Do not call it for optional files (a lookup with `throwException` or `displayWarning` at `false` is usually an optional file): the flag would make the launcher rescan at every start.

## Where it is called

- `CGlobalRetriever::CLrLoader::run` (`nel/src/pacs/global_retriever.cpp`): a PACS retriever `.lr` that is not found.
