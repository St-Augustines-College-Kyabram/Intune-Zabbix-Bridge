// SPDX-License-Identifier: MIT
#include "../src/summary_transport.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <zlib.h>

static std::string decode_base64(const std::string& s) {
    std::string result;
    unsigned bits = 0;
    int count = -8;
    for (char ch : s) {
        if (ch == '=') break;
        int value;
        if (ch >= 'A' && ch <= 'Z') value = ch-'A';
        else if (ch >= 'a' && ch <= 'z') value = ch-'a'+26;
        else if (ch >= '0' && ch <= '9') value = ch-'0'+52;
        else if (ch == '+') value = 62;
        else if (ch == '/') value = 63;
        else { assert(false && "invalid base64"); return {}; }
        bits = (bits << 6) | static_cast<unsigned>(value);
        count += 6;
        if (count >= 0) {
            result.push_back(static_cast<char>((bits >> count) & 0xffu));
            count -= 8;
        }
    }
    return result;
}
static void check(const std::string &raw, int expected) {
    std::vector<unsigned char> buffer(64000, 0xCD);
    size_t written = 123;
    int status = infiltrator_zabbix_summary_encode(
        reinterpret_cast<const unsigned char *>(raw.data()), raw.size(),
        buffer.data(), buffer.size(), &written);
    assert(status == expected);
    if (status != 0) {
        for (unsigned char c : buffer) assert(c == 0xCD);
        return;
    }
    const std::string wire(reinterpret_cast<const char *>(buffer.data()), written);
    if (raw.size() <= 64000) { assert(wire == raw); return; }
    const std::string start = "{\"transport\":\"intune-zabbix-zlib-v1\",\"uncompressed_bytes\":"
        + std::to_string(raw.size()) + ",\"data\":\"";
    assert(wire.rfind(start,0) == 0 && wire.size() >= start.size()+2);
    const std::string compressed = decode_base64(wire.substr(start.size(), wire.size()-start.size()-2));
    std::string decoded(raw.size(), '\0');
    uLongf length = static_cast<uLongf>(decoded.size());
    const int z = uncompress(reinterpret_cast<Bytef *>(decoded.data()), &length,
        reinterpret_cast<const Bytef *>(compressed.data()), static_cast<uLong>(compressed.size()));
    assert(z == Z_OK && length == raw.size() && decoded == raw);
}
int main() {
    check("{\"devices\":[]}",0);
    check(std::string(64000,'x'),0);
    check(std::string(64001,'x'),0);
    check(std::string(4000000,'x'),0);
    check(std::string(4000001,'x'),1);
    std::mt19937 gen(7312);
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string random;random.reserve(100000);
    for (int i=0;i<100000;++i) random.push_back(alphabet[gen()%alphabet.size()]);
    check(random,2);
    size_t required=999;
    const std::string tiny="abcdef";
    assert(infiltrator_zabbix_summary_encode(
        reinterpret_cast<const unsigned char *>(tiny.data()),tiny.size(),nullptr,0,&required)==3);
    assert(required==tiny.size());
    assert(infiltrator_zabbix_summary_encode(nullptr,1,nullptr,0,&required)==5);
    std::cout << "C++ transport: all native tests passed\n";
}
