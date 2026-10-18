#include "parser/internal/namespace_context.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::detail
{
    // Префиксы xml и xmlns предопределены спецификацией Namespaces in XML
    // The xml and xmlns prefixes are predefined by the Namespaces in XML specification
    NamespaceContext::NamespaceContext()
        : _bindings(), _scopes()
    {
        _bindings.push_back(Binding{"xml", std::string(XML_NAMESPACE_URI)});
        _bindings.push_back(Binding{"xmlns", std::string(XMLNS_NAMESPACE_URI)});
    }

    // Новая область запоминает, сколько связей было до неё
    // A new scope remembers how many bindings there were before it
    void NamespaceContext::pushScope()
    {
        _scopes.push_back(_bindings.size());
    }

    // Удаляются все связи, объявленные в закрываемой области
    // All bindings declared in the closed scope are removed
    void NamespaceContext::popScope()
    {
        if (_scopes.empty())
            return;
        _bindings.resize(_scopes.back());
        _scopes.pop_back();
    }

    /*
    ** Правила объявлений:
    ** - префикс xmlns объявлять нельзя; xml — только с его родным URI;
    ** - родные URI xml/xmlns нельзя связывать с другими префиксами;
    ** - префикс нельзя «отвязать» пустым URI (это допустимо только для xmlns="").
    **
    ** Declaration rules:
    **  - the xmlns prefix cannot be declared; xml — only with its own URI;
    **  - the own URIs of xml/xmlns cannot be bound to other prefixes;
    **  - a prefix cannot be "unbound" with an empty URI (this is allowed only for xmlns="").
    */
    void NamespaceContext::declare(std::string_view prefix, std::string_view uri)
    {
        bool isXml = prefix == "xml";

        if (prefix == "xmlns")
            throw DomException(DomErrorCode::NAMESPACE_ERROR, i18n::format(i18n::Message::PREFIX_XMLNS_DECLARED));
        if (isXml != (uri == XML_NAMESPACE_URI))
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::PREFIX_XML_REDEFINED));
        if (uri == XMLNS_NAMESPACE_URI)
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::XMLNS_URI_BOUND));
        if (!prefix.empty() && uri.empty())
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::PREFIX_EMPTY_URI, {prefix}));
        _bindings.push_back(Binding{std::string(prefix), std::string(uri)});
    }

    // Поиск с конца: ближайшая область перекрывает внешние
    // The lookup from the end: the nearest scope overrides the outer ones
    const std::string* NamespaceContext::resolve(std::string_view prefix) const noexcept
    {
        for (std::size_t index = _bindings.size(); index > 0; --index)
        {
            const Binding& binding = _bindings[index - 1];
            if (binding.prefix != prefix)
                continue;
            if (binding.uri.empty())
                return nullptr;
            return &binding.uri;
        }
        return nullptr;
    }
}
