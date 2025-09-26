#!/usr/bin/env bash
set -euo pipefail

# Fetch and build upstream snes9x (tag 1.63) into src/core/upstream.
# This script is conservative: it doesn't modify upstream sources here, it
# clones the repo and attempts to build it using a common workflow.

SOURCES_DIR=${SOURCES_DIR:-src/core/upstream}
UPSTREAM_REPO=${UPSTREAM_REPO:-https://github.com/snes9xgit/snes9x.git}
UPSTREAM_TAG=${UPSTREAM_TAG:-1.63}
BUILD_DIR=${BUILD_DIR:-${SOURCES_DIR}/build}

echo "Fetching upstream snes9x into ${SOURCES_DIR} (tag ${UPSTREAM_TAG})"

if [ -d "${SOURCES_DIR}/.git" ]; then
  echo "Upstream already cloned; fetching latest tags"
  git -C "${SOURCES_DIR}" fetch --tags || true
else
  git clone "${UPSTREAM_REPO}" "${SOURCES_DIR}"
fi

git -C "${SOURCES_DIR}" fetch --tags

echo "Checking out ${UPSTREAM_TAG}"
 git -C "${SOURCES_DIR}" checkout "${UPSTREAM_TAG}" || git -C "${SOURCES_DIR}" checkout "tags/${UPSTREAM_TAG}" || true

mkdir -p "${BUILD_DIR}"

# Try common build methods in order
pushd "${BUILD_DIR}" > /dev/null

if [ -f "${SOURCES_DIR}/CMakeLists.txt" ]; then
  echo "Upstream provides CMakeLists.txt; building with CMake"
  cmake "${SOURCES_DIR}" -DCMAKE_BUILD_TYPE=Release
  cmake --build . -- -j 2
else
  # Try autotools or plain Makefile
  if [ -f "${SOURCES_DIR}/Makefile" ]; then
    echo "Found Makefile; attempting make"
    (cd "${SOURCES_DIR}" && make -j 2)
  else
    # Try subdir builds or platform specific
    echo "No CMakeLists or Makefile detected; attempting generic build steps"
    if [ -f "${SOURCES_DIR}/autogen.sh" ]; then
      (cd "${SOURCES_DIR}" && ./autogen.sh && ./configure && make -j 2)
    else
      echo "Unable to determine build system for upstream Snes9x. Please build upstream manually or augment this script."
      exit 2
    fi
  fi
fi

popd > /dev/null

# Report potential libraries produced
echo "Search for produced libraries in ${BUILD_DIR} and upstream dirs:"
LIBS_FOUND=$(find "${BUILD_DIR}" "${SOURCES_DIR}" -type f \( -name "*.a" -o -name "*.dylib" -o -name "*.so" \) 2>/dev/null || true)
if [ -z "${LIBS_FOUND}" ]; then
  echo "No library files found after build. Upstream may have built a different artifact or failed to build." >&2
  exit 3
fi

echo "Found libraries:"
for f in ${LIBS_FOUND}; do echo "  $f"; done

echo "To integrate with this project, ensure CMake uses the produced library. You can set -DUSE_UPSTREAM_CORE=ON and optionally -DSNES9X_UPSTREAM_TARGET to an imported target name mapped to one of the above libraries."

exit 0
