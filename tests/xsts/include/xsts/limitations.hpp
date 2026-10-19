#ifndef XSTS_LIMITATIONS_HPP
#define XSTS_LIMITATIONS_HPP

#include "xsts/test_case.hpp"

#include <filesystem>
#include <map>
#include <string>

namespace xsts
{
    /*
    ** Сознательные ограничения библиотеки (раздел «Ограничения» в
    ** docs/comparison.md) и случаи вне XSD 1.0. Список ведётся вручную
    ** в known_limitations.txt после разбора расхождений: строка
    **     ключ  набор/группа[/случай]   # пояснение
    ** Ключ — одна из причин (upa, particle-restrict, disputed...).
    ** Случай из списка всё равно прогоняется: если он прошёл, он считается
    ** прошедшим, а провал попадает в отдельную графу «ограничение»
    ** и в процент не входит. Так список не прячет исправленные случаи.
    **
    ** The deliberate limitations of the library (the "Limitations" section in
    ** docs/comparison.md) and the cases outside XSD 1.0. The list is maintained by hand
    ** in known_limitations.txt after analysing the deviations: a line
    **     key  set/group[/case]   # explanation
    ** The key is one of the reasons (upa, particle-restrict, disputed...).
    ** A listed case is still run: if it passed, it counts as passed,
    ** and a failure goes into a separate "limitation" column
    ** and is not included in the percentage. So the list does not hide fixed cases.
    */
    class Limitations
    {
    public:
        // Читает список; отсутствующий файл — пустой список
        // Reads the list; a missing file is an empty list
        static Limitations load(const std::filesystem::path& file);

        // Причина-ограничение для случая или пустая строка
        // The limitation reason for a case or an empty string
        std::string reasonFor(const TestCase& testCase) const;

    private:
        // Описание причины по ключу из файла (неизвестный ключ — ошибка окружения)
        // The reason description by the key from the file (an unknown key is an environment error)
        static std::string describe(const std::string& key);

        std::map<std::string, std::string> _byId;    // "набор/группа[/случай]" -> описание
                                                    // "set/group[/case]" -> description
    };

    // Причина пропуска (спорный случай, вне XSD 1.0) или пустая строка
    // The skip reason (a disputed case, outside XSD 1.0) or an empty string
    std::string skipReason(const TestCase& testCase);
}

#endif
