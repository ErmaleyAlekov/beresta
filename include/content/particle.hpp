#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::content
{
    // Значение maxOccurs="unbounded" (и '*' / '+' в DTD)
    inline constexpr std::size_t UNBOUNDED = static_cast<std::size_t>(-1);

    // Вид частицы модели содержимого
    enum class ParticleKind
    {
        ELEMENT,    // Конкретный элемент
        SEQUENCE,   // Дети строго по порядку (',' в DTD, xs:sequence)
        CHOICE,     // Ровно один из вариантов ('|' в DTD, xs:choice)
        ALL,        // Каждый не более одного раза в любом порядке (xs:all)
        WILDCARD    // Любой элемент из разрешённых пространств имён (xs:any)
    };

    // Ограничение пространств имён для xs:any
    enum class WildcardMode
    {
        ANY_NAMESPACE,      // ##any
        OTHER_NAMESPACE,    // ##other — любое, кроме целевого и пустого
        NAMESPACE_LIST      // перечень URI (пустая строка — «без пространства»)
    };

    /*
    ** Particle — узел дерева модели содержимого (паттерн Composite).
    ** Одна и та же структура описывает и DTD (<!ELEMENT a (b, c*)>),
    ** и XML Schema (xs:sequence / xs:choice / xs:all), поэтому алгоритм
    ** сопоставления написан один раз для обоих валидаторов (DRY).
    **
    ** ELEMENT в режиме DTD сравнивается по полному имени (byQualifiedName),
    ** в режиме XSD — по паре «пространство имён + локальное имя».
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
    struct ChildKey
    {
        std::string_view qualifiedName;
        std::string_view localName;
        std::string_view namespaceUri;
    };

    // Итог проверки: успех или индекс первого «лишнего» ребёнка (size() — «не хватает детей»)
    struct MatchResult
    {
        bool matched;
        std::size_t failIndex;
    };

    // Соответствует ли последовательность детей модели
    MatchResult matchContent(const Particle& model, const std::vector<ChildKey>& children);

    // Подходит ли один ребёнок под частицу ELEMENT или WILDCARD
    bool particleAccepts(const Particle& particle, const ChildKey& child) noexcept;

    // Человекочитаемая запись модели для сообщений об ошибках: "(a, b*, (c | d))"
    std::string describeParticle(const Particle& particle);
}

#endif
