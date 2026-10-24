#include "xsd/schema.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd
{
    /*
    ** Глобальное объявление атрибута. Пространство xml: объявлено
    ** спецификацией XML, поэтому его атрибуты без схемы xml.xsd получают
    ** одно общее объявление типа anySimpleType (любое значение допустимо).
    **
    ** A global attribute declaration. The xml: namespace is declared by the
    ** XML specification, so its attributes without the xml.xsd schema get
    ** one shared declaration of the anySimpleType type (any value is valid).
    */
    const AttributeUse* Schema::findAttribute(std::string_view namespaceUri, std::string_view localName) const
    {
        static const AttributeUse XML_ATTRIBUTE{"", std::string(XML_NAMESPACE_URI),
            builtinType("anySimpleType"), false, false, std::nullopt, std::nullopt};
        auto found = _globalAttributes.find(Key(namespaceUri, localName));

        if (found != _globalAttributes.end())
            return &found->second;
        if (namespaceUri == XML_NAMESPACE_URI)
            return &XML_ATTRIBUTE;
        return nullptr;
    }

    /*
    ** xs:anyType: последовательность из одного xs:any (##any, lax,
    ** 0..unbounded), смешанное содержимое и xs:anyAttribute (##any, lax).
    ** Базовый тип — он сам (ссылка TypeRef без указателей), способ — ограничение.
    **
    ** xs:anyType: a sequence of one xs:any (##any, lax,
    ** 0..unbounded), mixed content and xs:anyAttribute (##any, lax).
    ** The base type is itself (a TypeRef without pointers), the method is restriction.
    */
    static ComplexType buildAnyType()
    {
        ComplexType type;
        content::Particle wildcard;

        type.name = "anyType";
        type.namespaceUri = std::string(XSD_NAMESPACE_URI);
        type.content = ContentType::MIXED_CONTENT;
        wildcard.kind = content::ParticleKind::WILDCARD;
        wildcard.minOccurs = 0;
        wildcard.maxOccurs = content::UNBOUNDED;
        wildcard.slot = 0;
        type.particle.children.push_back(wildcard);
        type.hasParticle = true;
        type.hasElementWildcard = true;
        type.elementWildcard = ProcessContents::LAX_CHECK;
        type.wildcards.push_back(ProcessContents::LAX_CHECK);
        type.anyAttribute = true;
        type.attributeWildcard.process = ProcessContents::LAX_CHECK;
        return type;
    }

    // Один неизменяемый экземпляр; инициализация локальной static потокобезопасна (C++11)
    // One immutable instance; initialization of a local static is thread-safe (C++11)
    const ComplexType& Schema::anyTypeDefinition()
    {
        static const ComplexType ANY_TYPE = buildAnyType();

        return ANY_TYPE;
    }
}
