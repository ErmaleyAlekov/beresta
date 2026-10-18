#include "xsd/internal/identity_path.hpp"

namespace beresta::xsd::detail
{
    // Разбор выражения целиком; при неудаче список вариантов пуст
    // Parsing the whole expression; on a failure the list of alternatives is empty
    bool parseStreamPaths(std::string_view text, const xpath::XPathContext& context,
        std::vector<StreamPath>& paths)
    {
        PathReader reader(text, context);

        paths.clear();
        if (reader.readAll(paths))
            return true;
        paths.clear();
        return false;
    }

    // '*' — любое имя; 'p:*' — любое имя в пространстве p; иначе — точное совпадение пары
    // '*' — any name; 'p:*' — any name in the namespace p; otherwise — an exact match of the pair
    bool nameMatches(const NameTest& test, std::string_view namespaceUri, std::string_view localName) noexcept
    {
        if (test.kind == NameTestKind::ANY)
            return true;
        if (test.kind == NameTestKind::NAMESPACE_ANY)
            return test.namespaceUri == namespaceUri;
        return test.namespaceUri == namespaceUri && test.localName == localName;
    }

    /*
    ** Шаги пути сверяются с именами уровней снизу вверх: последний шаг — с
    ** самим элементом, предпоследний — с его родителем и т. д. Без ".//"
    ** число шагов обязано равняться расстоянию от якоря; с ".//" путь
    ** может начинаться на любой глубине под якорем (или на нём самом).
    **
    ** The path steps are matched against the level names bottom-up: the last step — against
    ** the element itself, the one before — against its parent and so on. Without ".//"
    ** the number of steps must equal the distance from the anchor; with ".//" the path
    ** may start at any depth below the anchor (or at the anchor itself).
    */
    bool pathReaches(const StreamPath& path, const std::vector<LevelName>& names, std::size_t anchor,
        std::size_t level) noexcept
    {
        std::size_t count = path.steps.size();

        if (level < anchor || count > level - anchor)
            return false;
        if (!path.descendant && count != level - anchor)
            return false;
        for (std::size_t offset = 0; offset < count; ++offset)
        {
            const LevelName& name = names[level - offset];
            if (!nameMatches(path.steps[count - 1 - offset], name.namespaceUri, name.localName))
                return false;
        }
        return true;
    }
}
