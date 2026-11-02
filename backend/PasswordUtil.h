#ifndef PASSWORD_UTIL_H
#define PASSWORD_UTIL_H
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <windows.h>
#include <bcrypt.h>

inline std::string hexBytes(const std::vector<unsigned char>& bytes) {
    std::ostringstream out;
    for (unsigned char b : bytes) out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return out.str();
}
inline bool unhexBytes(const std::string& text, std::vector<unsigned char>& out) {
    if (text.size() % 2) return false;
    out.clear();
    for (size_t i = 0; i < text.size(); i += 2) {
        try { out.push_back(static_cast<unsigned char>(std::stoul(text.substr(i, 2), nullptr, 16))); }
        catch (...) { return false; }
    }
    return true;
}
inline bool derivePasswordKey(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& key) {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG) < 0) return false;
    key.assign(32, 0);
    NTSTATUS status = BCryptDeriveKeyPBKDF2(algorithm,
        reinterpret_cast<PUCHAR>(const_cast<char*>(password.data())), static_cast<ULONG>(password.size()),
        const_cast<PUCHAR>(salt.data()), static_cast<ULONG>(salt.size()), 100000, key.data(), static_cast<ULONG>(key.size()), 0);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    return status >= 0;
}
inline std::string hashPassword(const std::string& password) {
    if (password.rfind("pbkdf2-sha256$", 0) == 0) return password;
    std::vector<unsigned char> salt(16), key;
    if (BCryptGenRandom(nullptr, salt.data(), static_cast<ULONG>(salt.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0 || !derivePasswordKey(password, salt, key)) return "hash-error$";
    return "pbkdf2-sha256$100000$" + hexBytes(salt) + "$" + hexBytes(key);
}
inline bool verifyPassword(const std::string& password, const std::string& encoded) {
    if (encoded.rfind("hash-error$", 0) == 0) return false;
    if (encoded.rfind("pbkdf2-sha256$100000$", 0) != 0) return password == encoded; // legacy plaintext, upgraded on next password change
    auto split = encoded.find('$', 21);
    if (split == std::string::npos) return false;
    std::vector<unsigned char> salt, expected, actual;
    if (!unhexBytes(encoded.substr(21, split - 21), salt) || !unhexBytes(encoded.substr(split + 1), expected) || !derivePasswordKey(password, salt, actual) || expected.size() != actual.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < actual.size(); ++i) diff |= actual[i] ^ expected[i];
    return diff == 0;
}
#endif
