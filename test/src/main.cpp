#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <zlib.h>

int main(void) {
    char buffer_in [256] = {"Conan is a MIT-licensed, Open Source package manager for C and C++ development, "
                            "allowing development teams to easily and efficiently manage their packages and "
                            "dependencies across platforms and build systems."};
    char buffer_out [256] = {0};

    z_stream defstream = {
        .next_in = (Bytef *)buffer_in,
        .avail_in = (uInt)std::strlen(buffer_in),
        .total_in = 0,
        .next_out = (Bytef *)buffer_out,
        .avail_out = (uInt)sizeof(buffer_out),
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
    };

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    std::printf("Uncompressed size is: %lu\n", strlen(buffer_in));
    std::printf("Compressed size is: %lu\n", defstream.total_out);

    std::printf("ZLIB VERSION: %s\n", zlibVersion());

#ifdef __clang__
    constexpr char which[] = "clang";
#elif __GNUC__
    constexpr char which[] = "gcc";
#else
    constexpr char which[] = "unknown";
#endif
    std::printf("Compiler used: %s\n", which);

    return EXIT_SUCCESS;
}
