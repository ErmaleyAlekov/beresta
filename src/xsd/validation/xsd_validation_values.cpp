#include "xsd/internal/validation_run.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Элемент простого типа: ни дочерних элементов, ни атрибутов (кроме
    ** служебных xmlns и xsi:*), а текст обязан соответствовать типу.
    **
    ** An element of a simple type: neither child elements nor attributes (except
    ** the service xmlns and xsi:*), and the text must match the type.
    */
    void ValidationRun::validateSimpleElement(const Element& element, const SimpleType& type,
        const ElementDecl* declaration, std::size_t path)
    {
        for (const Node* child = element.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::ELEMENT)
            {
                report(element, path, i18n::format(i18n::Message::XSD_SIMPLE_WITH_CHILDREN, {type.name}));
                return;
            }
        }
        for (const Attribute& attribute : element.getAttributes())
        {
            if (utils::isNamespaceDeclaration(attribute.name))
                continue;
            if (attributeKey(element, attribute).first != XSI_NAMESPACE_URI)
                report(element, path, i18n::format(i18n::Message::XSD_SIMPLE_WITH_ATTRIBUTE, {attribute.name}));
        }
        std::string buffer;
        checkValue(element, type, textOf(element, buffer), declaration, path);
    }

    /*
    ** Значение текста: пустой текст заменяется значением default; затем
    ** проверка типа, учёт ID/IDREF и сравнение с fixed (по значению).
    **
    ** The text value: an empty text is replaced with the default value; then
    ** the type check, ID/IDREF accounting and the comparison with fixed (by value).
    */
    void ValidationRun::checkValue(const Element& element, const SimpleType& type, std::string_view value,
        const ElementDecl* declaration, std::size_t path)
    {
        if (value.empty() && declaration != nullptr && declaration->defaultValue.has_value())
            value = declaration->defaultValue.value();
        if (value.empty() && declaration != nullptr && declaration->fixedValue.has_value())
            value = declaration->fixedValue.value();
        std::optional<std::string> error = validateSimpleValue(type, value, &element);
        if (error.has_value())
        {
            report(element, path, error.value());
            return;
        }
        if (!_scopes.empty())
            _elementTypes[&element] = &type;
        recordIdentity(type, value, element, path);
        if (declaration != nullptr && declaration->fixedValue.has_value()
            && !valuesEqual(type, value, declaration->fixedValue.value()))
            report(element, path, i18n::format(i18n::Message::XSD_ELEMENT_FIXED, {declaration->fixedValue.value()}));
    }

    /*
    ** ID должны быть уникальны в документе, IDREF — указывать на существующие ID.
    ** Значения остальных типов здесь не нужны, и для них функция выходит сразу,
    ** не нормализуя (то есть не копируя) значение.
    **
    ** IDs must be unique in the document, IDREFs must point to existing IDs.
    ** Values of other types are not needed here, and for them the function returns at once
    ** without normalizing (that is, without copying) the value.
    */
    void ValidationRun::recordIdentity(const SimpleType& type, std::string_view value,
        const Element& element, std::size_t path)
    {
        bool referenceList = type.variety == Variety::LIST && type.itemType != nullptr
            && type.itemType->form == LexicalForm::IDREF;
        if (type.form != LexicalForm::ID && type.form != LexicalForm::IDREF && !referenceList)
            return;
        std::string normalized = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);
        if (type.form == LexicalForm::ID && !_ids.insert(normalized).second)
            report(element, path, i18n::format(i18n::Message::ID_DUPLICATE, {normalized}));
        if (type.form == LexicalForm::IDREF)
            _references.emplace_back(normalized, path);
        if (!referenceList)
            return;
        for (std::string_view token : utils::splitTokens(normalized))
            _references.emplace_back(std::string(token), path);
    }

    // Проверка ссылок IDREF после обхода всего документа
    // Checks the IDREF references after traversing the whole document
    void ValidationRun::finish(const Element& root)
    {
        for (const std::pair<std::string, std::size_t>& reference : _references)
        {
            if (_ids.find(reference.first) == _ids.end())
            {
                Diagnostic diagnostic{ErrorSeverity::RECOVERABLE_ERROR, pathText(reference.second) + ": " + i18n::format(i18n::Message::IDREF_DANGLING, {reference.first}),
                    ParsePosition{0, root.getSourceLocation().line, root.getSourceLocation().column}};
                _diagnostics.push_back(diagnostic);
            }
        }
    }

    // Ошибка у элемента с шагом пути path: строка пути строится только сейчас
    // An error at an element with the path step path: the path string is built only now
    void ValidationRun::report(const Element& element, std::size_t path, const std::string& message)
    {
        reportAt(element, pathText(path), message);
    }
}
