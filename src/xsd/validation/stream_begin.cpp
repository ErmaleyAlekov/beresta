#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    // xsi:type и xsi:nil — одним проходом по атрибутам (их пространства имён уже разрешены)
    // xsi:type and xsi:nil — in one pass over the attributes (their namespaces are already resolved)
    static XsiValues findXsi(const SaxElement& element) noexcept
    {
        XsiValues xsi;

        for (const SaxAttribute& attribute : element.attributes)
        {
            if (attribute.namespaceUri.size() != XSI_NAMESPACE_URI.size() || attribute.namespaceUri != XSI_NAMESPACE_URI)
                continue;
            if (!xsi.hasType && attribute.localName == "type")
            {
                xsi.type = attribute.value;
                xsi.hasType = true;
            }
            else if (!xsi.hasNil && attribute.localName == "nil")
            {
                xsi.nil = attribute.value;
                xsi.hasNil = true;
            }
        }
        return xsi;
    }

    /*
    ** Проверка элемента с объявлением — в том же порядке, что у обхода
    ** дерева: область ключей, xsi:type (ошибка — дальше не проверяется),
    ** abstract, xsi:nil (принятый — содержимое не проверяется), затем
    ** простой или составной тип. xs:anyType допускает что угодно.
    **
    ** Checking an element with a declaration — in the same order as the tree
    ** traversal: the key scope, xsi:type (an error — no further checks),
    ** abstract, xsi:nil (an accepted one — the content is not checked), then
    ** a simple or a complex type. xs:anyType allows anything.
    */
    void StreamValidator::beginValidation(const SaxElement& element, const ElementDecl& declaration)
    {
        ElementFrame& frame = _frames[_depth - 1];
        TypeRef type = declaration.type;
        XsiValues xsi = findXsi(element);

        frame.declaration = &declaration;
        frame.mode = FrameMode::OPAQUE;
        if (!declaration.identityConstraints.empty())
            openScope(declaration);
        if (_identity != nullptr)
            frame.attributeTypes.assign(element.attributes.size(), nullptr);
        if (!applyXsiType(xsi, type))
            return;
        if (declaration.isAbstract)
            report(_depth - 1, CheckPhase::ABSTRACT, i18n::format(i18n::Message::XSD_ABSTRACT_ELEMENT));
        if (isNil(xsi, declaration))
            frame.mode = FrameMode::NIL;
        else if (type.simple != nullptr)
            beginSimple(element, *type.simple);
        else if (type.complex != nullptr)
            beginComplex(element, *type.complex);
    }

    // xsi:type="p:Тип": префикс ищется среди объявлений открытых элементов, включая этот
    // xsi:type="p:Type": the prefix is looked up among the declarations of the open elements, including this one
    bool StreamValidator::applyXsiType(const XsiValues& xsi, TypeRef& type)
    {
        if (!xsi.hasType)
            return true;
        utils::QualifiedNameParts parts = utils::splitQualifiedName(xsi.type);
        std::string_view namespaceUri = _prefixes.lookup(parts.prefix).value_or(std::string_view());
        bool found = false;
        TypeRef replacement = _schema.findType(namespaceUri, parts.localName, found);
        if (!found)
        {
            report(_depth - 1, CheckPhase::XSI_TYPE, i18n::format(i18n::Message::XSD_XSI_TYPE_NOT_FOUND, {xsi.type}));
            return false;
        }
        type = replacement;
        return true;
    }

    // xsi:nil="true" допустим только у nillable-элементов; пустоту содержимого проверит закрытие
    // xsi:nil="true" is allowed only for nillable elements; the emptiness of the content is checked on closing
    bool StreamValidator::isNil(const XsiValues& xsi, const ElementDecl& declaration)
    {
        if (!xsi.hasNil || (xsi.nil != "true" && xsi.nil != "1"))
            return false;
        if (!declaration.nillable)
        {
            report(_depth - 1, CheckPhase::NIL, i18n::format(i18n::Message::XSD_NOT_NILLABLE));
            return false;
        }
        return true;
    }

    // Составной тип: атрибуты сразу, затем — простое содержимое (текст копится) или курсор модели
    // A complex type: the attributes at once, then — simple content (the text is accumulated) or the model cursor
    void StreamValidator::beginComplex(const SaxElement& element, const ComplexType& type)
    {
        ElementFrame& frame = _frames[_depth - 1];

        frame.mode = FrameMode::COMPLEX;
        frame.complex = &type;
        validateAttributes(element, type);
        if (type.content == ContentType::SIMPLE_CONTENT)
        {
            frame.valueType = type.simpleContent;
            frame.collectText = true;
            return;
        }
        if (!type.hasParticle)
            return;
        frame.plan = planFor(type.particle);
        frame.cursor.reset(type.particle, frame.plan);
    }
}
