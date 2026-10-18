#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Префиксы для выражений селекторов и полей. По XSD 1.0 они разрешаются
    ** по объявлениям xmlns:p, действующим на элементе схемы, — ближайшее
    ** к элементу объявление побеждает. Пространство имён по умолчанию
    ** (xmlns="...") к именам без префикса НЕ применяется: в XPath 1.0 имя
    ** без префикса всегда вне пространств имён.
    **
    ** Prefixes for selector and field expressions. By XSD 1.0 they are resolved
    ** by the xmlns:p declarations in scope on the schema element — the declaration
    ** nearest to the element wins. The default namespace
    ** (xmlns="...") is NOT applied to names without a prefix: in XPath 1.0 a name
    ** without a prefix is always in no namespace.
    */
    xpath::XPathContext namespacesInScope(const Element& element)
    {
        xpath::XPathContext context;

        for (const Node* node = &element; node != nullptr; node = node->getParent())
        {
            if (node->getType() != NodeType::ELEMENT)
                continue;
            for (const Attribute& attribute : static_cast<const Element*>(node)->getAttributes())
            {
                std::string prefix(utils::declaredPrefix(attribute.name));
                if (utils::isNamespaceDeclaration(attribute.name) && !prefix.empty())
                    context.namespaces.emplace(prefix, attribute.value);
            }
        }
        return context;
    }

    /*
    ** Связывание keyref с ключом по имени refer (после построения всей
    ** схемы — ключ может быть объявлен где угодно). Проверки по
    ** спецификации: ключ существует, это xs:key или xs:unique (не другой
    ** keyref), и число полей совпадает — иначе сравнивать нечего.
    **
    ** Binding a keyref to a key by the refer name (after the whole schema is
    ** built — the key may be declared anywhere). The checks by the
    ** specification: the key exists, it is an xs:key or xs:unique (not another
    ** keyref), and the number of fields matches — otherwise there is nothing to compare.
    */
    void SchemaBuilder::resolveKeyReferences()
    {
        for (const auto& entry : _builtIdentities)
        {
            IdentityConstraint& keyref = *entry.second;
            if (keyref.kind != IdentityKind::KEYREF)
                continue;
            auto found = _schema._identityByName.find(keyref.refer);
            if (found == _schema._identityByName.end())
                fail(*entry.first, i18n::format(i18n::Message::KEYREF_UNDECLARED_KEY, {keyref.name, keyref.refer.second}));
            if (found->second->kind == IdentityKind::KEYREF)
                fail(*entry.first, i18n::format(i18n::Message::KEYREF_TO_KEYREF, {keyref.name}));
            if (found->second->fields.size() != keyref.fields.size())
                fail(*entry.first, i18n::format(i18n::Message::KEYREF_FIELD_COUNT, {keyref.name, keyref.refer.second}));
            keyref.referenced = found->second;
        }
    }
}
