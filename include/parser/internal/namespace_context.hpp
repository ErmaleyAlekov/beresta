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
    */
    class NamespaceContext
    {
    public:
        NamespaceContext();

        void pushScope();
        void popScope();

        // Объявление в текущей области; нарушения правил — DomException(NAMESPACE_ERROR)
        void declare(std::string_view prefix, std::string_view uri);

        // URI префикса (пустой — пространство по умолчанию) или nullptr, если не связан
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
