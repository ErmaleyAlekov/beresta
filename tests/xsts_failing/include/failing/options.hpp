#ifndef FAILING_OPTIONS_HPP
#define FAILING_OPTIONS_HPP

#include "failing/debug_run.hpp"

#include <filesystem>
#include <string>

namespace failing
{
    /*
    ** Аргументы командной строки:
    **   xsts_failing [префикс] [--case N] [--stop-on-first]
    **                [--list файл.tsv] [--suite папка] [--limitations файл]
    **   xsts_failing --regenerate [--list файл.tsv] [--suite папка] [--limitations файл]
    ** Пути по умолчанию считаются от папки tests/xsts_failing (см. defaultHome).
    **
    ** The command line arguments:
    **   xsts_failing [prefix] [--case N] [--stop-on-first]
    **                [--list file.tsv] [--suite folder] [--limitations file]
    **   xsts_failing --regenerate [--list file.tsv] [--suite folder] [--limitations file]
    ** The default paths are taken from the tests/xsts_failing folder (see defaultHome).
    */
    struct Options
    {
        std::filesystem::path listFile;
        std::filesystem::path suiteRoot;
        std::filesystem::path limitationsFile;
        RunOptions run;
        bool regenerate = false;     // пересобрать список вместо прогона / rebuild the list instead of running
    };

    // Разбирает аргументы; неизвестный аргумент — failing::ListError
    // Parses the arguments; an unknown argument — failing::ListError
    Options parseOptions(int argc, char** argv);

    namespace detail
    {
        /*
        ** Папка tests/xsts_failing: при сборке CMake её путь вшит макросом
        ** XSTS_FAILING_HOME (IDE запускает программу из папки сборки),
        ** при сборке make программа запускается из самой папки — ".".
        **
        ** The tests/xsts_failing folder: with a CMake build its path is embedded by the
        ** XSTS_FAILING_HOME macro (an IDE starts the program from the build folder),
        ** with a make build the program is started from the folder itself — ".".
        */
        std::filesystem::path defaultHome();

        // Разбирает аргумент со значением (--case, --list, --suite, --limitations)
        // Parses an argument with a value (--case, --list, --suite, --limitations)
        bool parseValueOption(const std::string& name, const std::string& value, Options& options);

        // Номер случая для --case: целое больше нуля, иначе failing::ListError
        // The case number for --case: an integer above zero, otherwise failing::ListError
        std::size_t parseNumber(const std::string& text);
    }
}

#endif
