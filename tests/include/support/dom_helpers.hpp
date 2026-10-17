#ifndef DOM_HELPERS_HPP
#define DOM_HELPERS_HPP

#include "beresta.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace support
{
    // Итог попытки вставки: успех или код ошибки (code имеет смысл только при неудаче)
    struct AppendResult
    {
        bool succeeded;
        beresta::DomErrorCode code;
    };

    // Вставляет child в parent; при ошибке владение остаётся в child
    AppendResult tryAppend(beresta::ContainerNode& parent, std::unique_ptr<beresta::Node>& child);

    // Разбирает ввод, ожидая ошибку; возвращает её позицию (line = 0, если ошибки не было)
    beresta::ParsePosition parseErrorPosition(std::string_view input);

    // Разбор и немедленная сериализация — основа большинства тестов
    std::string roundTrip(std::string_view input);

    // Чистый временный каталог для тестов с файлами
    std::filesystem::path makeTestDirectory(const std::string& name);

    // Записывает файл с указанным содержимым (UTF-8)
    void writeTestFile(const std::filesystem::path& path, const std::string& content);
}

#endif
