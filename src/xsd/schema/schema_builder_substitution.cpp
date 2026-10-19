#include "xsd/internal/schema_builder.hpp"
#include "xsd/internal/type_derivation.hpp"
#include "i18n/i18n.hpp"

#include <algorithm>

namespace beresta::xsd::detail
{
    /*
    ** Группы замены вычисляются один раз, когда построены все глобальные
    ** элементы: сначала типы, унаследованные от голов, затем для каждого
    ** члена — проверка final его головы и добавление во множества всех
    ** голов цепочки. Множества упорядочиваются для двоичного поиска.
    ** Паттерн — предвычисление (Precomputation): проверка документа
    ** только ищет имя в готовом множестве, не обходя цепочки.
    **
    ** Substitution groups are computed once, when all the global
    ** elements are built: first the types inherited from heads, then for every
    ** member — the check of its head's final and adding it to the sets of all the
    ** heads of the chain. The sets are ordered for a binary search.
    ** The pattern is Precomputation: document validation
    ** only looks up a name in a ready set, without walking the chains.
    */
    void SchemaBuilder::buildSubstitutionGroups()
    {
        inheritHeadTypes();
        for (const auto& entry : _builtElements)
        {
            if (entry.second->substitutionHead == nullptr)
                continue;
            checkSubstitutionFinal(*entry.second);
            addSubstitute(*entry.second);
        }
        for (const auto& entry : _builtElements)
            std::sort(entry.second->substitutes.begin(), entry.second->substitutes.end());
    }

    /*
    ** Элемент без собственного типа получает тип головы. Голова при его
    ** построении могла ещё строиться, поэтому тип берётся сейчас — у
    ** первой головы цепочки, у которой тип свой. Цепочка длиннее числа
    ** элементов — значит, она замкнута в кольцо: ошибка схемы.
    **
    ** An element without its own type gets the head's type. The head could still
    ** be under construction when it was built, so the type is taken now — from
    ** the first head of the chain that has its own type. A chain longer than the number
    ** of elements means it is closed into a ring: a schema error.
    */
    void SchemaBuilder::inheritHeadTypes()
    {
        for (const auto& entry : _builtElements)
        {
            if (_headTyped.count(entry.second) == 0)
                continue;
            const ElementDecl* head = entry.second->substitutionHead;
            std::size_t steps = 0;
            while (head != nullptr && _headTyped.count(head) != 0 && steps++ <= _builtElements.size())
                head = head->substitutionHead;
            if (steps > _builtElements.size())
                fail(*_sources.front().root, i18n::format(i18n::Message::SCHEMA_SUBSTITUTION_CYCLE,
                    {entry.second->name}));
            entry.second->type = TypeRef();
            if (head != nullptr)
                entry.second->type = head->type;
        }
    }

    // Тип члена обязан быть выведен из типа головы способами, не запрещёнными final головы
    // The member's type must be derived from the head's type by methods not prohibited by the head's final
    void SchemaBuilder::checkSubstitutionFinal(const ElementDecl& member)
    {
        const ElementDecl& head = *member.substitutionHead;

        if (isDerivedFrom(member.type, head.type, head.finalSet))
            return;
        // Ошибка указывает на объявление члена, если оно найдено
        // The error points to the member's declaration if it is found
        const Definition* definition = findDefinition("element", QualifiedKey(member.namespaceUri, member.name));
        const Element* where = _sources.front().root;
        if (definition != nullptr)
            where = definition->element;
        fail(*where, i18n::format(i18n::Message::SCHEMA_SUBSTITUTION_FINAL, {member.name, head.name}));
    }

    /*
    ** Член попадает во множество каждой головы цепочки (транзитивно), если
    ** он не абстрактный, голова не запрещает замену (block="substitution"),
    ** а его тип выведен из типа головы способами, не запрещёнными block
    ** головы и её типа (Substitution Group OK (Transitive), раздел 3.3.6).
    **
    ** A member gets into the set of every head of the chain (transitively) if
    ** it is not abstract, the head does not prohibit substitution (block="substitution"),
    ** and its type is derived from the head's type by methods not prohibited by the block
    ** of the head and of its type (Substitution Group OK (Transitive), section 3.3.6).
    */
    void SchemaBuilder::addSubstitute(const ElementDecl& member)
    {
        std::size_t steps = 0;

        for (const ElementDecl* head = member.substitutionHead; head != nullptr; head = head->substitutionHead)
        {
            if (head == &member || ++steps > _builtElements.size())
                fail(*_sources.front().root, i18n::format(i18n::Message::SCHEMA_SUBSTITUTION_CYCLE,
                    {member.name}));
            DerivationSet blocked = head->blockSet;
            if (head->type.complex != nullptr)
                blocked |= head->type.complex->blockSet;
            if (member.isAbstract || (blocked & DERIVATION_SUBSTITUTION) != 0
                || !isDerivedFrom(member.type, head->type, blocked))
                continue;
            ElementDecl* target = _builtElements[QualifiedKey(head->namespaceUri, head->name)];
            target->substitutes.push_back(content::ElementName{member.name, member.namespaceUri});
        }
    }
}
