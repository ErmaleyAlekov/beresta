#include "xsd/simple_type.hpp"

#include <deque>

namespace beresta::xsd
{
    // Описание встроенного типа: имя, примитив, форма, пробелы, границы, тип элементов списка
    // A built-in type description: the name, the primitive, the form, whitespace, bounds, the list item type
    struct BuiltinEntry
    {
        const char* name;
        Primitive primitive;
        LexicalForm form;
        WhiteSpace whiteSpace;
        const char* minInclusive;
        const char* maxInclusive;
        const char* listItem;
    };

    /*
    ** Иерархия встроенных типов XSD 1.0 (часть 2 спецификации) в виде таблицы:
    ** производные целые задаются границами minInclusive / maxInclusive,
    ** списковые (NMTOKENS, IDREFS, ENTITIES) — типом элемента списка.
    **
    ** The hierarchy of the XSD 1.0 built-in types (part 2 of the specification) as a table:
    ** derived integers are defined by the minInclusive / maxInclusive bounds,
    ** list types (NMTOKENS, IDREFS, ENTITIES) by the list item type.
    */
    static const BuiltinEntry BUILTINS[] = {
        {"anySimpleType", Primitive::ANY_SIMPLE, LexicalForm::FREE, WhiteSpace::PRESERVE, "", "", ""},
        {"string", Primitive::STRING, LexicalForm::FREE, WhiteSpace::PRESERVE, "", "", ""},
        {"normalizedString", Primitive::STRING, LexicalForm::FREE, WhiteSpace::REPLACE, "", "", ""},
        {"token", Primitive::STRING, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"language", Primitive::STRING, LexicalForm::LANGUAGE, WhiteSpace::COLLAPSE, "", "", ""},
        {"Name", Primitive::STRING, LexicalForm::NAME, WhiteSpace::COLLAPSE, "", "", ""},
        {"NCName", Primitive::STRING, LexicalForm::NCNAME, WhiteSpace::COLLAPSE, "", "", ""},
        {"NMTOKEN", Primitive::STRING, LexicalForm::NMTOKEN, WhiteSpace::COLLAPSE, "", "", ""},
        {"ID", Primitive::STRING, LexicalForm::ID, WhiteSpace::COLLAPSE, "", "", ""},
        {"IDREF", Primitive::STRING, LexicalForm::IDREF, WhiteSpace::COLLAPSE, "", "", ""},
        {"ENTITY", Primitive::STRING, LexicalForm::ENTITY, WhiteSpace::COLLAPSE, "", "", ""},
        {"NMTOKENS", Primitive::STRING, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", "NMTOKEN"},
        {"IDREFS", Primitive::STRING, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", "IDREF"},
        {"ENTITIES", Primitive::STRING, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", "ENTITY"},
        {"boolean", Primitive::BOOLEAN, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"decimal", Primitive::DECIMAL, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"integer", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "", "", ""},
        {"nonPositiveInteger", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "", "0", ""},
        {"negativeInteger", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "", "-1", ""},
        {"nonNegativeInteger", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "0", "", ""},
        {"positiveInteger", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "1", "", ""},
        {"long", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE,
            "-9223372036854775808", "9223372036854775807", ""},
        {"int", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "-2147483648", "2147483647", ""},
        {"short", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "-32768", "32767", ""},
        {"byte", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "-128", "127", ""},
        {"unsignedLong", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE,
            "0", "18446744073709551615", ""},
        {"unsignedInt", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "0", "4294967295", ""},
        {"unsignedShort", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "0", "65535", ""},
        {"unsignedByte", Primitive::DECIMAL, LexicalForm::INTEGER, WhiteSpace::COLLAPSE, "0", "255", ""},
        {"float", Primitive::FLOAT, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"double", Primitive::DOUBLE, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"duration", Primitive::DURATION, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"dateTime", Primitive::DATE_TIME, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"time", Primitive::TIME, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"date", Primitive::DATE, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"gYearMonth", Primitive::G_YEAR_MONTH, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"gYear", Primitive::G_YEAR, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"gMonthDay", Primitive::G_MONTH_DAY, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"gDay", Primitive::G_DAY, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"gMonth", Primitive::G_MONTH, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"hexBinary", Primitive::HEX_BINARY, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"base64Binary", Primitive::BASE64_BINARY, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"anyURI", Primitive::ANY_URI, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"QName", Primitive::QNAME, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""},
        {"NOTATION", Primitive::NOTATION_TYPE, LexicalForm::FREE, WhiteSpace::COLLAPSE, "", "", ""}
    };

    // Тип элементов списка ищется среди уже созданных (он стоит в таблице раньше)
    // The list item type is looked up among the already created ones (it is earlier in the table)
    static const SimpleType* findBuilt(const std::deque<SimpleType>& built, std::string_view name)
    {
        for (const SimpleType& type : built)
        {
            if (type.name == name)
                return &type;
        }
        return nullptr;
    }

    // Объект встроенного типа по строке таблицы
    // A built-in type object from a table row
    static SimpleType makeBuiltin(const BuiltinEntry& entry, const std::deque<SimpleType>& built)
    {
        SimpleType type;
        std::string_view item(entry.listItem);

        type.name = entry.name;
        type.namespaceUri = std::string(XSD_NAMESPACE_URI);
        type.primitive = entry.primitive;
        type.form = entry.form;
        type.facets.whiteSpace = entry.whiteSpace;
        if (entry.minInclusive[0] != '\0')
            type.facets.minInclusive = std::string(entry.minInclusive);
        if (entry.maxInclusive[0] != '\0')
            type.facets.maxInclusive = std::string(entry.maxInclusive);
        if (!item.empty())
        {
            type.variety = Variety::LIST;
            type.itemType = findBuilt(built, item);
            type.facets.minLength = 1;
        }
        return type;
    }

    // Все встроенные типы в порядке таблицы
    // All built-in types in table order
    static std::deque<SimpleType> buildRegistry()
    {
        std::deque<SimpleType> registry;

        for (const BuiltinEntry& entry : BUILTINS)
            registry.push_back(makeBuiltin(entry, registry));
        return registry;
    }

    /*
    ** Реестр создаётся один раз при первом обращении: инициализация
    ** статической переменной в C++11 и новее потокобезопасна. std::deque
    ** не перемещает элементы при перемещении контейнера в статическую
    ** переменную, поэтому указатели на типы (itemType) остаются верными.
    **
    ** The registry is created once on first access: initialization of a
    ** static variable in C++11 and later is thread-safe. std::deque
    ** does not move its elements when the container is moved into the static
    ** variable, so the pointers to types (itemType) stay valid.
    */
    const SimpleType* builtinType(std::string_view localName)
    {
        static const std::deque<SimpleType> REGISTRY = buildRegistry();

        return findBuilt(REGISTRY, localName);
    }
}
