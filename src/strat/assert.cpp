module;

export module assert;

import std;

export void _assert(
    bool statement,
    const char *message,
    const std::source_location location = std::source_location::current()) {

#ifndef CACTUS_DISABLE_ASSERT
    if (statement) return;

    std::cerr << "\033[1m" << location.file_name() << "\033[0m:"
              << location.line() << ":"
              << location.column() << ": "
              << "\033[31mASSERTION FAILED\033[0m: "
              << message
              << std::endl;

    std::exit(1);
#endif
}

