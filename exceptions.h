#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <stdint.h>

void exception_dispatcher(uint64_t vector, uint64_t error_code);

#endif // EXCEPTIONS_H
