#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::content
{
    // Значение maxOccurs="unbounded" (и '*' / '+' в DTD)
    // The maxOccurs="unbounded" value (and '*' / '+' in a DTD)
    inline constexpr std::size_t UNBOUNDED = static_cast<std::size_t>(-1);

    // Вид частицы модели содержимого
    // The kind of a content model particle
    enum class ParticleKind
    {
        ELEMENT,    // Конкретный элемент / A specific element
        SEQUENCE,   // Дети строго по порядку (',' в DTD, xs:sequence)
                    // Children strictly in order (',' in a DTD, xs:sequence)
        CHOICE,     // Ровно один из вариантов ('|' в DTD, xs:choice)
                    // Exactly one of the alternatives ('|' in a DTD, xs:choice)
        ALL,        // Каждый не более одного раза в любом порядке (xs:all) / Each at most once in any order (xs:all)
        WILDCARD    // Любой элемент из разрешённых пространств имён (xs:any)
                    // Any element from the allowed namespaces (xs:any)
    };

    // Ограничение пространств имён для xs:any
    // The namespace constraint for xs:any
    enum class WildcardMode
    {
        ANY_NAMESPACE,      // ##any
        OTHER_NAMESPACE,    // ##other — любое, кроме целевого и пустого
                            // ##other — any except the target one and the empty one
        NAMESPACE_LIST      // перечень URI (пустая строка — «без пространства»)
                            // a list of URIs (an empty string means "no namespace")
    };

    /*
    ** Particle — узел дерева модели содержимого (паттерн Composite).
    ** Одна и та же структура описывает и DTD (<!ELEMENT a (b, c*)>),
    ** и XML Schema (xs:sequence / xs:choice / xs:all), поэтому алгоритм
    ** сопоставления написан один раз для обоих валидаторов (DRY).
    **
    ** ELEMENT в режиме DTD сравнивается по полному имени (byQualifiedName),
    ** в режиме XSD — по паре «пространство имён + локальное имя».
    **
    ** Particle — a node of the content model tree (the Composite pattern).
    ** The same structure describes both a DTD (<!ELEMENT a (b, c*)>)
    ** and an XML Schema (xs:sequence / xs:choice / xs:all), so the matching
    ** algorithm is written once for both validators (DRY).
    ** ELEMENT in DTD mode is compared by the qualified name (byQualifiedName),
    ** in XSD mode — by the pair "namespace + local name".
    */
    struct Particle
    {
        ParticleKind kind = ParticleKind::SEQUENCE;
        std::size_t minOccurs = 1;
        std::size_t maxOccurs = 1;
        std::string name;
        std::string namespaceUri;
        bool byQualifiedName = false;
        WildcardMode wildcardMode = WildcardMode::ANY_NAMESPACE;
        std::vector<std::string> wildcardNamespaces;
        std::vector<Particle> children;
    };

    // Ребёнок проверяемого элемента: полное имя, локальное имя и пространство имён
    // A child of the validated element: the qualified name, the local name and the namespace
    struct ChildKey
    {
        std::string_view qualifiedName;
        std::string_view localName;
        std::string_view namespaceUri;
    };

    // Итог проверки: успех или индекс первого «лишнего» ребёнка (size() — «не хватает детей»)
    // The check result: success or the index of the first "extra" child (size() — "not enough children")
    struct MatchResult
    {
        bool matched;
        std::size_t failIndex;
    };

    // Соответствует ли последовательность детей модели
    // Whether the sequence of children matches the model
    MatchResult matchContent(const Particle& model, const std::vector<ChildKey>& children);

    // Подходит ли один ребёнок под частицу ELEMENT или WILDCARD
    // Whether one child fits an ELEMENT or WILDCARD particle
    bool particleAccepts(const Particle& particle, const ChildKey& child) noexcept;

    // Человекочитаемая запись модели для сообщений об ошибках: "(a, b*, (c | d))"
    // A human-readable notation of the model for error messages: "(a, b*, (c | d))"
    std::string describeParticle(const Particle& particle);
}

#endif
