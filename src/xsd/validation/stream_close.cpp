#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Закрытие элемента простого типа. Были дети-элементы — о них уже
    ** сообщено, и больше ничего не проверяется (как у обхода дерева).
    ** Иначе — отложенные атрибуты, затем значение накопленного текста.
    ** Возвращается тип значения, если оно корректно (для ключей).
    **
    ** Closing an element of a simple type. If there were child elements — they have already
    ** been reported, and nothing else is checked (as in the tree traversal).
    ** Otherwise — the deferred attributes, then the value of the accumulated text.
    ** The value type is returned if the value is valid (for keys).
    */
    const SimpleType* StreamValidator::closeSimple(ElementFrame& frame)
    {
        if (frame.hasChildElement)
            return nullptr;
        for (std::size_t index = 0; index < frame.pendingCount; ++index)
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_SIMPLE_WITH_ATTRIBUTE,
                {frame.pendingAttributes[index]}));
        return checkValue(*frame.valueType, _text, CheckPhase::VALUE);
    }

    /*
    ** Закрытие составного типа. Простое содержимое — значение текста (если
    ** детей-элементов не было). Иначе — текст там, где он запрещён, и
    ** завершение модели: не хватает ли обязательных детей.
    **
    ** Closing a complex type. Simple content — the text value (if
    ** there were no child elements). Otherwise — text where it is forbidden, and
    ** finishing the model: whether mandatory children are missing.
    */
    const SimpleType* StreamValidator::closeComplex(ElementFrame& frame)
    {
        const ComplexType& type = *frame.complex;

        if (type.content == ContentType::SIMPLE_CONTENT)
        {
            if (frame.hasChildElement || frame.valueType == nullptr)
                return nullptr;
            return checkValue(*frame.valueType, _text, CheckPhase::VALUE);
        }
        if (type.content != ContentType::MIXED_CONTENT && frame.significantText)
            report(_depth - 1, CheckPhase::VALUE, i18n::format(i18n::Message::XSD_TEXT_NOT_ALLOWED));
        if (type.hasParticle && !frame.modelFailed)
            finishModel(frame);
        return nullptr;
    }

    /*
    ** Итог модели после последнего ребёнка. Пошаговые способы уже
    ** сообщили бы о лишнем ребёнке, поэтому здесь — «не хватает детей»;
    ** отложенный (BUFFERED) может указать и на ребёнка — его путь и
    ** позиция были запомнены при открытии.
    **
    ** The model result after the last child. The step-by-step plans would already
    ** have reported an extra child, so here it is "children are missing";
    ** the deferred one (BUFFERED) may also point to a child — its path and
    ** position were remembered when it was opened.
    */
    void StreamValidator::finishModel(ElementFrame& frame)
    {
        content::MatchResult result = frame.cursor.finish();
        std::string model;

        if (result.matched)
            return;
        model = content::describeParticle(frame.complex->particle);
        if (result.failIndex < frame.bufferedCount)
        {
            const BufferedChild& child = frame.bufferedChildren[result.failIndex];
            _log.add(DiagnosticOrder{static_cast<std::size_t>(OrderGroup::ELEMENTS), frame.ordinal,
                static_cast<std::size_t>(CheckPhase::MODEL), 0, 0}, child.step,
                i18n::format(i18n::Message::XSD_CHILD_POSITION, {model}), child.position);
            return;
        }
        report(_depth - 1, CheckPhase::MODEL, i18n::format(i18n::Message::XSD_CHILDREN_MISSING, {model}));
    }

    // Способ проверки модели выбирается один раз на частицу за проход (подряд обычно идёт один тип)
    // The model checking plan is chosen once per particle per pass (usually one type comes in a row)
    content::CursorPlan StreamValidator::planFor(const content::Particle& particle)
    {
        if (_lastParticle == &particle)
            return _lastPlan;
        auto found = _plans.find(&particle);
        if (found == _plans.end())
            found = _plans.emplace(&particle, content::choosePlan(particle)).first;
        _lastParticle = &particle;
        _lastPlan = found->second;
        return _lastPlan;
    }
}
