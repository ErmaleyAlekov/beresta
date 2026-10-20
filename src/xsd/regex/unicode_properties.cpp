#include "xsd/regex/char_set.hpp"
#include "xsd/regex/unicode_tables.hpp"

namespace beresta::xsd
{
    /*
    ** Допустимое имя категории по грамматике XSD 1.0 (часть 2, продукции
    ** [28]-[35]): класс и необязательный подкласс из своего списка. Первая
    ** буква строки — класс, остальные — его подклассы. Cs спецификация
    ** исключает: суррогаты не бывают символами XML-документа.
    **
    ** A valid category name by the XSD 1.0 grammar (part 2, productions
    ** [28]-[35]): a class and an optional subclass from its own list. The first
    ** letter of a string is the class, the rest are its subclasses. The specification
    ** excludes Cs: surrogates are never characters of an XML document.
    */
    static bool isCategoryName(std::string_view name) noexcept
    {
        static const std::string_view CLASSES[] = {"Lultmo", "Mnce", "Ndlo", "Pcdseifo", "Zslp", "Smcko", "Ccfon"};

        if (name.empty() || name.size() > 2)
            return false;
        for (std::string_view letters : CLASSES)
        {
            if (letters[0] != name[0])
                continue;
            if (name.size() == 1)
                return true;
            return letters.find(name[1], 1) != std::string_view::npos;
        }
        return false;
    }

    // Все назначенные коды Unicode 3.1; промежутки между ними — категория Cn
    // All assigned Unicode 3.1 codes; the gaps between them are the Cn category
    static CharSet assignedSet()
    {
        CharSet result;

        for (std::size_t index = 0; index < UNICODE_CATEGORY_COUNT; ++index)
            result.add(UNICODE_CATEGORIES[index].first, UNICODE_CATEGORIES[index].last);
        result.normalize();
        return result;
    }

    /*
    ** Категория: \p{L} собирает диапазоны всего класса, \p{Lu} — только
    ** подкласса. Неназначенных кодов (Cn) в таблице нет: это дополнение
    ** ко всем назначенным, и оно же входит в общий класс C.
    **
    ** A category: \p{L} collects the ranges of the whole class, \p{Lu} only those
    ** of the subclass. Unassigned codes (Cn) are not in the table: they are the complement
    ** of all assigned ones, and it is also part of the whole class C.
    */
    static CharSet categorySet(std::string_view name)
    {
        CharSet result;
        bool wholeClass = name.size() == 1;

        for (std::size_t index = 0; index < UNICODE_CATEGORY_COUNT; ++index)
        {
            const CategoryRange& range = UNICODE_CATEGORIES[index];
            if (range.major == name[0] && (wholeClass || range.minor == name[1]))
                result.add(range.first, range.last);
        }
        if (name == "C" || name == "Cn")
            result.addSet(assignedSet().complement());
        result.normalize();
        return result;
    }

    // Блок \p{IsИмя}; одно имя может занимать несколько диапазонов (IsSpecials)
    // The \p{IsName} block; one name can occupy several ranges (IsSpecials)
    static bool addBlock(std::string_view name, CharSet& result)
    {
        bool found = false;

        for (std::size_t index = 0; index < UNICODE_BLOCK_COUNT; ++index)
        {
            if (name != UNICODE_BLOCKS[index].name)
                continue;
            result.add(UNICODE_BLOCKS[index].first, UNICODE_BLOCKS[index].last);
            found = true;
        }
        result.normalize();
        return found;
    }

    /*
    ** Имя свойства из \p{...}: с приставкой Is — блок, иначе — общая
    ** категория. Имена сравниваются с учётом регистра, как в спецификации;
    ** неизвестное имя даёт known = false (ошибка схемы у вызывающего).
    **
    ** The property name from \p{...}: with the Is prefix it is a block, otherwise a general
    ** category. Names are compared case-sensitively, as in the specification;
    ** an unknown name gives known = false (a schema error at the caller).
    */
    CharSet unicodePropertySet(std::string_view name, bool& known)
    {
        CharSet result;

        known = false;
        if (name.substr(0, 2) == "Is")
        {
            known = addBlock(name, result);
            return result;
        }
        if (!isCategoryName(name))
            return result;
        known = true;
        return categorySet(name);
    }
}
