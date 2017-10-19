namespace crypto {
    void sha256sum(const void *data, const uint32_t datasize, uint8_t (&hash)[32]);
    bool sha256verify(const void *data, const uint32_t datasize, const uint8_t (&hash)[32]);
}
