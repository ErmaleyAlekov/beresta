#include "xsd/internal/type_derivation.hpp"

#include <cstddef>

namespace beresta::xsd::detail
{
    // Предел длины цепочки наследования: защита от зацикливания на испорченной схеме
    // The limit of the derivation chain length: protection against looping on a broken schema
    static const std::size_t MAX_DERIVATION_DEPTH = 256;

    /*
    ** Простой тип из простого (раздел 3.14.6): тот же тип; иначе каждый
    ** шаг — ограничение, и оно не должно быть запрещено. Из anySimpleType
    ** выведен любой простой тип (у list и union он базовый). Объединение
    ** принимает и типы, выведенные из любого его члена.
    **
    ** A simple type from a simple one (section 3.14.6): the same type; otherwise every
    ** step is a restriction, and it must not be prohibited. Any simple type is derived
    ** from anySimpleType (it is the base of list and union). A union
    ** also accepts the types derived from any of its members.
    */
    static bool simpleDerivedFrom(const SimpleType* derived, const SimpleType* base, DerivationSet blocked,
        std::size_t depth) noexcept
    {
        if (derived == base)
            return true;
        if ((blocked & DERIVATION_RESTRICTION) != 0 || depth > MAX_DERIVATION_DEPTH)
            return false;
        if (base->primitive == Primitive::ANY_SIMPLE && base->baseType == nullptr)
            return true;
        std::size_t steps = 0;
        for (const SimpleType* step = derived->baseType; step != nullptr && steps < MAX_DERIVATION_DEPTH;
            step = step->baseType)
        {
            if (step == base)
                return true;
            ++steps;
        }
        if (base->variety != Variety::UNION)
            return false;
        for (const SimpleType* member : base->memberTypes)
        {
            if (simpleDerivedFrom(derived, member, blocked, depth + 1))
                return true;
        }
        return false;
    }

    // Шаг цепочки дошёл до простого типа: составной базовый недостижим, xs:anyType — достижим ограничением
    // The chain step reached a simple type: a complex base is unreachable, xs:anyType is reachable by restriction
    static bool simpleStepDerived(const SimpleType* derived, TypeRef base, DerivationSet blocked) noexcept
    {
        if (base.complex != nullptr)
            return false;
        if (base.simple == nullptr)
            return (blocked & DERIVATION_RESTRICTION) == 0;
        return simpleDerivedFrom(derived, base.simple, blocked, 0);
    }

    // Ссылки равны, если указывают на один и тот же простой или составной тип
    // References are equal if they point to the same simple or complex type
    bool sameType(TypeRef left, TypeRef right) noexcept
    {
        return left.simple == right.simple && left.complex == right.complex;
    }

    /*
    ** Составной тип (раздел 3.4.6): поднимаемся по базовым типам, пока не
    ** встретим base. Способ каждого шага (extension / restriction) не должен
    ** входить в blocked. Дошли до xs:anyType, не встретив base, — не выведен;
    ** дошли до простого типа (simpleContent) — дальше правило простых типов.
    **
    ** A complex type (section 3.4.6): we climb the base types until we
    ** meet base. The method of every step (extension / restriction) must not
    ** be in blocked. Reaching xs:anyType without meeting base means not derived;
    ** reaching a simple type (simpleContent) — the simple type rule goes on from there.
    */
    bool isDerivedFrom(TypeRef derived, TypeRef base, DerivationSet blocked) noexcept
    {
        TypeRef step = derived;

        for (std::size_t depth = 0; depth < MAX_DERIVATION_DEPTH; ++depth)
        {
            if (sameType(step, base))
                return true;
            if (step.simple != nullptr)
                return simpleStepDerived(step.simple, base, blocked);
            if (step.complex == nullptr)
                return false;
            if ((step.complex->derivationMethod & blocked) != 0)
                return false;
            step = step.complex->baseType;
        }
        return false;
    }
}
