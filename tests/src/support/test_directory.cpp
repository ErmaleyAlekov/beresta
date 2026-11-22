#include "support/dom_helpers.hpp"

#include <random>

namespace
{
    // Только атомарно созданный каталог переходит во владение теста.
    // The test owns only a directory it has created atomically.
    std::filesystem::path createDirectory(const std::string& name)
    {
        std::random_device random;
        const auto root = std::filesystem::temp_directory_path();
        const auto prefix = "beresta_" + name + "_" + std::to_string(random()) + "_";
        for (std::size_t attempt = 0; ; ++attempt)
        {
            auto path = root / (prefix + std::to_string(attempt));
            if (std::filesystem::create_directory(path))
                return path;
        }
    }
}

namespace support
{
    TestDirectory::TestDirectory(const std::string& name) : path(createDirectory(name))
    {
    }

    TestDirectory::~TestDirectory()
    {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
}
