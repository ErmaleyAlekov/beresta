#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"
#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Значение текста элемента: пустой текст заменяется значением default
    ** (или fixed); затем проверка типа, учёт ID/IDREF и сравнение с fixed
    ** по значению. Префиксы QName — из объявлений открытых элементов.
    **
    ** The value of the element text: an empty text is replaced with the default value
    ** (or fixed); then the type check, ID/IDREF accounting and the comparison with fixed
    ** by value. QName prefixes come from the declarations of the open elements.
    */
    const SimpleType* StreamValidator::checkValue(const SimpleType& type, std::string_view text, CheckPhase phase)
    {
        const ElementDecl* declaration = _frames[_depth - 1].declaration;
        std::string_view value = text;

        if (value.empty() && declaration != nullptr && declaration->defaultValue.has_value())
            value = declaration->defaultValue.value();
        if (value.empty() && declaration != nullptr && declaration->fixedValue.has_value())
            value = declaration->fixedValue.value();
        std::optional<std::string> error = validateSimpleValue(type, value, _prefixes);
        if (error.has_value())
        {
            report(_depth - 1, phase, error.value());
            return nullptr;
        }
        recordIdentity(type, value, phase);
        if (declaration != nullptr && declaration->fixedValue.has_value()
            && !valuesEqual(type, value, declaration->fixedValue.value()))
            report(_depth - 1, phase, i18n::format(i18n::Message::XSD_ELEMENT_FIXED, {declaration->fixedValue.value()}));
        return &type;
    }

    /*
    ** ID должны быть уникальны в документе, IDREF — указывать на существующие ID.
    ** Значения остальных типов здесь не нужны, и для них функция выходит сразу,
    ** не нормализуя значение; нормализация — в переиспользуемый буфер. Ссылка запоминает шаг
    ** пути элемента: висячие ссылки выясняются лишь в конце документа.
    **
    ** IDs must be unique in the document, IDREFs must point to existing IDs.
    ** Values of other types are not needed here, and for them the function returns at once
    ** without normalizing the value; normalization goes into a reused buffer. A reference remembers the path
    ** step of the element: dangling references are found out only at the end of the document.
    */
    void StreamValidator::recordIdentity(const SimpleType& type, std::string_view value, CheckPhase phase)
    {
        bool referenceList = type.variety == Variety::LIST && type.itemType != nullptr
            && type.itemType->form == LexicalForm::IDREF;
        if (type.form != LexicalForm::ID && type.form != LexicalForm::IDREF && !referenceList)
            return;
        std::string_view normalized = normalizedView(value, WhiteSpace::COLLAPSE, _normalized);
        if (type.form == LexicalForm::ID && !_ids.insert(normalized))
            report(_depth - 1, phase, i18n::format(i18n::Message::ID_DUPLICATE, {normalized}));
        if (type.form == LexicalForm::IDREF)
            _references.emplace_back(std::string(normalized), _paths.pin(_depth - 1));
        if (!referenceList)
            return;
        for (std::string_view token : utils::splitTokens(normalized))
            _references.emplace_back(std::string(token), _paths.pin(_depth - 1));
    }

    // Висячие ссылки — после всего документа, с позицией корня (как у обхода дерева)
    // Dangling references — after the whole document, with the root position (as in the tree traversal)
    void StreamValidator::finishReferences()
    {
        for (std::size_t index = 0; index < _references.size(); ++index)
        {
            const std::pair<std::string, std::size_t>& reference = _references[index];
            if (_ids.contains(reference.first))
                continue;
            _log.add(DiagnosticOrder{static_cast<std::size_t>(OrderGroup::REFERENCES), 0, 0, 0, index},
                reference.second, i18n::format(i18n::Message::IDREF_DANGLING, {reference.first}), _rootPosition);
        }
        _references.clear();
    }
}
