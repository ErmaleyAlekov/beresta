#ifndef XSD_TYPE_DERIVATION_HPP
#define XSD_TYPE_DERIVATION_HPP

#include "xsd/schema.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Выведен ли тип derived из типа base, если способы наследования из
    ** blocked запрещены (правила «Type Derivation OK (Complex / Simple)»
    ** спецификации XSD 1.0, разделы 3.4.6 и 3.14.6). Каждый шаг цепочки
    ** базовых типов не должен использовать запрещённый способ. Пустая
    ** ссылка TypeRef — xs:anyType: из него выведено всё.
    ** Используется и при загрузке схемы (группы замены, final), и при
    ** проверке документа (xsi:type, block) — правило одно.
    **
    ** Whether the type derived is derived from the type base when the derivation
    ** methods from blocked are prohibited (the "Type Derivation OK (Complex / Simple)"
    ** rules of the XSD 1.0 specification, sections 3.4.6 and 3.14.6). Every step of the
    ** base type chain must not use a prohibited method. An empty
    ** TypeRef is xs:anyType: everything is derived from it.
    ** Used both at schema loading (substitution groups, final) and at
    ** document validation (xsi:type, block) — the rule is one.
    */
    bool isDerivedFrom(TypeRef derived, TypeRef base, DerivationSet blocked) noexcept;

    // Одинаковы ли две ссылки на тип (xs:anyType равен только себе)
    // Whether two type references are the same (xs:anyType equals only itself)
    bool sameType(TypeRef left, TypeRef right) noexcept;
}

#endif
