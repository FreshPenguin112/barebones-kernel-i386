qemu-system-x86_64 \
  -enable-kvm \
  -cpu host \
  -kernel mykernel.elf \
  -nographic \
  -device isa-debug-exit,iobase=0x501,iosize=0x04 \
  -no-reboot