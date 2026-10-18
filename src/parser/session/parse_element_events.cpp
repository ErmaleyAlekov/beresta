#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "dtd/attribute_defaults.hpp"
#include "utils/qualified_name.hpp"

#include <utility>

namespace beresta::detail
{
    /*
    ** Атрибуты из DTD: сначала (для standalone="yes") проверка по исходным
    ** значениям, затем нормализация не-CDATA типов и подстановка умолчаний.
    **
    ** Attributes from the DTD: first (for standalone="yes") the check against the original
    ** values, then normalization of non-CDATA types and substitution of defaults.
    */
    void ParseSession::prepareAttributes(const std::string& name, std::vector<Attribute>& attributes)
    {
        if (!_dtd)
            return;
        if (_validator && _standalone)
            _validator->checkStandalone(name, attributes);
        dtd::applyAttributeDeclarations(*_dtd, name, attributes, _options.applyDtdDefaults);
    }

    /*
    ** Событие открытия элемента. Порядок: проверка единственного корня,
    ** атрибуты по умолчанию из DTD, пространства имён, валидатор и,
    ** наконец, обработчик. Пустой элемент сразу же и закрывается.
    **
    ** The element start event. The order: the single root check,
    ** the default attributes from the DTD, namespaces, the validator and,
    ** finally, the handler. An empty element is closed right away.
    */
    void ParseSession::emitStartElement(std::string name, std::vector<Attribute>& attributes,
        bool isEmpty)
    {
        markEvent();
        if (_openElements.empty() && _rootSeen)
            _inputs.current().failAt(i18n::format(i18n::Message::SECOND_ROOT),
                _tokenStart);
        if (!_rootSeen && _options.validateDtd && !_validator)
            reportValidity(i18n::format(i18n::Message::NO_DOCTYPE_FOR_VALIDATION));
        _rootSeen = true;
        prepareAttributes(name, attributes);
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
    // The end event: the validator, the handler, closing the namespace scope
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
    // Without namespace support the local name is the same as the full one
    void ParseSession::buildPlainAttributes(const std::vector<Attribute>& attributes)
    {
        _saxAttributes.clear();
        for (const Attribute& attribute : attributes)
            _saxAttributes.push_back(SaxAttribute{attribute.name, attribute.value,
                std::string_view(), attribute.name});
    }
}
