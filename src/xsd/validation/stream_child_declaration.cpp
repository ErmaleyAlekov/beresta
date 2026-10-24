#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Объявление ребёнка берётся у листа модели, который его принял
    ** (cvc-complex-type.2.4: ребёнок проверяется по частице, с которой он
    ** сопоставлен). Поиск по имени давал неверный ответ, когда в модели
    ** несколько одноимённых элементов с разными типами или fixed
    ** (addB149, isDefault079) или когда тот же элемент принимает xs:any
    ** со своим processContents (QFE1700c2, wildI005). Если курсор листа
    ** не знает (способ BUFFERED, ошибка модели), объявление ищется по имени.
    **
    ** The declaration of a child is taken from the model leaf that accepted it
    ** (cvc-complex-type.2.4: a child is validated against the particle it is
    ** matched with). A lookup by name gave a wrong answer when the model has
    ** several elements of the same name with different types or fixed
    ** (addB149, isDefault079) or when the same element is accepted by an xs:any
    ** with its own processContents (QFE1700c2, wildI005). If the cursor does not know
    ** the leaf (the BUFFERED plan, a model error), the declaration is looked up by name.
    */
    const ElementDecl* StreamValidator::childDeclaration(const ComplexType& type, const SaxElement& element,
        const content::Particle* leaf)
    {
        bool known = leaf != nullptr && leaf->slot != content::NO_SLOT;

        if (known && leaf->kind == content::ParticleKind::ELEMENT && leaf->slot < type.elements.size())
            return leafDeclaration(*type.elements[leaf->slot], element);
        if (known && leaf->kind == content::ParticleKind::WILDCARD && leaf->slot < type.wildcards.size())
            return wildcardDeclaration(element, type.wildcards[leaf->slot]);
        return declarationByName(type, element);
    }

    /*
    ** Лист-элемент принял ребёнка под своим именем или как члена своей
    ** группы замены: у члена своё глобальное объявление (свой тип), а не головы.
    **
    ** An element leaf accepted the child under its own name or as a member of its
    ** substitution group: a member has its own global declaration (its own type), not the head's.
    */
    const ElementDecl* StreamValidator::leafDeclaration(const ElementDecl& declaration,
        const SaxElement& element) const
    {
        if (declaration.name == element.localName && declaration.namespaceUri == element.namespaceUri)
            return &declaration;
        return _schema.findElement(element.namespaceUri, element.localName);
    }

    /*
    ** Прежний поиск по имени: сначала объявления модели по паре
    ** (пространство имён, локальное имя), затем члены групп замены (только
    ** у голов с непустыми группами), затем xs:any типа с режимом последнего
    ** шаблона.
    **
    ** The former lookup by name: first the model declarations by the pair
    ** (namespace, local name), then the members of substitution groups (only
    ** at heads with non-empty groups), then the type's xs:any with the mode of the last
    ** wildcard.
    */
    const ElementDecl* StreamValidator::declarationByName(const ComplexType& type, const SaxElement& element)
    {
        content::ChildKey key{element.name, element.localName, element.namespaceUri};

        for (const ElementDecl* declaration : type.elements)
        {
            if (declaration->name == element.localName && declaration->namespaceUri == element.namespaceUri)
                return declaration;
        }
        for (const ElementDecl* declaration : type.elements)
        {
            if (!declaration->substitutes.empty() && content::isSubstitute(declaration->substitutes, key))
                return _schema.findElement(element.namespaceUri, element.localName);
        }
        if (!type.hasElementWildcard)
            return nullptr;
        return wildcardDeclaration(element, type.elementWildcard);
    }

    /*
    ** Ребёнок xs:any: при skip не проверяется; при lax / strict — по
    ** глобальному объявлению, а без него — по типу из xsi:type.
    ** Ненайденное при strict — ошибка у ребёнка.
    **
    ** A child of xs:any: with skip it is not validated; with lax / strict — against
    ** the global declaration, and without it — against the type from xsi:type.
    ** One not found with strict is an error at the child.
    */
    const ElementDecl* StreamValidator::wildcardDeclaration(const SaxElement& element, ProcessContents mode)
    {
        const ElementDecl* global = nullptr;

        if (mode == ProcessContents::SKIP_CHECK)
            return nullptr;
        global = _schema.findElement(element.namespaceUri, element.localName);
        if (global == nullptr)
            global = typeOnlyDeclaration(element);
        if (global == nullptr && mode == ProcessContents::STRICT_CHECK)
            reportAtChild(CheckPhase::WILDCARD, i18n::format(i18n::Message::XSD_WILDCARD_UNDECLARED));
        return global;
    }
}
