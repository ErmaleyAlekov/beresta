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
    */
    struct EntityDeclaration
    {
        std::string name;
        std::string value;
        std::string publicId;
        std::string systemId;
        std::string notation;           // NDATA — неразбираемая сущность (например, картинка)
        bool isParameter = false;       // %name; — используется только внутри DTD
        bool isExternal = false;
        bool isLoaded = false;          // внешняя сущность уже загружена в value
    };

    // Тип атрибута из <!ATTLIST>
    enum class AttributeType
    {
        CDATA, ID, IDREF, IDREFS, ENTITY, ENTITIES, NMTOKEN, NMTOKENS, NOTATION, ENUMERATION
    };

    // Вид значения по умолчанию
    enum class DefaultKind
    {
        REQUIRED,   // #REQUIRED — атрибут обязателен
        IMPLIED,    // #IMPLIED — необязателен, значения по умолчанию нет
        FIXED,      // #FIXED "v" — если указан, обязан равняться v
        VALUE       // "v" — значение по умолчанию
    };

    // Объявление одного атрибута
    struct AttributeDeclaration
    {
        std::string name;
        AttributeType type = AttributeType::CDATA;
        std::vector<std::string> allowedValues;     // для ENUMERATION и NOTATION
        DefaultKind defaultKind = DefaultKind::IMPLIED;
        std::string defaultValue;
    };

    // Вид содержимого элемента из <!ELEMENT>
    enum class ContentKind
    {
        EMPTY_CONTENT,  // EMPTY
        ANY_CONTENT,    // ANY
        MIXED,          // (#PCDATA | a | b)*
        CHILDREN        // модель из элементов: (a, (b | c)+)
    };

    // Объявление элемента
    struct ElementDeclaration
    {
        std::string name;
        ContentKind kind = ContentKind::ANY_CONTENT;
        content::Particle model;                // для CHILDREN
        std::vector<std::string> mixedNames;    // для MIXED
    };

    /*
    ** DtdModel — разобранный DTD. Заполняется DtdParser, после разбора
    ** используется только для чтения и разделяется между копиями DOCTYPE.
    ** По стандарту XML первое объявление сущности или атрибута «выигрывает»,
    ** повторные молча игнорируются; поэтому add* возвращают false для дублей.
    ** Ассоциативные контейнеры с std::less<> позволяют искать по string_view
    ** без создания временных строк.
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
        EntityDeclaration* findEntityForUpdate(std::string_view name, bool parameter) noexcept;

        const ElementDeclaration* findElement(std::string_view name) const noexcept;
        const std::vector<AttributeDeclaration>* findAttributes(std::string_view element) const noexcept;
        const AttributeDeclaration* findAttribute(std::string_view element,
            std::string_view attribute) const noexcept;

        bool hasNotation(std::string_view name) const noexcept;

        // Объявлен ли атрибут как ID (для Document::getElementById)
        bool isIdAttribute(std::string_view element, std::string_view attribute) const noexcept;

    private:
        std::map<std::string, EntityDeclaration, std::less<>> _entities;
        std::map<std::string, EntityDeclaration, std::less<>> _parameterEntities;
        std::map<std::string, ElementDeclaration, std::less<>> _elements;
        std::map<std::string, std::vector<AttributeDeclaration>, std::less<>> _attributes;
        std::set<std::string, std::less<>> _notations;
    };
}

#endif
