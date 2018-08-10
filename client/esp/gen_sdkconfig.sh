#!/bin/bash

. ../makefile.prefix_esp

if [ -f sdkconfig ]; then
    rm -vf sdkconfig
fi

cp -av sdkconfig.temp sdkconfig
echo $PREFIX
sed -i 's\xtensa-esp32-elf-\'"$PREFIX"'\g' sdkconfig
chmod 440 sdkconfig
