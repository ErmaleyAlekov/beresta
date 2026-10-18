#include "xsd/internal/validation_run.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    // Сколько объявлений атрибутов помещается в битовую маску seen
    // How many attribute declarations fit into the seen bit mask
    static const std::size_t USE_MASK_BITS = 64;

    // Номер объявления атрибута в типе по паре (пространство имён, локальное имя); size() — нет такого
    // The number of the attribute declaration in the type by the pair (namespace, local name); size() means none
    static std::size_t findAttributeUse(const ComplexType& type,
        const std::pair<std::string, std::string_view>& key)
    {
        std::size_t found = type.attributes.size();

        for (std::size_t index = 0; index < type.attributes.size(); ++index)
        {
            const AttributeUse& candidate = type.attributes[index];
            if (candidate.name == key.second && candidate.namespaceUri == key.first)
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
    void ValidationRun::validateAttributes(const Element& element, const ComplexType& type,
        std::size_t path)
    {
        std::uint64_t seen = 0;

        for (const Attribute& attribute : element.getAttributes())
        {
            if (utils::isNamespaceDeclaration(attribute.name))
                continue;
            std::pair<std::string, std::string_view> key = attributeKey(element, attribute);
            if (key.first == XSI_NAMESPACE_URI)
                continue;
            std::size_t index = findAttributeUse(type, key);
            const AttributeUse* use = nullptr;
            if (index < type.attributes.size())
                use = &type.attributes[index];
            if (use != nullptr && index < USE_MASK_BITS)
                seen |= std::uint64_t(1) << index;
            if (use == nullptr && !type.anyAttribute)
                report(element, path, i18n::format(i18n::Message::XSD_ATTRIBUTE_UNDECLARED, {attribute.name}));
            else if (use != nullptr && use->prohibited)
                report(element, path, i18n::format(i18n::Message::XSD_ATTRIBUTE_PROHIBITED, {attribute.name}));
            else if (use != nullptr)
                checkAttribute(element, *use, attribute, path);
        }
        checkRequiredAttributes(element, type, path, seen);
    }

    // Значение атрибута: тип (запоминается для ключей), учёт ID/IDREF, фиксированное значение
    // An attribute value: the type (remembered for keys), ID/IDREF accounting, the fixed value
    void ValidationRun::checkAttribute(const Element& element, const AttributeUse& use,
        const Attribute& attribute, std::size_t path)
    {
        const std::string& value = attribute.value;
        std::optional<std::string> error = validateSimpleValue(*use.type, value, &element);

        if (error.has_value())
        {
            report(element, path, i18n::format(i18n::Message::XSD_ATTRIBUTE_VALUE, {use.name, error.value()}));
            return;
        }
        if (!_scopes.empty())
            _attributeTypes[&attribute] = use.type;
        recordIdentity(*use.type, value, element, path);
        if (use.fixedValue.has_value() && !valuesEqual(*use.type, value, use.fixedValue.value()))
            report(element, path, i18n::format(i18n::Message::XSD_ATTRIBUTE_FIXED, {use.name, use.fixedValue.value()}));
    }

    /*
    ** Обязательные атрибуты (use="required"). Какие объявления уже
    ** встретились, validateAttributes отметила битами seen — повторно искать
    ** атрибуты по пространству имён не нужно. Объявления дальше 64-го (такое
    ** бывает разве что в сгенерированных схемах) ищутся по-старому.
    **
    ** Required attributes (use="required"). Which declarations were already
    ** met, validateAttributes marked with the bits of seen — there is no need to search
    ** the attributes by namespace again. Declarations past the 64th (this
    ** happens perhaps only in generated schemas) are searched the old way.
    */
    void ValidationRun::checkRequiredAttributes(const Element& element, const ComplexType& type,
        std::size_t path, std::uint64_t seen)
    {
        for (std::size_t index = 0; index < type.attributes.size(); ++index)
        {
            const AttributeUse& use = type.attributes[index];
            bool present = false;

            if (!use.required)
                continue;
            if (index < USE_MASK_BITS)
                present = ((seen >> index) & 1u) != 0;
            else
                present = element.hasAttributeNS(use.namespaceUri, use.name);
            if (!present)
                report(element, path, i18n::format(i18n::Message::ATTRIBUTE_REQUIRED_MISSING, {use.name}));
        }
    }

    // Атрибут без префикса не принадлежит пространству имён; с префиксом — по объявлению xmlns
    // An attribute without a prefix belongs to no namespace; with a prefix — by the xmlns declaration
    std::pair<std::string, std::string_view> attributeKey(const Element& element, const Attribute& attribute)
    {
        utils::QualifiedNameParts parts = utils::splitQualifiedName(attribute.name);

        if (parts.prefix.empty())
            return std::pair<std::string, std::string_view>(std::string(), parts.localName);
        return std::pair<std::string, std::string_view>(element.lookupNamespaceURI(parts.prefix).value_or(""),
            parts.localName);
    }
}
