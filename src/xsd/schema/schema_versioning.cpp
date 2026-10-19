#include "xsd/internal/schema_builder.hpp"
#include "xsd/internal/datatype_checks.hpp"
#include "utils/decimal.hpp"

namespace beresta::xsd::detail
{
    // Пространство имён условного включения XSD 1.1 (атрибуты vc:minVersion / vc:maxVersion)
    // The XSD 1.1 conditional inclusion namespace (the vc:minVersion / vc:maxVersion attributes)
    static const char* const VERSIONING_NAMESPACE = "http://www.w3.org/2007/XMLSchema-versioning";

    // Версия языка XML Schema, которую «Берёста» объявляет для условного включения
    // The XML Schema language version that Beresta declares for conditional inclusion
    static const char* const PROCESSOR_VERSION = "1.1";

    /*
    ** Сравнение атрибута-версии с версией процессора: -1, 0, 1 или 2, если
    ** атрибута нет (или он не число xs:decimal — тогда он не учитывается).
    ** Версии сравниваются точно, как десятичные числа ("1.10" = "1.1").
    **
    ** Comparing a version attribute with the processor version: -1, 0, 1 or 2 if
    ** there is no attribute (or it is not an xs:decimal number — then it is ignored).
    ** Versions are compared exactly, as decimal numbers ("1.10" = "1.1").
    */
    static int compareVersion(const Element& element, std::string_view localName)
    {
        std::optional<std::string> raw = element.getAttributeNS(VERSIONING_NAMESPACE, localName);

        if (!raw.has_value())
            return 2;
        std::string value = normalizeWhiteSpace(raw.value(), WhiteSpace::COLLAPSE);
        if (!isDecimalLexical(value, false))
            return 2;
        return utils::compareDecimalText(value, PROCESSOR_VERSION);
    }

    // Включается ли компонент: minVersion <= 1.1 < maxVersion (отсутствующая граница не мешает)
    // Whether the component is included: minVersion <= 1.1 < maxVersion (an absent bound does not interfere)
    bool versionAccepted(const Element& element)
    {
        int minimum = compareVersion(element, "minVersion");
        int maximum = compareVersion(element, "maxVersion");

        if (minimum == 1)
            return false;
        return maximum == 2 || maximum == 1;
    }
}
