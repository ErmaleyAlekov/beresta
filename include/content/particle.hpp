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

    // Частица без номера объявления (DTD, группы, частицы вне типа XSD)
    // A particle without a declaration number (a DTD, groups, particles outside an XSD type)
    inline constexpr std::size_t NO_SLOT = static_cast<std::size_t>(-1);

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

    // Имя элемента, который может стоять вместо частицы (член группы замены XSD)
    // The name of an element that may stand in place of a particle (a member of an XSD substitution group)
    struct ElementName
    {
        std::string localName;
        std::string namespaceUri;
    };

    /*
    ** Допустимые заменители головы группы замены, упорядоченные по паре
    ** (пространство имён, локальное имя): поиск — двоичный, за O(log n),
    ** даже для отраслевых схем с сотнями членов одной группы.
    **
    ** The admissible substitutes of a substitution group head, ordered by the pair
    ** (namespace, local name): the lookup is binary, in O(log n),
    ** even for industry schemas with hundreds of members in one group.
    */
    using SubstitutionGroup = std::vector<ElementName>;

    /*
    ** Particle — узел дерева модели содержимого (паттерн Composite).
    ** Одна и та же структура описывает и DTD (<!ELEMENT a (b, c*)>),
    ** и XML Schema (xs:sequence / xs:choice / xs:all), поэтому алгоритм
    ** сопоставления написан один раз для обоих валидаторов (DRY).
    **
    ** ELEMENT в режиме DTD сравнивается по полному имени (byQualifiedName),
    ** в режиме XSD — по паре «пространство имён + локальное имя».
    ** substitutes — группа замены глобального элемента (ссылка ref): она
    ** заполняется после загрузки всей схемы, поэтому частица хранит лишь
    ** указатель на неё. У DTD и у локальных элементов он пустой.
    **
    ** Particle — a node of the content model tree (the Composite pattern).
    ** The same structure describes both a DTD (<!ELEMENT a (b, c*)>)
    ** and an XML Schema (xs:sequence / xs:choice / xs:all), so the matching
    ** algorithm is written once for both validators (DRY).
    ** ELEMENT in DTD mode is compared by the qualified name (byQualifiedName),
    ** in XSD mode — by the pair "namespace + local name".
    ** substitutes is the substitution group of a global element (a ref reference): it
    ** is filled after the whole schema is loaded, so the particle keeps only
    ** a pointer to it. For a DTD and for local elements it is null.
    **
    ** slot — номер, по которому владелец модели находит своё описание листа:
    ** у XSD это индекс объявления элемента (ELEMENT) или режима
    ** processContents (WILDCARD) в составном типе. Сопоставление его не
    ** читает: курсор лишь сообщает, какой лист принял ребёнка.
    **
    ** slot is the number by which the model owner finds its own description of the leaf:
    ** for XSD it is the index of the element declaration (ELEMENT) or of the
    ** processContents mode (WILDCARD) in the complex type. Matching does not
    ** read it: the cursor only reports which leaf accepted the child.
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
        const SubstitutionGroup* substitutes = nullptr;
        std::size_t slot = NO_SLOT;
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

    // Есть ли у частицы непустая группа замены (такие модели не идут жадным путём; substitution.cpp)
    // Whether the particle has a non-empty substitution group (such models do not take the greedy path; substitution.cpp)
    bool hasSubstitutes(const Particle& particle) noexcept;

    // Входит ли ребёнок в упорядоченную группу замены (substitution.cpp)
    // Whether the child belongs to an ordered substitution group (substitution.cpp)
    bool isSubstitute(const SubstitutionGroup& group, const ChildKey& child) noexcept;

    // Порядок имён в группе замены: сначала пространство имён, затем локальное имя (substitution.cpp)
    // The order of names in a substitution group: the namespace first, then the local name (substitution.cpp)
    bool operator<(const ElementName& left, const ElementName& right) noexcept;

    // Человекочитаемая запись модели для сообщений об ошибках: "(a, b*, (c | d))"
    // A human-readable notation of the model for error messages: "(a, b*, (c | d))"
    std::string describeParticle(const Particle& particle);
}

#endif
