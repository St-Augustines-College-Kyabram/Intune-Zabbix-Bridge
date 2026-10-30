// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// 0 OK; 1 decoded >4 MB; 2 wire >64 KB; 3 output buffer too small;
// 4 zlib failure; 5 invalid arguments.
// On return 0 or 3 output_len is the required exact byte count.
// No NUL is added. Failed calls do not modify output data.
int infiltrator_zabbix_summary_encode(
    const unsigned char *input, size_t input_len,
    unsigned char *output, size_t output_capacity, size_t *output_len);
#ifdef __cplusplus
}
#endif
