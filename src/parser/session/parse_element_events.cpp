#include "parser/parse_session.hpp"
#include "dtd/attribute_defaults.hpp"
#include "utils/qualified_name.hpp"

#include <utility>

namespace beresta::detail
{
    /*
    ** Событие открытия элемента. Порядок: проверка единственного корня,
    ** атрибуты по умолчанию из DTD, пространства имён, валидатор и,
    ** наконец, обработчик. Пустой элемент сразу же и закрывается.
    */
    void ParseSession::emitStartElement(std::string name, std::vector<Attribute>& attributes,
        bool isEmpty)
    {
        markEvent();
        if (_openElements.empty() && _rootSeen)
            _inputs.current().failAt("документ может содержать только один корневой элемент",
                _tokenStart);
        if (!_rootSeen && _options.validateDtd && !_validator)
            reportValidity("документ не содержит DOCTYPE, проверка по DTD невозможна");
        _rootSeen = true;
        if (_dtd)
            dtd::applyAttributeDeclarations(*_dtd, name, attributes, _options.applyDtdDefaults);
        OpenElement element{std::move(name), std::string()};
        if (_options.namespaceAware)
            processNamespaces(element, attributes);
        else
            buildPlainAttributes(attributes);
        if (_validator)
            _validator->startElement(element.name, attributes);
        std::string_view localName = element.name;
        if (_options.namespaceAware)
            localName = utils::splitQualifiedName(element.name).localName;
        _handler.startElement(SaxElement{element.name, element.namespaceUri, localName, _saxAttributes});
        _openElements.push_back(std::move(element));
        if (isEmpty)
            emitEndElement();
    }

    // Событие закрытия: валидатор, обработчик, закрытие области пространств имён
    void ParseSession::emitEndElement()
    {
        static const std::vector<SaxAttribute> NO_ATTRIBUTES;
        const OpenElement& element = _openElements.back();
        std::string_view localName = element.name;

        markEvent();
        if (_validator)
            _validator->endElement();
        if (_options.namespaceAware)
            localName = utils::splitQualifiedName(element.name).localName;
        _handler.endElement(SaxElement{element.name, element.namespaceUri, localName, NO_ATTRIBUTES});
        if (_options.namespaceAware)
            _namespaces.popScope();
        _openElements.pop_back();
    }

    // Без поддержки пространств имён локальное имя совпадает с полным
    void ParseSession::buildPlainAttributes(const std::vector<Attribute>& attributes)
    {
        _saxAttributes.clear();
        for (const Attribute& attribute : attributes)
            _saxAttributes.push_back(SaxAttribute{attribute.name, attribute.value,
                std::string_view(), attribute.name});
    }
}
