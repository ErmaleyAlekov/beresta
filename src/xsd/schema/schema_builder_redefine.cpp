#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Виды компонентов, которые может переопределить xs:redefine (XSD 1.0, раздел 4.2.2)
    // The component kinds xs:redefine may redefine (XSD 1.0, section 4.2.2)
    static bool isRedefinable(std::string_view kind) noexcept
    {
        return kind == "simpleType" || kind == "complexType" || kind == "group" || kind == "attributeGroup";
    }

    /*
    ** xs:redefine / xs:override — работа только этапа загрузки схемы: в
    ** реестре определений имя компонента начинает указывать на новое
    ** определение ещё до построения моделей, поэтому проверка документов
    ** о подмене не знает и не замедляется.
    ** override (XSD 1.1) регистрирует замены ДО загрузки документа: его
    ** собственные определения с теми же именами затем пропускаются.
    ** redefine загружает документ, а затем подменяет его компоненты,
    ** сохраняя прежние: новое определение ссылается на них по своему имени.
    **
    ** xs:redefine / xs:override are work of the schema loading stage only: in
    ** the definition registry the component name starts pointing to the new
    ** definition even before the models are built, so document validation
    ** does not know about the replacement and is not slowed down.
    ** override (XSD 1.1) registers the replacements BEFORE loading the document: its
    ** own definitions with the same names are then skipped.
    ** redefine loads the document and then replaces its components,
    ** keeping the former ones: the new definition refers to them by its own name.
    */
    void SchemaBuilder::loadRedefinition(const Element& directive, const SchemaSource& source)
    {
        bool redefine = isSchemaElement(directive, "redefine");
        std::vector<const Element*> components = schemaChildren(directive);

        if (!redefine)
        {
            for (const Element* component : components)
                overrideDefinition(*component, source);
        }
        std::string targetUri = loadReferenced(directive, source);
        if (redefine)
        {
            for (const Element* component : components)
                redefineDefinition(*component, source, targetUri);
        }
    }

    /*
    ** Подмена через xs:redefine: прежнее определение обязано существовать
    ** в подключённой схеме, а тип — наследоваться от самого себя. Прежнее
    ** определение сохраняется — на него ведёт ссылка нового на своё имя.
    ** Если имя уже занято переопределением, оно обязано принадлежать
    ** самому подключаемому документу (цепочка: A переопределяет B, который
    ** переопределил C) или документу, загруженному изнутри текущего
    ** (A включает B, оба переопределяют C: A уточняет версию B). Иначе это
    ** второе переопределение того же компонента (два соседних xs:redefine
    ** одного C или два одноимённых компонента в одном xs:redefine): ошибка.
    **
    ** A replacement through xs:redefine: the former definition must exist
    ** in the included schema, and a type must derive from itself. The former
    ** definition is kept — the new one's reference to its own name leads to it.
    ** If the name is already taken by a redefinition, it must belong to the
    ** included document itself (a chain: A redefines B, which
    ** redefined C) or to a document loaded from inside the current one
    ** (A includes B, both redefine C: A refines B's version). Otherwise it is
    ** a second redefinition of the same component (two sibling xs:redefine
    ** of one C or two components of the same name in one xs:redefine): an error.
    */
    void SchemaBuilder::redefineDefinition(const Element& child, const SchemaSource& source,
        const std::string& targetUri)
    {
        std::string kind(child.getLocalName());
        std::map<QualifiedKey, Definition>* registry = registryFor(kind);
        std::string name = attributeOr(child, "name", "");

        if (registry == nullptr || !isRedefinable(kind))
            fail(child, i18n::format(i18n::Message::SCHEMA_REDEFINE_UNEXPECTED, {"redefine", kind}));
        QualifiedKey key(source.targetNamespace, name);
        auto found = registry->find(key);
        if (found == registry->end())
            fail(child, i18n::format(i18n::Message::SCHEMA_REDEFINE_MISSING, {kind, name}));
        const std::string& owner = found->second.source->baseUri;
        bool nested = owner == targetUri || (owner != source.baseUri && loadedWithin(owner, source.baseUri));
        if (_redefinitions.count(found->second.element) != 0 && !nested)
            fail(child, i18n::format(i18n::Message::SCHEMA_REDEFINE_TWICE, {kind, name}));
        checkSelfDerivation(child, key);
        checkSelfReferences(child, key);
        _redefinitions[&child] = Redefinition{kind, key, found->second};
        found->second = Definition{&child, &source};
    }

    /*
    ** Подмена через xs:override (XSD 1.1): новое определение занимает имя
    ** целиком, прежнего может и не быть. Имя запоминается — одноимённое
    ** определение подключаемого документа при регистрации пропускается.
    ** xs:notation допустим, но нотации схема не хранит.
    **
    ** A replacement through xs:override (XSD 1.1): the new definition takes the name
    ** entirely, there may be no former one. The name is remembered — a definition
    ** of the included document with the same name is skipped at registration.
    ** xs:notation is allowed, but the schema does not keep notations.
    */
    void SchemaBuilder::overrideDefinition(const Element& child, const SchemaSource& source)
    {
        std::string kind(child.getLocalName());
        std::map<QualifiedKey, Definition>* registry = registryFor(kind);
        std::string name = attributeOr(child, "name", "");

        if (kind == "notation")
            return;
        if (registry == nullptr)
            fail(child, i18n::format(i18n::Message::SCHEMA_REDEFINE_UNEXPECTED, {"override", kind}));
        if (name.empty())
            fail(child, i18n::format(i18n::Message::SCHEMA_GLOBAL_NO_NAME, {kind}));
        QualifiedKey key(source.targetNamespace, name);
        (*registry)[key] = Definition{&child, &source};
        _overridden.insert(std::make_pair(kind, key));
    }

    /*
    ** Тип внутри xs:redefine обязан быть ограничением или расширением
    ** самого себя (src-redefine.5): base указывает на собственное имя.
    ** Группы и группы атрибутов ссылаются на себя по желанию.
    **
    ** A type inside xs:redefine must be a restriction or an extension
    ** of itself (src-redefine.5): base points to its own name.
    ** Groups and attribute groups refer to themselves optionally.
    */
    void SchemaBuilder::checkSelfDerivation(const Element& child, const QualifiedKey& key) const
    {
        const Element* derivation = nullptr;
        const Element* content = firstSchemaChild(child, "complexContent");

        if (isSchemaElement(child, "simpleType"))
            derivation = firstSchemaChild(child, "restriction");
        else if (!isSchemaElement(child, "complexType"))
            return;
        if (content == nullptr)
            content = firstSchemaChild(child, "simpleContent");
        if (content != nullptr)
            derivation = firstSchemaChild(*content, "extension");
        if (content != nullptr && derivation == nullptr)
            derivation = firstSchemaChild(*content, "restriction");
        std::optional<std::string> base;
        if (derivation != nullptr)
            base = derivation->getAttribute("base");
        if (!base.has_value() || resolveQName(*derivation, base.value()) != key)
            fail(child, i18n::format(i18n::Message::SCHEMA_REDEFINE_NOT_SELF, {key.second}));
    }
}
