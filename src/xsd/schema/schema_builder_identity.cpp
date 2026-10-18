#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    // Вид ограничения по локальному имени элемента схемы
    // The kind of a constraint by the local name of the schema element
    static IdentityKind identityKind(const Element& definition)
    {
        if (isSchemaElement(definition, "key"))
            return IdentityKind::KEY;
        if (isSchemaElement(definition, "keyref"))
            return IdentityKind::KEYREF;
        return IdentityKind::UNIQUE;
    }

    /*
    ** Ограничения идентичности объявляются внутри xs:element после типа.
    ** Они относятся к объявлению элемента: каждый элемент документа с этим
    ** объявлением становится областью, в которой ключи должны быть уникальны.
    **
    ** Identity constraints are declared inside xs:element after the type.
    ** They belong to the element declaration: every document element with this
    ** declaration becomes a scope in which the keys must be unique.
    */
    void SchemaBuilder::readIdentityConstraints(const Element& definition, const SchemaSource& source,
        ElementDecl& declaration)
    {
        for (const Element* child : schemaChildren(definition))
        {
            bool identity = isSchemaElement(*child, "unique") || isSchemaElement(*child, "key")
                || isSchemaElement(*child, "keyref");
            if (identity)
                declaration.identityConstraints.push_back(buildIdentityConstraint(*child, source));
        }
    }

    /*
    ** Построение одного ограничения. Локальное объявление элемента может
    ** строиться повторно (группа, использованная в нескольких типах), поэтому
    ** результат запоминается по узлу схемы: одно определение — одно
    ** ограничение. Имена ограничений образуют одно пространство символов
    ** схемы: два разных определения с одним именем — ошибка схемы.
    **
    ** Building one constraint. A local element declaration may be
    ** built repeatedly (a group used in several types), so the result
    ** is memoized by the schema node: one definition — one
    ** constraint. The constraint names form one symbol space of the
    ** schema: two different definitions with the same name are a schema error.
    */
    const IdentityConstraint* SchemaBuilder::buildIdentityConstraint(const Element& definition,
        const SchemaSource& source)
    {
        auto built = _builtIdentities.find(&definition);
        if (built != _builtIdentities.end())
            return built->second;
        QualifiedKey key(source.targetNamespace, attributeOr(definition, "name", ""));
        if (key.second.empty())
            fail(definition, i18n::format(i18n::Message::IDENTITY_NO_NAME));
        if (_schema._identityByName.count(key) != 0)
            fail(definition, i18n::format(i18n::Message::IDENTITY_DUPLICATE, {key.second}));
        const Element* selector = firstSchemaChild(definition, "selector");
        if (selector == nullptr)
            fail(definition, i18n::format(i18n::Message::IDENTITY_NO_SELECTOR, {key.second}));
        QualifiedKey refer;
        if (identityKind(definition) == IdentityKind::KEYREF)
            refer = resolveQName(definition, attributeOr(definition, "refer", ""));
        _schema._identityConstraints.push_back(IdentityConstraint{identityKind(definition), key.second,
            key.first, compileIdentityPath(*selector), identityFields(definition),
            namespacesInScope(definition), refer, nullptr});
        IdentityConstraint* constraint = &_schema._identityConstraints.back();
        _schema._identityByName[key] = constraint;
        _builtIdentities[&definition] = constraint;
        return constraint;
    }

    // Поля ключа в порядке объявления; хотя бы одно обязательно
    // The key fields in declaration order; at least one is mandatory
    std::vector<xpath::XPathExpression> SchemaBuilder::identityFields(const Element& definition) const
    {
        std::vector<xpath::XPathExpression> fields;

        for (const Element* child : schemaChildren(definition))
        {
            if (isSchemaElement(*child, "field"))
                fields.push_back(compileIdentityPath(*child));
        }
        if (fields.empty())
            fail(definition, i18n::format(i18n::Message::IDENTITY_NO_FIELDS));
        return fields;
    }

    /*
    ** Компиляция атрибута xpath у xs:selector / xs:field. Грамматика XSD —
    ** подмножество XPath 1.0 (оси child и attribute, ".//" и "|"), поэтому
    ** выражение компилирует общий движок. Синтаксическая ошибка XPath
    ** превращается в ошибку схемы с позицией в документе схемы.
    **
    ** Compiling the xpath attribute of xs:selector / xs:field. The XSD grammar is
    ** a subset of XPath 1.0 (the child and attribute axes, ".//" and "|"), so
    ** the common engine compiles the expression. An XPath syntax error
    ** becomes a schema error with the position in the schema document.
    */
    xpath::XPathExpression SchemaBuilder::compileIdentityPath(const Element& part) const
    {
        std::string path = attributeOr(part, "xpath", "");

        if (path.empty())
            fail(part, i18n::format(i18n::Message::IDENTITY_NO_XPATH, {part.getLocalName()}));
        try
        {
            return xpath::XPathExpression(path);
        }
        catch (const DomException& error)
        {
            fail(part, i18n::format(i18n::Message::IDENTITY_XPATH_INVALID, {path, error.getMessage()}));
        }
    }
}
