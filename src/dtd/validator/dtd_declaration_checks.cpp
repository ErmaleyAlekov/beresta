#include "dtd/dtd_declaration_checks.hpp"
#include "i18n/i18n.hpp"

#include <set>

namespace beresta::dtd
{
    namespace
    {
        // VC: No Duplicate Types — в (#PCDATA | a | b)* каждое имя встречается один раз
        // VC: No Duplicate Types — in (#PCDATA | a | b)* each name occurs once
        void checkMixedContent(const ElementDeclaration& declaration, ValidityReporter& reporter)
        {
            std::set<std::string, std::less<>> seen;

            if (declaration.kind != ContentKind::MIXED)
                return;
            for (const std::string& name : declaration.mixedNames)
            {
                if (!seen.insert(name).second)
                    reporter.reportValidity(i18n::format(i18n::Message::DTD_MIXED_DUPLICATE, {name, declaration.name}));
            }
        }

        // VC: Notation Declared — NDATA ссылается на объявленную нотацию
        // VC: Notation Declared — NDATA refers to a declared notation
        void checkUnparsedEntities(const DtdModel& model, ValidityReporter& reporter)
        {
            for (const auto& entry : model.getEntities())
            {
                const EntityDeclaration& entity = entry.second;

                if (!entity.notation.empty() && !model.hasNotation(entity.notation))
                    reporter.reportValidity(i18n::format(i18n::Message::DTD_NOTATION_UNDECLARED_ENTITY, {entity.notation, entity.name}));
            }
        }
    }

    // Сначала замечания разбора, затем элементы, списки атрибутов и сущности
    // First the parsing notes, then the elements, the attribute lists and the entities
    void checkDeclarations(const DtdModel& model, ValidityReporter& reporter)
    {
        for (const std::string& note : model.getValidityNotes())
            reporter.reportValidity(note);
        for (const auto& entry : model.getElements())
            checkMixedContent(entry.second, reporter);
        for (const auto& entry : model.getAttributeLists())
            checkAttributeList(model, entry.first, entry.second, reporter);
        checkUnparsedEntities(model, reporter);
    }
}
