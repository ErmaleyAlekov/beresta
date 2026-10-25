#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace beresta::xsd::detail
{
    /*
    ** Место ребёнка по схеме для схем — правила, которые не зависят от
    ** порядка детей:
    **  - внутри xs:annotation — только xs:appinfo и xs:documentation
    **    (аннотация в аннотации и любой компонент там — ошибка);
    **  - xs:notation — только прямой ребёнок xs:schema (или xs:override):
    **    их дети сюда не попадают, поэтому любая нотация здесь не на месте;
    **  - внутри xs:notation — только xs:annotation.
    ** Вызывается из checkChildOrder для каждого ребёнка из пространства XSD.
    **
    ** The place of a child by the schema for schemas — the rules that do not depend
    ** on the order of the children:
    **  - inside xs:annotation — only xs:appinfo and xs:documentation
    **    (an annotation inside an annotation and any component there is an error);
    **  - xs:notation is only a direct child of xs:schema (or xs:override):
    **    their children do not get here, so any notation here is misplaced;
    **  - inside xs:notation — only xs:annotation.
    ** It is called from checkChildOrder for every child of the XSD namespace.
    */
    void SchemaBuilder::checkSchemaPlacement(const Element& parent, const Element& child) const
    {
        std::string_view parentKind = parent.getLocalName();
        std::string_view kind = child.getLocalName();
        bool misplaced = kind == "notation";

        if (parentKind == "annotation" && kind != "appinfo" && kind != "documentation")
            misplaced = true;
        if (parentKind == "notation" && kind != "annotation")
            misplaced = true;
        if (misplaced)
            fail(child, i18n::format(i18n::Message::SCHEMA_CHILD_NOT_ALLOWED,
                {std::string(kind), std::string(parentKind)}));
    }

    /*
    ** Объявление нотации: имя (обязательное NCName) и его повтор уже проверены
    ** при регистрации. Здесь — идентификатор: нужен хотя бы один из public и
    ** system. В 1-й редакции XSD 1.0 public был обязателен, исправление 2-й
    ** редакции разрешило нотацию только с system (случай errC004 набора W3C),
    ** но нотация совсем без идентификатора по-прежнему ошибка.
    **
    ** A notation declaration: the name (a mandatory NCName) and its repetition are
    ** already checked at registration. Here — the identifier: at least one of public
    ** and system is needed. In the 1st edition of XSD 1.0 public was mandatory, an
    ** erratum of the 2nd edition allowed a notation with only system (the errC004 case
    ** of the W3C suite), but a notation with no identifier at all is still an error.
    */
    void SchemaBuilder::checkNotation(const Element& notation) const
    {
        if (!notation.hasAttribute("public") && !notation.hasAttribute("system"))
            fail(notation, i18n::format(i18n::Message::SCHEMA_NOTATION_NO_IDENTIFIER,
                {attributeOr(notation, "name", "")}));
    }
}
