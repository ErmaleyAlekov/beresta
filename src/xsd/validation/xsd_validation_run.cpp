#include "xsd/internal/validation_run.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    // Проход связан со схемой, но не владеет ею
    // The pass is bound to the schema but does not own it
    ValidationRun::ValidationRun(const Schema& schema)
        : _schema(schema), _diagnostics(), _ids(), _references(), _pending(), _steps(), _children(),
          _keys(), _tasks(), _scopes(), _tables(), _elementPaths(), _elementTypes(), _attributeTypes()
    {
    }

    // Корень обязан соответствовать глобальному объявлению элемента схемы
    // The root must match a global element declaration of the schema
    std::vector<Diagnostic> ValidationRun::run(const Document& document)
    {
        const Element* root = document.getRoot();

        if (root == nullptr)
            return _diagnostics;
        _steps.push_back(PathStep{NO_PARENT_STEP, root, 0});
        std::string namespaceUri = root->getNamespaceURI().value_or("");
        const ElementDecl* declaration = _schema.findElement(namespaceUri, root->getLocalName());
        if (declaration == nullptr)
            report(*root, 0, i18n::format(i18n::Message::XSD_ROOT_UNDECLARED, {namespaceUri, root->getLocalName()}));
        else
        {
            _pending.push_back(PendingElement{root, declaration->type, declaration, 0});
            validateQueued();
        }
        finish(*root);
        checkIdentityConstraints();
        return _diagnostics;
    }

    /*
    ** Элемент: тип из объявления может быть заменён через xsi:type;
    ** xsi:nil="true" отменяет проверку содержимого; xs:anyType допускает всё.
    **
    ** An element: the type from the declaration may be replaced through xsi:type;
    ** xsi:nil="true" cancels the content check; xs:anyType allows everything.
    */
    void ValidationRun::validateElement(const Element& element, TypeRef type,
        const ElementDecl* declaration, std::size_t path)
    {
        if (declaration != nullptr && !declaration->identityConstraints.empty())
            _scopes.push_back(IdentityScope{&element, declaration, path});
        if (!_scopes.empty())
            _elementPaths.emplace(&element, path);
        if (!applyXsiType(element, type, path))
            return;
        if (declaration != nullptr && declaration->isAbstract)
            report(element, path, i18n::format(i18n::Message::XSD_ABSTRACT_ELEMENT));
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
    // xsi:type="p:Type" — replacing the element type in the document instance
    bool ValidationRun::applyXsiType(const Element& element, TypeRef& type, std::size_t path)
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
            report(element, path, i18n::format(i18n::Message::XSD_XSI_TYPE_NOT_FOUND, {value.value()}));
            return false;
        }
        type = replacement;
        return true;
    }

    // xsi:nil="true": допустим только для nillable-элементов и требует пустого содержимого
    // xsi:nil="true": allowed only for nillable elements and requires empty content
    bool ValidationRun::isNil(const Element& element, const ElementDecl* declaration,
        std::size_t path)
    {
        std::optional<std::string> nil = element.getAttributeNS(XSI_NAMESPACE_URI, "nil");

        if (!nil.has_value() || (nil.value() != "true" && nil.value() != "1"))
            return false;
        if (declaration == nullptr || !declaration->nillable)
        {
            report(element, path, i18n::format(i18n::Message::XSD_NOT_NILLABLE));
            return false;
        }
        if (element.getChildCount() > 0)
            report(element, path, i18n::format(i18n::Message::XSD_NIL_NOT_EMPTY));
        return true;
    }
}
