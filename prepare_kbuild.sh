#!/bin/bash
cp -r $1/module/* $2/lkm/
cp $1/library/include/dpi_shared_defs.h $2/lkm/include/
cd $2/lkm
echo "obj-m += dpi_module.o" > Kbuild
for src_file in $(find ./ -name '*.c'); do
    name=${src_file%.c}
    echo "dpi_module-y += ${name}.o" >> Kbuild
done
echo "" >> Kbuild
echo "ccflags-y := -I$2/lkm/include" >> Kbuild

