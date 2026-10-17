#include "xsd/internal/validation_run.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    // Проход связан со схемой, но не владеет ею
    ValidationRun::ValidationRun(const Schema& schema)
        : _schema(schema), _diagnostics(), _ids(), _references()
    {
    }

    // Корень обязан соответствовать глобальному объявлению элемента схемы
    std::vector<Diagnostic> ValidationRun::run(const Document& document)
    {
        const Element* root = document.getRoot();

        if (root == nullptr)
            return _diagnostics;
        std::string path = "/" + root->getName();
        std::string namespaceUri = root->getNamespaceURI().value_or("");
        const ElementDecl* declaration = _schema.findElement(namespaceUri, root->getLocalName());
        if (declaration == nullptr)
            report(*root, path, "корневой элемент {" + namespaceUri + "}" + std::string(root->getLocalName())
                + " не объявлен в схеме");
        else
            validateElement(*root, declaration->type, declaration, path);
        finish(*root);
        return _diagnostics;
    }

    /*
    ** Элемент: тип из объявления может быть заменён через xsi:type;
    ** xsi:nil="true" отменяет проверку содержимого; xs:anyType допускает всё.
    */
    void ValidationRun::validateElement(const Element& element, TypeRef type,
        const ElementDecl* declaration, const std::string& path)
    {
        if (!applyXsiType(element, type, path))
            return;
        if (declaration != nullptr && declaration->isAbstract)
            report(element, path, "абстрактный элемент нельзя использовать в документе");
        if (isNil(element, declaration, path))
            return;
        if (type.simple != nullptr)
        {
            validateSimpleElement(element, *type.simple, declaration, path);
            return;
        }
        if (type.complex == nullptr)
            return;
        validateAttributes(element, *type.complex, path);
        validateContent(element, *type.complex, declaration, path);
    }

    // xsi:type="p:Тип" — замена типа элемента в экземпляре документа
    bool ValidationRun::applyXsiType(const Element& element, TypeRef& type, const std::string& path)
    {
        std::optional<std::string> value = element.getAttributeNS(XSI_NAMESPACE_URI, "type");

        if (!value.has_value())
            return true;
        utils::QualifiedNameParts parts = utils::splitQualifiedName(value.value());
        std::string namespaceUri = element.lookupNamespaceURI(parts.prefix).value_or("");
        bool found = false;
        TypeRef replacement = _schema.findType(namespaceUri, parts.localName, found);
        if (!found)
        {
            report(element, path, "тип xsi:type=\"" + value.value() + "\" не найден в схеме");
            return false;
        }
        type = replacement;
        return true;
    }

    // xsi:nil="true": допустим только для nillable-элементов и требует пустого содержимого
    bool ValidationRun::isNil(const Element& element, const ElementDecl* declaration,
        const std::string& path)
    {
        std::optional<std::string> nil = element.getAttributeNS(XSI_NAMESPACE_URI, "nil");

        if (!nil.has_value() || (nil.value() != "true" && nil.value() != "1"))
            return false;
        if (declaration == nullptr || !declaration->nillable)
        {
            report(element, path, "элемент не допускает xsi:nil (нет nillable=\"true\")");
            return false;
        }
        if (element.getChildCount() > 0)
            report(element, path, "элемент с xsi:nil=\"true\" должен быть пустым");
        return true;
    }
}
