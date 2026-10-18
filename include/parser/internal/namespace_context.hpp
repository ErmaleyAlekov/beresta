#ifndef NAMESPACE_CONTEXT_HPP
#define NAMESPACE_CONTEXT_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::detail
{
    /*
    ** NamespaceContext — стек областей видимости префиксов при разборе.
    ** Каждый открывающий тег открывает область (pushScope), объявления
    ** xmlns добавляются в неё, закрывающий тег удаляет её (popScope).
    ** Поиск идёт с конца, поэтому ближайшее объявление перекрывает внешние.
    ** Префиксы xml и xmlns связаны со своими URI изначально и навсегда.
    **
    ** NamespaceContext — a stack of prefix scopes during parsing.
    ** Each start tag opens a scope (pushScope), xmlns declarations
    ** are added to it, the end tag removes it (popScope).
    ** The lookup goes from the end, so the nearest declaration overrides the outer ones.
    ** The xml and xmlns prefixes are bound to their URIs from the start and forever.
    */
    class NamespaceContext
    {
    public:
        NamespaceContext();

        void pushScope();
        void popScope();

        // Объявление в текущей области; нарушения правил — DomException(NAMESPACE_ERROR)
        // A declaration in the current scope; rule violations — DomException(NAMESPACE_ERROR)
        void declare(std::string_view prefix, std::string_view uri);

        // URI префикса (пустой — пространство по умолчанию) или nullptr, если не связан
        // The URI of a prefix (empty — the default namespace) or nullptr if it is not bound
        const std::string* resolve(std::string_view prefix) const noexcept;

    private:
        struct Binding
        {
            std::string prefix;
            std::string uri;
        };

        std::vector<Binding> _bindings;
        std::vector<std::size_t> _scopes;
    };
}

#endif
