#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    // Сколько объявлений атрибутов помещается в битовую маску seen
    // How many attribute declarations fit into the seen bit mask
    static const std::size_t USE_MASK_BITS = 64;

    // Номер объявления атрибута по паре (пространство имён, локальное имя); size() — нет такого
    // The number of the attribute declaration by the pair (namespace, local name); size() means none
    static std::size_t findAttributeUse(const ComplexType& type, const SaxAttribute& attribute) noexcept
    {
        std::size_t found = type.attributes.size();

        for (std::size_t index = 0; index < type.attributes.size(); ++index)
        {
            const AttributeUse& candidate = type.attributes[index];
            if (candidate.name == attribute.localName && candidate.namespaceUri == attribute.namespaceUri)
                found = index;
        }
        return found;
    }

    /*
    ** Каждый атрибут экземпляра должен быть объявлен в типе (или допущен
    ** xs:anyAttribute). Объявления xmlns и атрибуты xsi:* — служебные.
    **
    ** Every instance attribute must be declared in the type (or admitted by
    ** xs:anyAttribute). xmlns declarations and xsi:* attributes are service ones.
    */
    void StreamValidator::validateAttributes(const SaxElement& element, const ComplexType& type)
    {
        std::uint64_t seen = 0;
        bool declares = _frames[_depth - 1].declaresNamespaces;

        for (std::size_t position = 0; position < element.attributes.size(); ++position)
        {
            const SaxAttribute& attribute = element.attributes[position];
            if ((declares && utils::isNamespaceDeclaration(attribute.name)) || attribute.namespaceUri == XSI_NAMESPACE_URI)
                continue;
            std::size_t index = findAttributeUse(type, attribute);
            const AttributeUse* use = nullptr;
            if (index < type.attributes.size())
                use = &type.attributes[index];
            if (use != nullptr && index < USE_MASK_BITS)
                seen |= std::uint64_t(1) << index;
            if (use == nullptr && !type.anyAttribute)
                report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_ATTRIBUTE_UNDECLARED,
                    {attribute.name}));
            else if (use != nullptr && use->prohibited)
                report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_ATTRIBUTE_PROHIBITED,
                    {attribute.name}));
            else if (use != nullptr)
                checkAttribute(*use, attribute, position);
        }
        checkRequiredAttributes(element, type, seen);
    }

    // Значение атрибута: тип (запоминается для ключей), учёт ID/IDREF, фиксированное значение
    // An attribute value: the type (remembered for keys), ID/IDREF accounting, the fixed value
    void StreamValidator::checkAttribute(const AttributeUse& use, const SaxAttribute& attribute, std::size_t index)
    {
        std::optional<std::string> error = validateSimpleValue(*use.type, attribute.value, _prefixes);
        ElementFrame& frame = _frames[_depth - 1];

        if (error.has_value())
        {
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_ATTRIBUTE_VALUE,
                {use.name, error.value()}));
            return;
        }
        if (index < frame.attributeTypes.size())
            frame.attributeTypes[index] = use.type;
        recordIdentity(*use.type, attribute.value, CheckPhase::ATTRIBUTES);
        if (use.fixedValue.has_value() && !valuesEqual(*use.type, attribute.value, use.fixedValue.value()))
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_ATTRIBUTE_FIXED,
                {use.name, use.fixedValue.value()}));
    }

    /*
    ** Обязательные атрибуты (use="required"). Встреченные объявления
    ** отмечены битами seen; объявления дальше 64-го (такое бывает разве что
    ** в сгенерированных схемах) ищутся перебором атрибутов.
    **
    ** Required attributes (use="required"). The met declarations
    ** are marked with the bits of seen; declarations past the 64th (this happens
    ** perhaps only in generated schemas) are searched by a loop over the attributes.
    */
    void StreamValidator::checkRequiredAttributes(const SaxElement& element, const ComplexType& type,
        std::uint64_t seen)
    {
        for (std::size_t index = 0; index < type.attributes.size(); ++index)
        {
            const AttributeUse& use = type.attributes[index];
            bool present = false;

            if (!use.required)
                continue;
            if (index < USE_MASK_BITS)
                present = ((seen >> index) & 1u) != 0;
            for (std::size_t other = 0; index >= USE_MASK_BITS && other < element.attributes.size(); ++other)
            {
                const SaxAttribute& attribute = element.attributes[other];
                if (attribute.localName == use.name && attribute.namespaceUri == use.namespaceUri)
                    present = true;
            }
            if (!present)
                report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::ATTRIBUTE_REQUIRED_MISSING,
                    {use.name}));
        }
    }

    /*
    ** Простой тип: атрибутов (кроме служебных) быть не должно. Сообщать о
    ** них сразу нельзя: если у элемента найдутся дети-элементы, обход
    ** дерева сообщал только о детях. Поэтому имена откладываются до закрытия.
    **
    ** A simple type: there must be no attributes (except the service ones). Reporting
    ** them at once is not possible: if the element turns out to have child elements, the tree
    ** traversal reported only the children. So the names are deferred until closing.
    */
    void StreamValidator::beginSimple(const SaxElement& element, const SimpleType& type)
    {
        ElementFrame& frame = _frames[_depth - 1];
        bool declares = frame.declaresNamespaces;

        frame.mode = FrameMode::SIMPLE;
        frame.valueType = &type;
        frame.collectText = true;
        for (const SaxAttribute& attribute : element.attributes)
        {
            if ((declares && utils::isNamespaceDeclaration(attribute.name)) || attribute.namespaceUri == XSI_NAMESPACE_URI)
                continue;
            if (frame.pendingCount == frame.pendingAttributes.size())
                frame.pendingAttributes.emplace_back();
            frame.pendingAttributes[frame.pendingCount++].assign(attribute.name.data(), attribute.name.size());
        }
    }
}
