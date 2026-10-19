#include "xsts/limitations.hpp"
#include "xsts/xsts_error.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace xsts
{
    namespace
    {
        // Отрезает комментарий "# ..." и пробелы по краям
        // Cuts off the "# ..." comment and the spaces at the edges
        std::string stripComment(const std::string& line)
        {
            std::string text = line.substr(0, line.find('#'));
            std::size_t first = text.find_first_not_of(" \t\r");

            if (first == std::string::npos)
                return std::string();
            return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
        }
    }

    /*
    ** Строка файла: «ключ идентификатор». Пустые строки и комментарии
    ** пропускаются; неизвестный ключ — ошибка окружения, чтобы опечатка
    ** не превратила ограничение в тихий провал.
    **
    ** A file line: "key identifier". Empty lines and comments
    ** are skipped; an unknown key is an environment error so that a typo
    ** does not turn a limitation into a silent failure.
    */
    Limitations Limitations::load(const std::filesystem::path& file)
    {
        Limitations limitations;
        std::ifstream stream(file, std::ios::binary);
        std::string line;

        while (stream && std::getline(stream, line))
        {
            std::istringstream fields(stripComment(line));
            std::string key;
            std::string id;

            if (!(fields >> key))
                continue;
            if (!(fields >> id))
                throw XstsError("в " + file.u8string() + " строка без идентификатора: " + line);
            limitations._byId[id] = describe(key);
        }
        return limitations;
    }

    // Сначала точное совпадение со случаем, затем — вся группа "набор/группа"
    // First an exact match with the case, then the whole "set/group" group
    std::string Limitations::reasonFor(const TestCase& testCase) const
    {
        auto exact = _byId.find(testCase.id);

        if (exact != _byId.end())
            return exact->second;
        auto group = _byId.find(testCase.testSet + "/" + testCase.group);
        if (group != _byId.end())
            return group->second;
        return std::string();
    }

    /*
    ** Ключи соответствуют пунктам раздела «Ограничения» docs/comparison.md;
    ** disputed — случаи, где ожидание набора расходится со спецификацией
    ** или зависит от необязательного поведения процессора (README.md рядом).
    **
    ** The keys correspond to the items of the "Limitations" section of docs/comparison.md;
    ** disputed — the cases where the expectation of the suite disagrees with the specification
    ** or depends on optional processor behavior (the README.md next to it).
    */
    std::string Limitations::describe(const std::string& key)
    {
        static const char* const REASONS[][2] = {
            {"upa", "однозначность модели (UPA, cos-nonambig) не проверяется"},
            {"particle-restrict", "корректность ограничения модели (cos-particle-restrict) не проверяется"},
            {"redefine-restrict", "xs:redefine без ссылки на себя не проверяется как ограничение (src-redefine.6.2, 7.2)"},
            {"duration-order", "у duration нет порядка для фасетов-границ"},
            {"unresolved-location", "недоступный schemaLocation считается ошибкой загрузки, а не пропускается"},
            {"s4s-namespace", "схемы с targetNamespace XML Schema (схема для схем) не загружаются"},
            {"redefine-cycle", "циклы через xs:redefine (A переопределяет B, B переопределяет или включает A)"},
            {"nested-derivation", "тип, выведенный из составного внутри его же модели, считается циклическим"},
            {"disputed", "ожидание набора расходится со спецификацией или зависит от процессора"}
        };

        for (const auto& reason : REASONS)
        {
            if (key == reason[0])
                return reason[1];
        }
        throw XstsError("неизвестный ключ ограничения: " + key);
    }
}
