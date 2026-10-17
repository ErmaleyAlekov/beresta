#include "xsd/internal/validation_run.hpp"
#include "utils/qualified_name.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Элемент простого типа: ни дочерних элементов, ни атрибутов (кроме
    ** служебных xmlns и xsi:*), а текст обязан соответствовать типу.
    */
    void ValidationRun::validateSimpleElement(const Element& element, const SimpleType& type,
        const ElementDecl* declaration, const std::string& path)
    {
        for (const Node* child = element.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::ELEMENT)
            {
                report(element, path, "элемент простого типа " + type.name + " не может содержать элементы");
                return;
            }
        }
        for (const Attribute& attribute : element.getAttributes())
        {
            bool service = utils::isNamespaceDeclaration(attribute.name)
                || attributeKey(element, attribute).first == XSI_NAMESPACE_URI;
            if (!service)
                report(element, path, "атрибут \"" + attribute.name + "\" недопустим у элемента простого типа");
        }
        checkValue(element, type, element.getTextContent(), declaration, path);
    }

    /*
    ** Значение текста: пустой текст заменяется значением default; затем
    ** проверка типа, учёт ID/IDREF и сравнение с fixed (по значению).
    */
    void ValidationRun::checkValue(const Element& element, const SimpleType& type, std::string value,
        const ElementDecl* declaration, const std::string& path)
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
        recordIdentity(type, value, element, path);
        if (declaration != nullptr && declaration->fixedValue.has_value()
            && !valuesEqual(type, value, declaration->fixedValue.value()))
            report(element, path, "значение должно быть равно фиксированному \""
                + declaration->fixedValue.value() + "\"");
    }

    // ID должны быть уникальны в документе, IDREF — указывать на существующие ID
    void ValidationRun::recordIdentity(const SimpleType& type, std::string_view value,
        const Element& element, const std::string& path)
    {
        std::string normalized = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);

        if (type.form == LexicalForm::ID && !_ids.insert(normalized).second)
            report(element, path, "повторяющийся ID \"" + normalized + "\"");
        if (type.form == LexicalForm::IDREF)
            _references.emplace_back(normalized, path);
        if (type.variety != Variety::LIST || type.itemType == nullptr
            || type.itemType->form != LexicalForm::IDREF)
            return;
        for (std::string_view token : utils::splitTokens(normalized))
            _references.emplace_back(std::string(token), path);
    }

    // Проверка ссылок IDREF после обхода всего документа
    void ValidationRun::finish(const Element& root)
    {
        for (const std::pair<std::string, std::string>& reference : _references)
        {
            if (_ids.find(reference.first) == _ids.end())
            {
                Diagnostic diagnostic{ErrorSeverity::RECOVERABLE_ERROR, reference.second
                    + ": IDREF \"" + reference.first + "\" ссылается на несуществующий ID",
                    ParsePosition{0, root.getSourceLocation().line, root.getSourceLocation().column}};
                _diagnostics.push_back(diagnostic);
            }
        }
    }

    // Ошибка с путём к элементу и его позицией в исходнике (если она записана)
    void ValidationRun::report(const Element& element, const std::string& path, const std::string& message)
    {
        const SourceLocation& location = element.getSourceLocation();

        _diagnostics.push_back(Diagnostic{ErrorSeverity::RECOVERABLE_ERROR, path + ": " + message,
            ParsePosition{0, location.line, location.column}});
    }
}
