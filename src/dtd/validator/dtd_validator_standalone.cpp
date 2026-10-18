#include "dtd/dtd_validator.hpp"
#include "i18n/i18n.hpp"
#include "dtd/attribute_defaults.hpp"

namespace beresta::dtd
{
    namespace
    {
        // Указанный в теге атрибут с этим именем или nullptr
        // The attribute with this name specified in the tag or nullptr
        const Attribute* findAttribute(const std::vector<Attribute>& attributes, const std::string& name)
        {
            for (const Attribute& attribute : attributes)
            {
                if (attribute.name == name)
                    return &attribute;
            }
            return nullptr;
        }
    }

    /*
    ** VC: Standalone Document Declaration (раздел 2.9). Документ с
    ** standalone="yes" обязан давать тот же результат без внешнего DTD:
    **  - внешне объявленное умолчание не должно понадобиться (атрибут указан);
    **  - нормализация внешне объявленного не-CDATA атрибута не меняет значение.
    **
    ** VC: Standalone Document Declaration (section 2.9). A document with
    ** standalone="yes" must give the same result without the external DTD:
    **  - an externally declared default must not be needed (the attribute is specified);
    **  - normalization of an externally declared non-CDATA attribute does not change the value.
    */
    void DtdValidator::checkStandalone(std::string_view element, const std::vector<Attribute>& attributes)
    {
        const std::vector<AttributeDeclaration>* declarations = _model.findAttributes(element);

        if (declarations == nullptr)
            return;
        for (const AttributeDeclaration& declaration : *declarations)
        {
            const Attribute* present = findAttribute(attributes, declaration.name);
            bool hasDefault = declaration.defaultKind == DefaultKind::VALUE
                || declaration.defaultKind == DefaultKind::FIXED;

            if (!declaration.externallyDeclared)
                continue;
            if (present == nullptr && hasDefault)
                _reporter.reportValidity(i18n::format(i18n::Message::DTD_STANDALONE_DEFAULT, {declaration.name}));
            bool tokenized = declaration.type != AttributeType::CDATA;
            if (present != nullptr && tokenized && collapseWhitespace(present->value) != present->value)
                _reporter.reportValidity(i18n::format(i18n::Message::DTD_STANDALONE_NORMALIZATION, {declaration.name}));
        }
    }
}
