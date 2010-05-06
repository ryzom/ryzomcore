@echo off
# Default the source directory to the current directory.
set NEL_DIR=%CD%
set BUILDTYPE=Experimental
# Load the development environment. If you run MSVC2008 you may need to change this to VS90COMNTOOLS.
call "%VS80COMNTOOLS%vsvars32.bat"

if "%1" == "" (
  rem No arguments were specified. Print out usage information.
  echo Usage: %0 <Continuous|Experimental|Nightly> [c:\path\to\source]
  exit /b
)
set BUILDTYPE=%1

if not "%2" == "" (
set NEL_DIR=%2
)


# Execute the build. If you run an Express version of MSVC you may need to change devenv to vcexpress.
devenv "%NEL_DIR%\build\NeL.sln" /build Debug /project %1%
