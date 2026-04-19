//
// Created by cereal on 19.04.26.
//

#ifndef PROMETHEUS_SIGNATURE_H
#define PROMETHEUS_SIGNATURE_H

#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/evp.h>

std::string md5_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        printf("Cannot make md5: File open failed.");
        return "";
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        printf("Cannot make md5: EVP_MD_CTX_new failed.");
        return "";
    }

    if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        printf("Cannot make md5: EVP_DigestInit_ex failed.");
        return "";
    }

    constexpr size_t BUF_SIZE = 64 * 1024;
    std::vector<char> buf(BUF_SIZE);

    while (file.read(buf.data(), BUF_SIZE) || file.gcount() > 0) {
        if (EVP_DigestUpdate(ctx, buf.data(), file.gcount()) != 1) {
            EVP_MD_CTX_free(ctx);
            printf("Cannot make md5: EVP_DigestUpdate failed.");
            return "";
        }
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  digest_len = 0;

    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(ctx);
        printf("Cannot make md5: EVP_DigestFinal_ex failed.");
        return "";
    }

    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < digest_len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];

    return oss.str();
}

#endif //PROMETHEUS_SIGNATURE_H