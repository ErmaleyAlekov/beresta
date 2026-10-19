#include "xpath/internal/xpath_parser.hpp"
#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Пространство имён типов XML Schema (префикс конструкторов xs:*)
    // The XML Schema types namespace (the prefix of the xs:* constructors)
    static const char* const SCHEMA_TYPES_NAMESPACE = "http://www.w3.org/2001/XMLSchema";

    // Строка таблицы конструкторов: локальное имя типа и вид приведения
    // A row of the constructor table: the local type name and the kind of cast
    struct ConstructorEntry
    {
        const char* localName;
        AtomicType type;
    };

    // Поддерживаемые конструкторы xs:тип(…)
    // The supported xs:type(…) constructors
    static const ConstructorEntry CONSTRUCTORS[] = {
        {"string", AtomicType::STRING}, {"boolean", AtomicType::BOOLEAN}, {"decimal", AtomicType::DECIMAL},
        {"integer", AtomicType::INTEGER}, {"double", AtomicType::DOUBLE}, {"date", AtomicType::DATE},
        {"dateTime", AtomicType::DATE_TIME}
    };

    /*
    ** Конструктор по имени prefix:local: префикс обязан быть связан с
    ** пространством XML Schema в документе схемы (как xs или xsd), а
    ** локальное имя — быть в таблице.
    **
    ** A constructor by the name prefix:local: the prefix must be bound to
    ** the XML Schema namespace in the schema document (like xs or xsd), and
    ** the local name must be in the table.
    */
    static bool constructorType(const std::string& name, const XPathContext* namespaces, AtomicType& type)
    {
        std::size_t colon = name.find(':');

        if (colon == std::string::npos || namespaces == nullptr)
            return false;
        auto found = namespaces->namespaces.find(name.substr(0, colon));
        if (found == namespaces->namespaces.end() || found->second != SCHEMA_TYPES_NAMESPACE)
            return false;
        for (const ConstructorEntry& entry : CONSTRUCTORS)
        {
            if (name.compare(colon + 1, std::string::npos, entry.localName) != 0)
                continue;
            type = entry.type;
            return true;
        }
        return false;
    }

    /*
    ** Вызов функции в утверждении XSD — проверка при компиляции, то есть при
    ** загрузке схемы: конструктор xs:* (ровно один аргумент), функция XPath
    ** 1.0 или exists / empty. Всё прочее (fn:matches, tokenize, xs:time …)
    ** не поддерживается и даёт ошибку, а не «тихое» значение при проверке.
    **
    ** A function call in an XSD assertion — checked at compilation, that is when
    ** the schema is loaded: an xs:* constructor (exactly one argument), an XPath
    ** 1.0 function or exists / empty. Everything else (fn:matches, tokenize, xs:time …)
    ** is not supported and gives an error rather than a "silent" value during validation.
    */
    ExprPtr XPathParser::assertionCall(std::string name, std::vector<ExprPtr> arguments)
    {
        AtomicType type = AtomicType::STRING;

        if (constructorType(name, _namespaces, type))
        {
            if (arguments.size() != 1)
                fail(i18n::format(i18n::Message::XPATH_WRONG_ARGUMENT_COUNT, {name}));
            return std::make_unique<ConstructorExpr>(type, std::move(arguments.front()));
        }
        if (!isKnownFunction(name, arguments.size(), true))
            fail(i18n::format(i18n::Message::XPATH_ASSERTION_FUNCTION, {name}));
        return std::make_unique<FunctionCallExpr>(std::move(name), std::move(arguments));
    }
}
