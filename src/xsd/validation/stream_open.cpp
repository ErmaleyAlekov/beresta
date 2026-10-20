#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Новый кадр на вершине стека: порядковый номер (ключ порядка
    ** сообщений), метка начала элемента, уровень пути и объявления
    ** префиксов. Метка берётся сразу: ошибки, найденные при закрытии,
    ** тоже указывают на начало элемента; строка и столбец по ней будут
    ** посчитаны лишь для ошибок. Текст родителя больше не копится.
    **
    ** A new frame on the top of the stack: the ordinal (the message order
    ** key), the mark of the element start, the path level and the prefix
    ** declarations. The mark is taken at once: errors found on closing
    ** also point to the element start; the line and column by it will be
    ** computed only for errors. The parent's text is no longer accumulated.
    */
    void StreamValidator::openFrame(const SaxElement& element)
    {
        if (_depth == _frames.size())
            _frames.emplace_back();
        ElementFrame& frame = _frames[_depth++];
        frame.ordinal = _nextOrdinal++;
        frame.position = EventPosition();
        if (_locator != nullptr)
            frame.position.mark = _locator->getMark();
        if (_locator != nullptr && frame.position.mark == Locator::NO_MARK)
            frame.position.location = _locator->getLocation();
        frame.mode = FrameMode::UNCHECKED;
        frame.declaration = nullptr;
        frame.complex = nullptr;
        frame.valueType = nullptr;
        frame.hasChildElement = frame.hasChildNode = frame.significantText = false;
        frame.collectText = frame.modelFailed = frame.reportedChildren = false;
        frame.childCount = frame.pendingCount = frame.bufferedCount = 0;
        frame.attributeTypes.clear();
        frame.untypedAttributes.clear();
        frame.defaultAttributes.clear();
        _paths.open(element.name);
        frame.declaresNamespaces = _prefixes.push(element.attributes);
        _text.clear();
    }

    // Корень обязан соответствовать глобальному объявлению элемента схемы (или иметь xsi:type)
    // The root must match a global element declaration of the schema (or have xsi:type)
    const ElementDecl* StreamValidator::admitRoot(const SaxElement& element)
    {
        const ElementDecl* declaration = _schema.findElement(element.namespaceUri, element.localName);

        _rootPosition = _frames[0].position;
        if (declaration == nullptr)
            declaration = typeOnlyDeclaration(element);
        if (declaration == nullptr)
            report(0, CheckPhase::XSI_TYPE, i18n::format(i18n::Message::XSD_ROOT_UNDECLARED,
                {element.namespaceUri, element.localName}));
        return declaration;
    }

    /*
    ** Родитель принимает ребёнка. У простого типа детей быть не может, у
    ** простого содержимого — тоже (сообщается один раз, дети не
    ** проверяются); у составного — шаг модели и объявление ребёнка.
    **
    ** The parent admits a child. A simple type cannot have children, nor can
    ** simple content (reported once, the children are not
    ** validated); for a complex type — a model step and the child's declaration.
    */
    const ElementDecl* StreamValidator::admitChild(const SaxElement& element)
    {
        std::size_t level = _depth - 2;
        ElementFrame& parent = _frames[level];

        parent.hasChildElement = parent.hasChildNode = true;
        ++parent.childCount;
        if (parent.mode == FrameMode::SIMPLE && !parent.reportedChildren)
            report(level, CheckPhase::SIMPLE_CHILDREN, i18n::format(i18n::Message::XSD_SIMPLE_WITH_CHILDREN,
                {parent.valueType->name}));
        if (parent.mode == FrameMode::SIMPLE)
            parent.reportedChildren = true;
        if (parent.mode != FrameMode::COMPLEX)
            return nullptr;
        if (parent.complex->content != ContentType::SIMPLE_CONTENT)
        {
            stepModel(parent, element);
            return childDeclaration(*parent.complex, element);
        }
        if (!parent.reportedChildren)
            report(level, CheckPhase::VALUE, i18n::format(i18n::Message::XSD_SIMPLE_CONTENT_CHILDREN));
        parent.reportedChildren = true;
        return nullptr;
    }

    /*
    ** Шаг модели содержимого родителя. Первый недопустимый ребёнок — место
    ** ошибки (как самая дальняя позиция общего алгоритма); дальше модель
    ** не проверяется. При отложенной проверке (BUFFERED) запоминается, где
    ** сообщить о ребёнке, если в конце он окажется лишним.
    **
    ** A step of the parent's content model. The first disallowed child is the error
    ** location (like the farthest position of the general algorithm); after that the model
    ** is not checked. With a deferred check (BUFFERED) it is remembered where
    ** to report the child if at the end it turns out to be extra.
    */
    void StreamValidator::stepModel(ElementFrame& parent, const SaxElement& element)
    {
        if (!parent.complex->hasParticle)
        {
            if (!parent.reportedChildren)
                report(_depth - 2, CheckPhase::MODEL, i18n::format(i18n::Message::XSD_CHILDREN_NOT_ALLOWED));
            parent.reportedChildren = true;
            return;
        }
        if (parent.plan == content::CursorPlan::BUFFERED)
        {
            if (parent.bufferedCount == parent.bufferedChildren.size())
                parent.bufferedChildren.emplace_back();
            parent.bufferedChildren[parent.bufferedCount++] = BufferedChild{_paths.pin(_depth - 1),
                _frames[_depth - 1].position};
        }
        if (parent.cursor.step(content::ChildKey{element.name, element.localName, element.namespaceUri})
            || parent.modelFailed)
            return;
        parent.modelFailed = true;
        reportAtChild(CheckPhase::MODEL, i18n::format(i18n::Message::XSD_CHILD_POSITION,
            {content::describeParticle(parent.complex->particle)}));
    }

    /*
    ** Объявление ребёнка: из модели типа по паре (пространство имён,
    ** локальное имя); затем — член группы замены одного из элементов
    ** модели: у него своё глобальное объявление (свой тип), а не головы;
    ** иначе — для xs:any с lax / strict — глобальное, а без него — тип
    ** из xsi:type. Ненайденное при strict — ошибка у ребёнка. Второй проход идёт, только если имя не
    ** нашлось в модели, и смотрит лишь на головы с непустыми группами.
    **
    ** The child's declaration: from the type model by the pair (namespace,
    ** local name); then — a member of the substitution group of one of the model's
    ** elements: it has its own global declaration (its own type), not the head's;
    ** otherwise — for xs:any with lax / strict — a global one, and without it — the type
    ** from xsi:type. One not found with strict is an error at the child. The second pass runs only if the name was not
    ** found in the model, and looks only at heads with non-empty groups.
    */
    const ElementDecl* StreamValidator::childDeclaration(const ComplexType& type, const SaxElement& element)
    {
        for (const ElementDecl* declaration : type.elements)
        {
            if (declaration->name == element.localName && declaration->namespaceUri == element.namespaceUri)
                return declaration;
        }
        content::ChildKey key{element.name, element.localName, element.namespaceUri};
        for (const ElementDecl* declaration : type.elements)
        {
            if (!declaration->substitutes.empty() && content::isSubstitute(declaration->substitutes, key))
                return _schema.findElement(element.namespaceUri, element.localName);
        }
        const ElementDecl* global = nullptr;
        if (type.hasElementWildcard && type.elementWildcard != ProcessContents::SKIP_CHECK)
            global = _schema.findElement(element.namespaceUri, element.localName);
        if (global == nullptr && type.hasElementWildcard && type.elementWildcard != ProcessContents::SKIP_CHECK)
            global = typeOnlyDeclaration(element);
        if (global == nullptr && type.hasElementWildcard && type.elementWildcard == ProcessContents::STRICT_CHECK)
            reportAtChild(CheckPhase::WILDCARD, i18n::format(i18n::Message::XSD_WILDCARD_UNDECLARED));
        return global;
    }
}
