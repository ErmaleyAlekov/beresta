#include "failing/options.hpp"
#include "failing/list_error.hpp"

#include <limits>

namespace failing
{
    /*
    ** Аргументы без значения проверяются первыми; аргумент со значением
    ** забирает следующий элемент argv; слово без «-» — префикс id.
    **
    ** The arguments without a value are checked first; an argument with a value
    ** takes the next argv element; a word without "-" is an id prefix.
    */
    Options parseOptions(int argc, char** argv)
    {
        std::filesystem::path home = detail::defaultHome();
        Options options;

        options.listFile = home / "failing_cases.tsv";
        options.suiteRoot = home / ".." / "xsts" / "xmlschema2006-11-06";
        options.limitationsFile = home / ".." / "xsts" / "known_limitations.txt";
        for (int index = 1; index < argc; ++index)
        {
            std::string current = argv[index];

            if (current == "--stop-on-first")
                options.run.stopOnFirst = true;
            else if (current == "--regenerate")
                options.regenerate = true;
            else if (detail::parseValueOption(current, index + 1 < argc ? argv[index + 1] : "", options))
                ++index;
            else if (!current.empty() && current[0] != '-')
                options.run.prefix = current;
            else
                throw ListError("неизвестный аргумент: " + current);
        }
        return options;
    }

    namespace detail
    {
        // Макрос задаёт только CMakeLists.txt: путь с пробелами там экранируется надёжно
        // Only CMakeLists.txt sets the macro: a path with spaces is escaped reliably there
        std::filesystem::path defaultHome()
        {
#ifdef XSTS_FAILING_HOME
            return std::filesystem::u8path(XSTS_FAILING_HOME);
#else
            return std::filesystem::path(".");
#endif
        }

        // Возвращает false, если имя — не аргумент со значением
        // Returns false if the name is not an argument with a value
        bool parseValueOption(const std::string& name, const std::string& value, Options& options)
        {
            if ((name == "--case" || name == "--list" || name == "--suite" || name == "--limitations")
                && (value.empty() || value.compare(0, 2, "--") == 0))
                throw ListError("нет значения для " + name);
            if (name == "--case")
                options.run.caseNumber = parseNumber(value);
            else if (name == "--list")
                options.listFile = std::filesystem::u8path(value);
            else if (name == "--suite")
                options.suiteRoot = std::filesystem::u8path(value);
            else if (name == "--limitations")
                options.limitationsFile = std::filesystem::u8path(value);
            else
                return false;
            return true;
        }

        // Только десятичные цифры и не ноль: номера в списке начинаются с 1
        // Only decimal digits and not zero: the numbers in the list start from 1
        std::size_t parseNumber(const std::string& text)
        {
            std::size_t number = 0;

            for (char symbol : text)
            {
                if (symbol < '0' || symbol > '9')
                    throw ListError("номер случая должен быть целым числом: " + text);
                auto digit = static_cast<std::size_t>(symbol - '0');
                if (number > (std::numeric_limits<std::size_t>::max() - digit) / 10)
                    throw ListError("слишком большой номер случая: " + text);
                number = number * 10 + digit;
            }
            if (number == 0)
                throw ListError("номер случая начинается с 1: " + text);
            return number;
        }
    }
}
