#!/bin/sh

OPENOCD=${OPENOCD:-openocd}
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

${OPENOCD} -f ${DIR}/openocd.cfg --command "adapter_khz 10000; init; reset init; start_barebox"
