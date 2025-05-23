#include "crypto_utils.h"

std::string Crypt(const std::string& input, const std::string& key) {
    std::string output = input;
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] ^= key[i % key.size()];
    }
    return output;
}
