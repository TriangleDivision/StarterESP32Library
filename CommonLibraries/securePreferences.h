#ifndef SECURE_PREFERENCES_H
#define SECURE_PREFERENCES_H

#include <Preferences.h>
#include <mbedtls/aes.h>
#include <esp_random.h>

class SecurePreferences {
private:
    static const uint8_t DEFAULT_KEY[16];
    static const uint8_t DEFAULT_IV[16];
    
    static void aesEncrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* input, uint8_t* output, size_t length) {
        mbedtls_aes_context aes;
        mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_enc(&aes, key, 128);
        mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, length, (unsigned char*)iv, input, output);
        mbedtls_aes_free(&aes);
    }

    static void aesDecrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* input, uint8_t* output, size_t length) {
        mbedtls_aes_context aes;
        mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_dec(&aes, key, 128);
        mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, length, (unsigned char*)iv, input, output);
        mbedtls_aes_free(&aes);
    }

public:
    static void saveEncryptedString(Preferences &prefs, const char* key, const String &value, const uint8_t* encryptionKey = nullptr, const uint8_t* iv = nullptr) {
        if(encryptionKey == nullptr)
          encryptionKey = DEFAULT_KEY;
        if(iv == nullptr)
          iv = DEFAULT_IV;
          
        size_t paddedLength = ((value.length() + 15) / 16) * 16;  // Pad to 16-byte blocks
        uint8_t *input = new uint8_t[paddedLength];
        uint8_t *output = new uint8_t[paddedLength];
        memset(input, 0, paddedLength);
        memcpy(input, value.c_str(), value.length());
        aesEncrypt(encryptionKey, iv, input, output, paddedLength);
        prefs.putBytes(key, output, paddedLength);
        delete[] input;
        delete[] output;
    }

    static String readDecryptedString(Preferences &prefs, const char* key, const uint8_t* encryptionKey = nullptr, const uint8_t* iv = nullptr) {
        if(encryptionKey == nullptr)
          encryptionKey = DEFAULT_KEY;
        if(iv == nullptr)
          iv = DEFAULT_IV;

        size_t length = prefs.getBytesLength(key);
        if (length == 0) return "";
        uint8_t *encryptedData = new uint8_t[length];
        uint8_t *decryptedData = new uint8_t[length];
        prefs.getBytes(key, encryptedData, length);
        aesDecrypt(encryptionKey, iv, encryptedData, decryptedData, length);
        String result = String((char*)decryptedData);
        delete[] encryptedData;
        delete[] decryptedData;
        return result;
    }
};

// Define the static members outside the class
inline constexpr uint8_t SecurePreferences::DEFAULT_KEY[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x06, 0x0c, 0x0d, 0x07, 0x02};
inline constexpr uint8_t SecurePreferences::DEFAULT_IV[16] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x14, 0x17, 0x17, 0x19, 0x1a, 0x1b, 0x1c, 0x17, 0x1e, 0x1f};

#endif // SECURE_PREFERENCES_H