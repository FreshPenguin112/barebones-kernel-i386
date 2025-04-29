rm -f *.o
rm -f initfs/*.elf
sleep 1
docker run --mount type=bind,source=$(realpath .),target=/root -w /root kevincharm/i686-elf-gcc-toolchain:5.5.0 sh build.sh
sleep 1
tar -cf initfs.tar -C initfs .
xxd -i initfs.tar > initramfs.c