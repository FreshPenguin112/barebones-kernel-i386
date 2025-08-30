#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>

void kernel_output_capture_start(char *buf, size_t bufsize);
void kernel_output_capture_stop(void);

#endif
