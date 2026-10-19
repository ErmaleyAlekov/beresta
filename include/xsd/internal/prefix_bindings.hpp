#ifndef XSD_PREFIX_BINDINGS_HPP
#define XSD_PREFIX_BINDINGS_HPP

#include "sax/sax_types.hpp"
#include "xsd/prefix_resolver.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xsd::detail
{
    // Объявление префикса из атрибута xmlns / xmlns:p (свои строки: представления SAX временны)
    // A prefix declaration from an xmlns / xmlns:p attribute (own strings: SAX views are temporary)
    struct PrefixBinding
    {
        std::string prefix;
        std::string namespaceUri;
    };

    /*
    ** PrefixBindings — стек объявлений префиксов открытых элементов.
    ** Нужен только значениям, которые сами содержат префиксы (xsi:type,
    ** значения типа QName): пространства имён элементов и атрибутов уже
    ** разрешил парсер. Поиск — с вершины стека вниз, как подъём к корню
    ** в дереве; xmlns:p="" отменяет объявление. Объекты объявлений
    ** переиспользуются, и элемент без xmlns стоит одну запись отметки.
    **
    ** PrefixBindings — the stack of prefix declarations of the open elements.
    ** It is needed only by values that contain prefixes themselves (xsi:type,
    ** values of the QName type): the namespaces of elements and attributes have already
    ** been resolved by the parser. The lookup goes from the top of the stack down, like climbing to the root
    ** in a tree; xmlns:p="" cancels a declaration. Declaration objects
    ** are reused, and an element without xmlns costs one mark record.
    */
    class PrefixBindings : public PrefixResolver
    {
    public:
        PrefixBindings();

        // Объявления открываемого элемента (из его атрибутов xmlns*); true — такие атрибуты были
        // The declarations of the element being opened (from its xmlns* attributes); true — there were such attributes
        bool push(const std::vector<SaxAttribute>& attributes);

        // Закрытие элемента: его объявления больше не действуют
        // Closing an element: its declarations are no longer in effect
        void pop();

        // URI префикса ("" — пространство по умолчанию); nullopt — префикс не объявлен
        // The URI of a prefix ("" — the default namespace); nullopt — the prefix is not declared
        std::optional<std::string_view> lookup(std::string_view prefix) const;

        bool isBound(std::string_view prefix) const override;

        // Действующие объявления (ближайшее для каждого префикса; prefix_bindings_scope.cpp)
        // The declarations in effect (the nearest one for each prefix; prefix_bindings_scope.cpp)
        std::vector<PrefixBinding> inScope() const;

    private:
        std::vector<PrefixBinding> _bindings;
        std::size_t _used;
        std::vector<std::size_t> _marks;
    };
}

#endif
