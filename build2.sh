rm -f *.o userland/*.o initfs/*.o initfs/*.elf initfs.tar initramfs.c
docker run --rm --mount type=bind,source=$(realpath .),target=/root -w /root bjrowlett2/x86_64-elf-gcc:latest bash -c "sed -i 's/x86_64-elf-gcc /x86_64-elf-gcc -mcmodel=kernel /g' userlandbuild.sh && sh userlandbuild.sh"
#sleep 2
tar -cf initfs.tar -C initfs .
xxd -i initfs.tar > initramfs.c
#sleep 2
docker run --rm --mount type=bind,source=$(realpath .),target=/root -w /root bjrowlett2/x86_64-elf-gcc:latest bash -c "sed -i 's/x86_64-elf-gcc /x86_64-elf-gcc -mcmodel=kernel /g' build.sh && sh build.sh"
docker run --rm --mount type=bind,source=$(realpath .),target=/root -w /root bjrowlett2/x86_64-elf-gcc:latest bash -c "x86_64-elf-strip --strip-all initfs/*.elf mykernel.elf"