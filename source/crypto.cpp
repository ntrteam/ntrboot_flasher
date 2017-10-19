#include <cstdint>
#include <cstring>

using std::uint8_t;
using std::uint32_t;

#define REG_SHACNT      (reinterpret_cast<volatile uint32_t *>(0x1000A000))
#define REG_SHABLKCNT   (reinterpret_cast<volatile uint32_t *>(0x1000A004))
#define REG_SHAHASH     (reinterpret_cast<volatile uint32_t *>(0x1000A040))
#define REG_SHAINFIFO   (reinterpret_cast<volatile uint32_t *>(0x1000A080))

#define SHA_CNT_STATE           0x00000003
#define SHA_CNT_OUTPUT_ENDIAN   0x00000008
#define SHA_CNT_MODE            0x00000030
#define SHA_CNT_ENABLE          0x00010000
#define SHA_CNT_ACTIVE          0x00020000

#define SHA_HASH_READY          0x00000000
#define SHA_NORMAL_ROUND        0x00000001
#define SHA_FINAL_ROUND         0x00000002

#define SHA256_MODE             0
#define SHA224_MODE             0x00000010
#define SHA1_MODE               0x00000020

namespace {
    constexpr uint32_t sha_hashsize(const uint32_t mode) {
        switch (mode & SHA_CNT_MODE) {
            case SHA1_MODE:
                return 160/8;
            case SHA224_MODE:
                return 224/8;
            case SHA256_MODE:
                return 256/8;
            default:
                return 0;
        }
    }

    void sha_init(const uint32_t mode) {
        while (*REG_SHACNT & 1);
        *REG_SHACNT = mode | SHA_CNT_OUTPUT_ENDIAN | SHA_NORMAL_ROUND;
    }

    void sha_update(const void *src, uint32_t size) {
        const uint32_t *src32 = static_cast<const uint32_t *>(src);
        while (size >= 0x40) {
            while (*REG_SHACNT & 1);
            for(uint32_t i = 0; i < 4; i++) {
                *REG_SHAINFIFO = *src32++;
                *REG_SHAINFIFO = *src32++;
                *REG_SHAINFIFO = *src32++;
                *REG_SHAINFIFO = *src32++;
            }
            size -= 0x40;
        }

        while (*REG_SHACNT & 1);
        while (size >= 4) {
            *REG_SHAINFIFO = *src32++;
            size -= 4;
        }
        const uint8_t *src8 = static_cast<const uint8_t *>(static_cast<const void *>(src32));
        volatile uint8_t *dst8 = static_cast<volatile uint8_t *>(static_cast<volatile void *>(REG_SHAINFIFO));
        for (uint32_t i = 0; i < size; i++) {
            *dst8++ = *src8++;
        }
    }

    void sha_get(void *res) {
        const uint32_t hash_size = sha_hashsize(*REG_SHACNT);
        uint32_t *const dst32 = static_cast<uint32_t *>(res);

        *REG_SHACNT = (*REG_SHACNT & ~SHA_NORMAL_ROUND) | SHA_FINAL_ROUND;
        while (*REG_SHACNT & SHA_FINAL_ROUND);
        while (*REG_SHACNT & 1);

        for (uint32_t i = 0; i < hash_size/4; ++i) {
            *(dst32 + i) = *(REG_SHAHASH + i);
        }
    }

    void sha_quick(void *res, const void *src, const uint32_t size, const uint32_t mode) {
        sha_init(mode);
        sha_update(src, size);
        sha_get(res);
    }
}

namespace crypto {
    void sha256sum(const void *data, const uint32_t datasize, uint8_t (&hash)[32]) {
        sha_quick(&hash, data, datasize, SHA256_MODE);
    }

    bool sha256verify(const void *data, const uint32_t datasize, const uint8_t (&hash)[32]) {
        uint8_t calchash[32];
        sha256sum(data, datasize, calchash);
        return !std::memcmp(calchash, hash, sizeof(calchash));

        static_assert(sizeof(calchash) == sizeof(hash), "SHA256 hash sizes differ?");
    }
}
