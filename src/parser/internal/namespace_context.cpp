#include "parser/internal/namespace_context.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::detail
{
    // Префиксы xml и xmlns предопределены спецификацией Namespaces in XML
    NamespaceContext::NamespaceContext()
        : _bindings(), _scopes()
    {
        _bindings.push_back(Binding{"xml", std::string(XML_NAMESPACE_URI)});
        _bindings.push_back(Binding{"xmlns", std::string(XMLNS_NAMESPACE_URI)});
    }

    // Новая область запоминает, сколько связей было до неё
    void NamespaceContext::pushScope()
    {
        _scopes.push_back(_bindings.size());
    }

    // Удаляются все связи, объявленные в закрываемой области
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
    */
    void NamespaceContext::declare(std::string_view prefix, std::string_view uri)
    {
        bool isXml = prefix == "xml";

        if (prefix == "xmlns")
            throw DomException(DomErrorCode::NAMESPACE_ERROR, "префикс xmlns нельзя объявлять");
        if (isXml != (uri == XML_NAMESPACE_URI))
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                "префикс xml и его пространство имён нельзя переопределять");
        if (uri == XMLNS_NAMESPACE_URI)
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                "пространство имён xmlns нельзя связывать с префиксом");
        if (!prefix.empty() && uri.empty())
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                "префикс \"" + std::string(prefix) + "\" нельзя связать с пустым URI");
        _bindings.push_back(Binding{std::string(prefix), std::string(uri)});
    }

    // Поиск с конца: ближайшая область перекрывает внешние
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
