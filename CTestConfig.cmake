## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(Dart)
set(CTEST_PROJECT_NAME "RyzomCore")
set(CTEST_NIGHTLY_START_TIME "00:00:00 CST")
set(CTEST_UPDATE_TYPE "git")
set(CTEST_DROP_METHOD "https")
## Placeholder, a dashboard needs the CDash server it submits to
set(CTEST_DROP_SITE "example.ryzom.dev")
set(CTEST_DROP_LOCATION "/submit.php?project=RyzomCore")
set(CTEST_DROP_SITE_CDASH TRUE)
