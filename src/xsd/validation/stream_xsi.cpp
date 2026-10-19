#include "xsd/internal/stream_validator.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Значения xsi:type (QName) и xsi:nil (boolean) сворачиваются по
    ** правилу whiteSpace="collapse" своих типов: пробелы по краям не
    ** значимы, "  xs:boolean  " — то же, что "xs:boolean".
    **
    ** The values of xsi:type (QName) and xsi:nil (boolean) are collapsed by
    ** the whiteSpace="collapse" rule of their types: the spaces at the edges are not
    ** significant, "  xs:boolean  " is the same as "xs:boolean".
    */
    static std::string_view trimXmlSpaces(std::string_view value) noexcept
    {
        while (!value.empty() && utils::isWhitespace(value.front()))
            value.remove_prefix(1);
        while (!value.empty() && utils::isWhitespace(value.back()))
            value.remove_suffix(1);
        return value;
    }

    // xsi:type и xsi:nil — одним проходом по атрибутам (их пространства имён уже разрешены)
    // xsi:type and xsi:nil — in one pass over the attributes (their namespaces are already resolved)
    XsiValues findXsi(const SaxElement& element) noexcept
    {
        XsiValues xsi;

        for (const SaxAttribute& attribute : element.attributes)
        {
            if (attribute.namespaceUri.size() != XSI_NAMESPACE_URI.size() || attribute.namespaceUri != XSI_NAMESPACE_URI)
                continue;
            if (!xsi.hasType && attribute.localName == "type")
            {
                xsi.type = trimXmlSpaces(attribute.value);
                xsi.hasType = true;
            }
            else if (!xsi.hasNil && attribute.localName == "nil")
            {
                xsi.nil = trimXmlSpaces(attribute.value);
                xsi.hasNil = true;
            }
        }
        return xsi;
    }

    /*
    ** Заглушка общая и неизменяемая (тип — xs:anyType, ни nillable, ни
    ** ограничений): действующий тип элементу даст applyXsiType, а любой
    ** тип выведен из xs:anyType. Разделение между потоками безопасно.
    **
    ** The stub is shared and immutable (the type is xs:anyType, neither nillable nor
    ** constraints): applyXsiType gives the element its effective type, and any
    ** type is derived from xs:anyType. Sharing it between threads is safe.
    */
    const ElementDecl* typeOnlyDeclaration(const SaxElement& element) noexcept
    {
        static const ElementDecl TYPE_ONLY;

        if (!findXsi(element).hasType)
            return nullptr;
        return &TYPE_ONLY;
    }
}
