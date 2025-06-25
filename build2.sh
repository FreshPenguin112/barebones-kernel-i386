#!/bin/bash
# Build script for kernel with GDB debugging support
# Usage: ./build2.sh
# To debug: run this script, then in another terminal: gdb mykernel.elf

rm -f *.o userland/*.o initfs/*.o initfs/*.elf initfs.tar initramfs.c
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/i686-elf-gcc-toolchain:5.5.0 bash -c "sh userlandbuild.sh"
#sleep 2
tar -cf initfs.tar -C initfs .
xxd -i initfs.tar > initramfs.c
#sleep 2
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/i686-elf-gcc-toolchain:5.5.0 bash -c "sh build.sh"
#docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/i686-elf-gcc-toolchain:5.5.0 bash -c "i686-elf-strip --strip-all initfs/*.elf mykernel.elf"