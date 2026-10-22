#ifndef FAILING_FAILING_CASE_HPP
#define FAILING_FAILING_CASE_HPP

#include <string>

namespace failing
{
    /*
    ** Одна строка списка провалов failing_cases.tsv. Список строится
    ** прогоном (--regenerate), а не руками: id — ключ, по которому случай
    ** ищется в каталоге набора; остальные поля — справка для человека,
    ** чтобы по одной строке понять, что открыть и чего ожидать.
    **
    ** One line of the failure list failing_cases.tsv. The list is built
    ** by a run (--regenerate), not by hand: id is the key by which the case
    ** is looked up in the suite catalog; the other fields are a reference for a human,
    ** so that one line tells what to open and what to expect.
    */
    struct FailingEntry
    {
        std::string id;          // "набор/группа/случай" / "set/group/case"
        std::string kind;        // schemaTest или instanceTest / schemaTest or instanceTest
        std::string expected;    // ожидание набора: valid / invalid / the suite expectation: valid / invalid
        std::string actual;      // что сделала библиотека: valid / invalid / сбой / нет схемы
                                 // what the library did: valid / invalid / сбой (crash) / нет схемы (no schema)
        std::string documents;   // документы случая относительно корня набора через ";"
                                 // the case documents relative to the suite root, separated by ";"
        std::string schema;      // для instanceTest — документы схемы группы, иначе "-"
                                 // for an instanceTest — the group schema documents, otherwise "-"
        std::string message;     // причина провала одной строкой (текст библиотеки)
                                 // the failure reason as one line (the library text)
        std::string description; // первая documentation группы: что проверяет случай
                                 // the first documentation of the group: what the case checks
    };

    /*
    ** Итог выполнения одного случая на текущей библиотеке:
    ** passed — вердикт совпал с ожиданием набора (случай исправлен),
    ** actual — фактическое поведение в тех же словах, что и в списке.
    **
    ** The outcome of running one case on the current library:
    ** passed — the verdict matched the suite expectation (the case is fixed),
    ** actual — the actual behavior in the same words as in the list.
    */
    struct Execution
    {
        bool passed = false;
        std::string actual;
        std::string message;
    };
}

#endif
