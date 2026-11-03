#include "xsd/internal/assertion_recorder.hpp"
#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Тип узла запоминается по его адресу
    // The node type is remembered by its address
    void AssertionTypes::annotate(const void* node, NodeTyping typing)
    {
        _typings[node] = typing;
    }

    // Узлы вложенного мини-дерева теперь принадлежат этому — их типы переходят сюда же
    // The nodes of a nested mini-tree now belong to this one — their types move here as well
    void AssertionTypes::absorb(AssertionTypes& other)
    {
        _typings.merge(other._typings);
    }

    /*
    ** Атомизация узла: элемент или атрибут с известным простым типом —
    ** типизированное значение его текста; элемент «только из элементов» —
    ** ошибка XPath 2.0 (FOTY0012); всё прочее (текст, нетипизированные и
    ** непроверенные узлы, значения с ошибкой) — строковое значение.
    **
    ** Atomizing a node: an element or an attribute with a known simple type —
    ** the typed value of its text; an "element-only" element is the
    ** XPath 2.0 error (FOTY0012); everything else (text, untyped and
    ** unvalidated nodes, invalid values) — the string value.
    */
    xpath::XPathValue AssertionTypes::atomize(const xpath::XPathItem& item) const
    {
        const void* key = item.node;

        if (item.attribute != nullptr)
            key = item.attribute;
        auto found = _typings.find(key);
        if (item.isNamespace || found == _typings.end() || (found->second.type == nullptr && !found->second.elementOnly))
            return xpath::XPathValue::fromString(xpath::stringValue(item));
        if (found->second.elementOnly)
            xpath::detail::failEvaluation(i18n::format(i18n::Message::XPATH_ELEMENT_ONLY,
                {static_cast<const Element*>(item.node)->getName()}));
        return typedValueOf(*found->second.type, xpath::stringValue(item));
    }

    // Даты для конструкторов и сравнений со строками — по лексике XSD
    // Dates for constructors and comparisons with strings — by the XSD lexis
    bool AssertionTypes::castDate(xpath::ValueType type, std::string_view text, xpath::XPathValue& result) const
    {
        return parseDateAtom(type, text, result);
    }

    /*
    ** $value утверждения xs:assert (XSD 1.1, часть 1, 3.13.4.1, правило 3.1.2):
    ** у типа с простым содержимым — типизированное значение корня мини-дерева
    ** (тот же путь, что атомизация узла: тип корня уже отмечен в annotate),
    ** у прочих типов — пустая последовательность, а не «переменная не определена».
    **
    ** The $value of an xs:assert assertion (XSD 1.1, part 1, 3.13.4.1, rule 3.1.2):
    ** for a type with simple content — the typed value of the mini-tree root
    ** (the same path as node atomization: the root type is already recorded in annotate),
    ** for other types — the empty sequence rather than "the variable is not defined".
    */
    xpath::XPathValue assertValueOf(const AssertionCapture& capture)
    {
        if (capture.type->content != ContentType::SIMPLE_CONTENT)
            return xpath::XPathValue::fromNodeSet({});
        return capture.types.atomize(xpath::XPathItem{capture.root.get(), nullptr, nullptr});
    }
}
