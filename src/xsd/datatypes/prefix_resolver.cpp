#include "xsd/prefix_resolver.hpp"

namespace beresta::xsd
{
    // Виртуальный деструктор интерфейса
    // The virtual destructor of the interface
    PrefixResolver::~PrefixResolver()
    {
    }

    // Резолвер только запоминает элемент: поиск — при каждом запросе
    // The resolver only remembers the element: the lookup is done on each request
    ElementPrefixResolver::ElementPrefixResolver(const Element* element) noexcept
        : _element(element)
    {
    }

    /*
    ** Без элемента известны лишь встроенные префиксы — так раньше вела себя
    ** проверка QName с контекстом nullptr (префикс считался необъявленным).
    **
    ** Without an element only the built-in prefixes are known — this is how the
    ** QName check with the nullptr context behaved before (the prefix was considered undeclared).
    */
    bool ElementPrefixResolver::isBound(std::string_view prefix) const
    {
        if (_element == nullptr)
            return false;
        return _element->lookupNamespaceURI(prefix).has_value();
    }
}
