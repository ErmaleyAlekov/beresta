#include "support/dom_helpers.hpp"

#include <utility>

namespace support
{
    // Перехватываем исключение и возвращаем его код вместо проброса
    AppendResult tryAppend(beresta::ContainerNode& parent, std::unique_ptr<beresta::Node>& child)
    {
        try
        {
            parent.appendChild(std::move(child));
        }
        catch (const beresta::DomException& error)
        {
            return AppendResult{false, error.getCode()};
        }
        return AppendResult{true, beresta::DomErrorCode::PARSE_ERROR};
    }

    // Нулевая строка в результате означает, что исключения не было
    beresta::ParsePosition parseErrorPosition(std::string_view input)
    {
        try
        {
            beresta::XmlParser().parse(input);
        }
        catch (const beresta::ParseException& error)
        {
            return error.getPosition();
        }
        return beresta::ParsePosition{0, 0, 0};
    }

    // Разбор с настройками по умолчанию и вывод в строку
    std::string roundTrip(std::string_view input)
    {
        return beresta::XmlParser().parse(input).toString();
    }

    // Каталог создаётся заново внутри системного каталога временных файлов
    std::filesystem::path makeTestDirectory(const std::string& name)
    {
        std::filesystem::path directory =
            std::filesystem::temp_directory_path() / "beresta_tests" / name;

        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
        return directory;
    }

    // Запись через io библиотеки — заодно проверяется и она
    void writeTestFile(const std::filesystem::path& path, const std::string& content)
    {
        beresta::io::writeFile(path, content);
    }
}
