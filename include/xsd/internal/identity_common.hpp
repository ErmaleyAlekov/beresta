#ifndef XSD_IDENTITY_COMMON_HPP
#define XSD_IDENTITY_COMMON_HPP

#include "core/element.hpp"
#include "xsd/schema.hpp"

#include <string>
#include <vector>

namespace beresta::xsd::detail
{
    // Значения полей ключа через запятую (для сообщений об ошибках): "A1, 2026"
    // The key field values separated by commas (for error messages): "A1, 2026"
    std::string joinKeyValues(const std::vector<std::string>& values);

    // Вид ограничения для сообщения о повторе: "xs:key" или "xs:unique"
    // The constraint kind for a repetition message: "xs:key" or "xs:unique"
    std::string uniquenessLabel(const IdentityConstraint& constraint);

    // Путь к ребёнку: /родитель/имя или /родитель/имя[k], если одноимённых соседей несколько
    // The path to a child: /parent/name or /parent/name[k] if there are several siblings with the same name
    std::string childPath(const std::string& parentPath, const Element& child);

    // Абсолютный путь элемента дерева вида /a/b[2]/c
    // The absolute path of a tree element like /a/b[2]/c
    std::string pathOf(const Element& element);

    // Следующий элемент в прямом порядке документа внутри поддерева root (nullptr — конец)
    // The next element in document order inside the subtree root (nullptr — the end)
    const Element* nextElement(const Element& current, const Element& root) noexcept;
}

#endif
