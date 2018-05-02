#!/bin/bash

set -e

echo "## ARCH          = $ARCH"
echo "## CROSS_COMPILE = $CROSS_COMPILE"
echo "## KBUILD_OUTPUT = $KBUILD_OUTPUT"

mkdir -p $KBUILD_OUTPUT

export DEFCONFIG=${DEFCONFIG:-ppc64le_defconfig}
echo "## DEFCONFIG     = $DEFCONFIG"

JFACTOR=${JFACTOR:-$(nproc)}
echo "## JFACTOR       = $JFACTOR"

if [[ ${SILENT} == 0 ]]; then
    SILENT=""
else
    SILENT="-s"
fi
echo "## SILENT        = $SILENT"

rc=0

if [[ ${BUILD_KERNEL} != 0 ]]; then
    (set -x; make ${SILENT} ${DEFCONFIG}; make ${SILENT} -j ${JFACTOR} ${MAKE_TARGET}) || rc=$?
    echo "## Kernel build completed rc = $rc"
fi

if [[ $rc -eq 0 && ${BUILD_SELFTESTS} == 1 ]]; then
    (set -x; make ${SILENT} -j ${JFACTOR} -C tools/testing/selftests TARGETS=powerpc) || rc=$?
    echo "## Selftest build completed rc = $rc"
fi

if [[ $rc -ne 0 ]]; then
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "!! Error build failed rc $rc"
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
else
    echo "## Build completed OK"
fi

exit $rc
