rm -f *.o userland/*.o initfs/*.o initfs/*.elf initfs.tar initramfs.c
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/x86_64-elf-gcc-toolchain:latest bash -c "sh userlandbuild.sh"
#sleep 2
tar -cf initfs.tar -C initfs .
xxd -i initfs.tar > initramfs.c
#sleep 2
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/x86_64-elf-gcc-toolchain:latest bash -c "sh build.sh"
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/x86_64-elf-gcc-toolchain:latest bash -c "x86_64-elf-strip --strip-all initfs/*.elf mykernel.elf"