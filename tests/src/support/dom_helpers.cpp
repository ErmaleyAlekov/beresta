#include "support/dom_helpers.hpp"

#include <utility>

namespace support
{
    // Перехватываем исключение и возвращаем его код вместо проброса
    // Catches the exception and returns its code instead of rethrowing
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
    // A zero line in the result means there was no exception
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
    // Parsing with the default settings and output to a string
    std::string roundTrip(std::string_view input)
    {
        return beresta::XmlParser().parse(input).toString();
    }

    // Каталог создаётся заново внутри системного каталога временных файлов
    // The directory is created anew inside the system temporary directory
    std::filesystem::path makeTestDirectory(const std::string& name)
    {
        std::filesystem::path directory =
            std::filesystem::temp_directory_path() / "beresta_tests" / name;

        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
        return directory;
    }

    // Запись через io библиотеки — заодно проверяется и она
    // Writing through the library io — it is tested along the way
    void writeTestFile(const std::filesystem::path& path, const std::string& content)
    {
        beresta::io::writeFile(path, content);
    }
}
