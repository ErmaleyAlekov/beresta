#ifndef DTD_MODEL_HPP
#define DTD_MODEL_HPP

#include "content/particle.hpp"

#include <functional>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::dtd
{
    /*
    ** Объявление сущности <!ENTITY ...>.
    ** value — текст замены внутренней сущности (символьные ссылки и
    ** параметрические сущности уже раскрыты, ссылки на общие сущности
    ** оставлены как есть — они раскрываются в момент использования).
    ** Для внешней сущности value заполняется при первой загрузке.
    **
    ** An entity declaration <!ENTITY ...>.
    ** value is the replacement text of an internal entity (character references and
    ** parameter entities are already expanded, references to general entities are
    ** left as is — they are expanded at the point of use).
    ** For an external entity value is filled on the first load.
    */
    struct EntityDeclaration
    {
        std::string name;
        std::string value;
        std::string publicId;
        std::string systemId;
        std::string notation;           // NDATA — неразбираемая сущность (например, картинка)
                                        // NDATA — an unparsed entity (for example, a picture)
        bool isParameter = false;       // %name; — используется только внутри DTD / %name; — used only inside a DTD
        bool isExternal = false;
        bool isLoaded = false;          // внешняя сущность уже загружена в value
                                        // the external entity is already loaded into value
        // Объявлена во внешнем подмножестве или внешней PE: при standalone="yes"
        // ссылаться на неё нельзя (WFC: Entity Declared, раздел 4.1)
        // Declared in the external subset or an external PE: with standalone="yes"
        // it must not be referenced (WFC: Entity Declared, section 4.1)
        bool externallyDeclared = false;
        // База для systemId: URI текста, где стоит объявление (раздел 4.2.2)
        // The base for systemId: the URI of the text containing the declaration (section 4.2.2)
        std::string baseUri;
        // URI, по которому реально загружен текст внешней сущности (база для её содержимого)
        // The URI the external entity text was actually loaded from (the base for its content)
        std::string resolvedUri;
    };

    // Тип атрибута из <!ATTLIST>
    // The attribute type from <!ATTLIST>
    enum class AttributeType
    {
        CDATA, ID, IDREF, IDREFS, ENTITY, ENTITIES, NMTOKEN, NMTOKENS, NOTATION, ENUMERATION
    };

    // Вид значения по умолчанию
    // The kind of the default value
    enum class DefaultKind
    {
        REQUIRED,   // #REQUIRED — атрибут обязателен / #REQUIRED — the attribute is mandatory
        IMPLIED,    // #IMPLIED — необязателен, значения по умолчанию нет
                    // #IMPLIED — optional, there is no default value
        FIXED,      // #FIXED "v" — если указан, обязан равняться v / #FIXED "v" — if specified, it must equal v
        VALUE       // "v" — значение по умолчанию / "v" — the default value
    };

    // Объявление одного атрибута
    // The declaration of one attribute
    struct AttributeDeclaration
    {
        std::string name;
        AttributeType type = AttributeType::CDATA;
        std::vector<std::string> allowedValues;     // для ENUMERATION и NOTATION / for ENUMERATION and NOTATION
        DefaultKind defaultKind = DefaultKind::IMPLIED;
        std::string defaultValue;
        bool externallyDeclared = false;            // во внешнем DTD (VC: Standalone, раздел 2.9)
                                                    // in the external DTD (VC: Standalone, section 2.9)
    };

    // Вид содержимого элемента из <!ELEMENT>
    // The kind of element content from <!ELEMENT>
    enum class ContentKind
    {
        EMPTY_CONTENT,  // EMPTY
        ANY_CONTENT,    // ANY
        MIXED,          // (#PCDATA | a | b)*
        CHILDREN        // модель из элементов: (a, (b | c)+) / a model of elements: (a, (b | c)+)
    };

    // Объявление элемента
    // An element declaration
    struct ElementDeclaration
    {
        std::string name;
        ContentKind kind = ContentKind::ANY_CONTENT;
        content::Particle model;                // для CHILDREN / for CHILDREN
        std::vector<std::string> mixedNames;    // для MIXED / for MIXED
        bool externallyDeclared = false;        // во внешнем DTD (VC: Standalone, раздел 2.9)
                                                // in the external DTD (VC: Standalone, section 2.9)
    };

    /*
    ** DtdModel — разобранный DTD. Заполняется DtdParser, после разбора
    ** используется только для чтения и разделяется между копиями DOCTYPE.
    ** По стандарту XML первое объявление сущности или атрибута «выигрывает»,
    ** повторные молча игнорируются; поэтому add* возвращают false для дублей.
    ** Ассоциативные контейнеры с std::less<> позволяют искать по string_view
    ** без создания временных строк.
    **
    ** DtdModel — a parsed DTD. Filled by DtdParser; after parsing it is
    ** used read-only and shared between DOCTYPE copies.
    ** By the XML standard the first declaration of an entity or attribute "wins",
    ** repeated ones are silently ignored; so add* return false for duplicates.
    ** Associative containers with std::less<> allow lookup by string_view
    ** without creating temporary strings.
    */
    class DtdModel
    {
    public:
        bool addEntity(EntityDeclaration declaration);
        bool addElement(ElementDeclaration declaration);
        bool addAttribute(std::string_view element, AttributeDeclaration declaration);
        void addNotation(std::string name);

        const EntityDeclaration* findEntity(std::string_view name) const noexcept;
        const EntityDeclaration* findParameterEntity(std::string_view name) const noexcept;

        // Изменяемый доступ — для загрузки текста внешней сущности при первом использовании
        // Mutable access — to load the text of an external entity on first use
        EntityDeclaration* findEntityForUpdate(std::string_view name, bool parameter) noexcept;

        const ElementDeclaration* findElement(std::string_view name) const noexcept;
        const std::vector<AttributeDeclaration>* findAttributes(std::string_view element) const noexcept;
        const AttributeDeclaration* findAttribute(std::string_view element,
            std::string_view attribute) const noexcept;

        bool hasNotation(std::string_view name) const noexcept;

        // Объявлен ли атрибут как ID (для Document::getElementById)
        // Whether the attribute is declared as ID (for Document::getElementById)
        bool isIdAttribute(std::string_view element, std::string_view attribute) const noexcept;

        // ---- Обход всех объявлений (проверки ограничений валидности DTD) ----
        // ---- Iterating all declarations (DTD validity constraint checks) ----
        const std::map<std::string, ElementDeclaration, std::less<>>& getElements() const noexcept;
        const std::map<std::string, std::vector<AttributeDeclaration>, std::less<>>&
            getAttributeLists() const noexcept;
        const std::map<std::string, EntityDeclaration, std::less<>>& getEntities() const noexcept;

        /*
        ** Нарушения валидности, замеченные ещё при разборе DTD (повторное
        ** объявление элемента, нарушение вложенности PE). Сообщаются
        ** сессией только при включённой проверке по DTD.
        **
        ** Validity violations noticed already while parsing the DTD (a repeated
        ** element declaration, a PE nesting violation). They are reported
        ** by the session only when DTD validation is enabled.
        */
        void addValidityNote(std::string message);
        const std::vector<std::string>& getValidityNotes() const noexcept;

        /*
        ** Есть ли у документа внешнее подмножество или ссылки %pe; — тогда
        ** необъявленная общая сущность нарушает валидность, а не
        ** well-formedness (WFC/VC: Entity Declared, раздел 4.1).
        **
        ** Whether the document has an external subset or %pe; references — then
        ** an undeclared general entity violates validity, not well-formedness
        ** (WFC/VC: Entity Declared, section 4.1).
        */
        void markExternalParts() noexcept;
        bool hasExternalParts() const noexcept;

    private:
        std::map<std::string, EntityDeclaration, std::less<>> _entities;
        std::map<std::string, EntityDeclaration, std::less<>> _parameterEntities;
        std::map<std::string, ElementDeclaration, std::less<>> _elements;
        std::map<std::string, std::vector<AttributeDeclaration>, std::less<>> _attributes;
        std::set<std::string, std::less<>> _notations;
        std::vector<std::string> _validityNotes;
        bool _hasExternalParts = false;
    };
}

#endif
