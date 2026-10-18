#ifndef XSD_IDENTITY_PATH_HPP
#define XSD_IDENTITY_PATH_HPP

#include "xpath/xpath.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xsd::detail
{
    // Вид проверки имени в шаге пути
    // The kind of name check in a path step
    enum class NameTestKind
    {
        ANY,            // *
        NAMESPACE_ANY,  // p:* — любое имя в пространстве p / any name in the namespace p
        QUALIFIED       // имя без префикса (вне пространств) или p:имя / a name without a prefix (no namespace) or p:name
    };

    // Проверка имени: префикс уже заменён на URI по объявлениям документа схемы
    // A name check: the prefix is already replaced with a URI by the declarations of the schema document
    struct NameTest
    {
        NameTestKind kind = NameTestKind::ANY;
        std::string namespaceUri;
        std::string localName;
    };

    /*
    ** Путь из грамматики селекторов и полей XSD (раздел 3.11.6): по желанию
    ** ".//" в начале (descendant-or-self), затем шаги по детям; у поля в
    ** конце может стоять атрибут. Шаги "." (сам узел) выброшены при разборе.
    **
    ** A path from the grammar of XSD selectors and fields (section 3.11.6): optionally
    ** ".//" at the start (descendant-or-self), then steps over children; a field may
    ** have an attribute at the end. The "." steps (the node itself) are dropped while parsing.
    */
    struct StreamPath
    {
        bool descendant = false;
        std::vector<NameTest> steps;
        bool attribute = false;
        NameTest attributeTest;
    };

    /*
    ** Разбор выражения xs:selector / xs:field в варианты, разделённые "|".
    ** false — выражение выходит за грамматику XSD (например, использует
    ** предикаты или функции): такое ограничение потоково не проверить.
    **
    ** Parsing an xs:selector / xs:field expression into the alternatives separated by "|".
    ** false — the expression goes beyond the XSD grammar (for example, it uses
    ** predicates or functions): such a constraint cannot be checked in a stream.
    */
    bool parseStreamPaths(std::string_view text, const xpath::XPathContext& context,
        std::vector<StreamPath>& paths);

    /*
    ** PathReader — разбор одного выражения методом рекурсивного спуска по
    ** грамматике XSD (без рекурсии: грамматика плоская). Префиксы имён
    ** разрешаются по объявлениям xmlns документа схемы (context.namespaces).
    **
    ** PathReader — parsing one expression by recursive descent over the
    ** XSD grammar (without recursion: the grammar is flat). Name prefixes
    ** are resolved by the xmlns declarations of the schema document (context.namespaces).
    */
    class PathReader
    {
    public:
        PathReader(std::string_view text, const xpath::XPathContext& context) noexcept;

        // Все варианты через "|" до конца текста
        // All alternatives through "|" up to the end of the text
        bool readAll(std::vector<StreamPath>& paths);

    private:
        // ---- Пути и шаги (identity_path_reader.cpp) ----
        // ---- Paths and steps (identity_path_reader.cpp) ----
        bool readPath(StreamPath& path);
        bool readStep(StreamPath& path, bool& attribute);
        bool consume(std::string_view token) noexcept;

        // ---- Имена (identity_path_names.cpp) ----
        // ---- Names (identity_path_names.cpp) ----
        bool readNameTest(NameTest& test);
        bool readName(std::string_view& name) noexcept;
        void skipSpaces() noexcept;

        std::string_view _text;
        std::size_t _position;
        const xpath::XPathContext& _context;
    };

    // Подходит ли имя (пространство имён + локальное имя) под проверку
    // Whether a name (namespace + local name) fits the check
    bool nameMatches(const NameTest& test, std::string_view namespaceUri, std::string_view localName) noexcept;

    /*
    ** Имя открытого элемента для сопоставления путей. Хранится у каждого
    ** уровня стека; строки переиспользуются от прошлых элементов уровня.
    **
    ** The name of an open element for matching paths. It is kept at each
    ** level of the stack; the strings are reused from the previous elements of the level.
    */
    struct LevelName
    {
        std::string namespaceUri;
        std::string localName;
    };

    // Выбирает ли элементную часть пути элемент уровня level от узла-якоря уровня anchor
    // Whether the element part of the path selects the element of the level level from the anchor node of the level anchor
    bool pathReaches(const StreamPath& path, const std::vector<LevelName>& names, std::size_t anchor,
        std::size_t level) noexcept;
}

#endif
