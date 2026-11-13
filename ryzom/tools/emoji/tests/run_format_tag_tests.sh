#!/bin/bash
# Build and run the CViewText format-tag tests inside the build container.
#
# Uses g++-4.8 with -std=gnu++11, exactly what CMAKE_CXX_COMPILER/FLAGS say the
# client is built with. Anything newer would both diverge from the code under
# test and risk an ABI mismatch against libnelgui.
#
# There is no CI harness for this: nel_unit_test needs cpptest, which is not
# installed, and WITH_NEL_TESTS is off in this build anyway. So the test links
# the real libnelgui that the client itself uses and is run on demand.
#
#   ./run_format_tag_tests.sh
#
# Assumes the source has been synced into the build volume and nelgui built:
#   cd ~/dev/ryzom_docker && ./sync-local.sh
#   docker run --rm -v ryzom-src-ryzom_test:/src -v ryzom-build-ryzom_test:/build \
#       --entrypoint bash ryzom-client-builder:latest -c 'cd /build && make nelgui -j8'
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_VOLUME="${SRC_VOLUME:-ryzom-src-ryzom_test}"
BUILD_VOLUME="${BUILD_VOLUME:-ryzom-build-ryzom_test}"
IMAGE="${IMAGE:-ryzom-client-builder:latest}"

docker run --rm \
	-v "${SRC_VOLUME}":/src:ro \
	-v "${BUILD_VOLUME}":/build:ro \
	-v "${SCRIPT_DIR}":/tests:ro \
	--entrypoint bash "${IMAGE}" -c '
		set -euo pipefail
		g++-4.8 -std=gnu++11 -O2 -DNL_RELEASE -DHAVE_X86_64 \
			-I/src/ryzom-core/nel/include \
			-I/build \
			/tests/test_format_tags.cpp \
			-L/build/lib -lnelgui -lnelmisc \
			-Wl,-rpath,/build/lib \
			-o /tmp/test_format_tags
		/tmp/test_format_tags
	'
