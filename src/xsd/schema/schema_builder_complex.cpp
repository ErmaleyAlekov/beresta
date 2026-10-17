#include "xsd/internal/schema_builder.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:complexType. Объект регистрируется до построения содержимого
    ** (рекурсивные модели), но помечается незавершённым: наследоваться
    ** от незавершённого типа нельзя — это циклическое наследование.
    */
    const ComplexType* SchemaBuilder::buildComplexType(const Element& definition,
        const SchemaSource& source, const QualifiedKey& name)
    {
        _schema._complexTypes.emplace_back();
        ComplexType& type = _schema._complexTypes.back();
        const Element* simpleContent = firstSchemaChild(definition, "simpleContent");
        const Element* complexContent = firstSchemaChild(definition, "complexContent");
        bool mixed = attributeOr(definition, "mixed", "false") == "true";

        type.name = name.second;
        type.namespaceUri = name.first;
        if (!name.second.empty())
        {
            _builtTypes[name] = TypeRef{nullptr, &type};
            _schema._globalTypes[name] = TypeRef{nullptr, &type};
        }
        _incomplete.insert(&type);
        if (simpleContent != nullptr)
            buildSimpleContent(type, *simpleContent, source);
        else if (complexContent != nullptr)
            buildComplexContent(type, *complexContent, source, mixed);
        else
            buildTypeBody(type, definition, source, mixed);
        _incomplete.erase(&type);
        return &type;
    }

    /*
    ** Тело типа: необязательная модель (sequence / choice / all / group)
    ** и атрибуты. Модель заменяет унаследованную только если она есть;
    ** склейку при расширении выполняет buildComplexContent.
    */
    void SchemaBuilder::buildTypeBody(ComplexType& type, const Element& body, const SchemaSource& source,
        bool mixed)
    {
        for (const Element* child : schemaChildren(body))
        {
            std::string_view kind = child->getLocalName();
            if (kind != "sequence" && kind != "choice" && kind != "all" && kind != "group")
                continue;
            type.particle = buildParticle(*child, source, type, 0);
            type.hasParticle = true;
            break;
        }
        if (type.hasParticle || mixed)
            type.content = ContentType::ELEMENT_ONLY;
        if (mixed)
            type.content = ContentType::MIXED_CONTENT;
        buildAttributes(body, source, type, 0);
    }

    // Наследование задаётся единственным xs:extension или xs:restriction
    const Element& SchemaBuilder::derivationOf(const Element& content, bool& extension) const
    {
        const Element* derivation = firstSchemaChild(content, "extension");

        extension = derivation != nullptr;
        if (derivation == nullptr)
            derivation = firstSchemaChild(content, "restriction");
        if (derivation == nullptr)
            fail(content, "ожидался xs:extension или xs:restriction");
        return *derivation;
    }

    // Базовый тип (по умолчанию xs:anyType); от незавершённого типа наследоваться нельзя
    const ComplexType* SchemaBuilder::completeBase(const Element& derivation, TypeRef& base)
    {
        std::optional<std::string> name = derivation.getAttribute("base");

        base = TypeRef();
        if (name.has_value())
            base = resolveType(resolveQName(derivation, name.value()));
        if (base.complex != nullptr && _incomplete.count(base.complex) != 0)
            fail(derivation, "циклическое наследование типа");
        return base.complex;
    }
}
