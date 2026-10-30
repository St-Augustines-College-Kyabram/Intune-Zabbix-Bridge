// SPDX-License-Identifier: MIT
// Lossless Zabbix fleet-summary framing. Native C++ implementation.
#include "summary_transport.h"
#include <cstring>
#include <string>
#include <vector>
#include <zlib.h>

namespace {
constexpr size_t max_wire_bytes = 64000;
constexpr size_t max_decoded_bytes = 4000000;
constexpr char base64_digits[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::string base64_encode(const unsigned char *bytes, size_t size) {
    std::string output;
    output.reserve(((size + 2) / 3) * 4);
    for (size_t i = 0; i < size; i += 3) {
        const unsigned bits = (static_cast<unsigned>(bytes[i]) << 16)
            | (i + 1 < size ? (static_cast<unsigned>(bytes[i + 1]) << 8) : 0u)
            | (i + 2 < size ? static_cast<unsigned>(bytes[i + 2]) : 0u);
        output.push_back(base64_digits[(bits >> 18) & 63u]);
        output.push_back(base64_digits[(bits >> 12) & 63u]);
        output.push_back(i + 1 < size ? base64_digits[(bits >> 6) & 63u] : '=');
        output.push_back(i + 2 < size ? base64_digits[bits & 63u] : '=');
    }
    return output;
}
}  // namespace

extern "C" int infiltrator_zabbix_summary_encode(
    const unsigned char *input, size_t input_len,
    unsigned char *output, size_t output_capacity, size_t *output_len)
{
    if (!output_len || (!input && input_len) || (!output && output_capacity))
        return 5;
    *output_len = 0;
    if (input_len > max_decoded_bytes) return 1;
    if (input_len <= max_wire_bytes) {
        *output_len = input_len;
        if (output_capacity < input_len) return 3;
        if (input_len) std::memmove(output, input, input_len);
        return 0;
    }
    uLongf compressed_len = compressBound(static_cast<uLong>(input_len));
    std::vector<unsigned char> compressed(static_cast<size_t>(compressed_len));
    const int status = compress2(compressed.data(), &compressed_len, input,
                                 static_cast<uLong>(input_len), Z_BEST_COMPRESSION);
    if (status != Z_OK) return 4;
    const std::string encoded = base64_encode(compressed.data(), compressed_len);
    const std::string envelope =
        std::string("{\"transport\":\"intune-zabbix-zlib-v1\",\"uncompressed_bytes\":")
        + std::to_string(input_len) + ",\"data\":\"" + encoded + "\"}";
    if (envelope.size() > max_wire_bytes) return 2;
    *output_len = envelope.size();
    if (output_capacity < envelope.size()) return 3;
    std::memmove(output, envelope.data(), envelope.size());
    return 0;
}
