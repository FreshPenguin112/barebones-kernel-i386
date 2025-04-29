sudo rm *.o
#sudo rm initfs/*.elf
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/i686-elf-gcc-toolchain:5.5.0 sh build.sh
tar -cf initfs.tar -C initfs .
xxd -i initfs.tar > initramfs.c