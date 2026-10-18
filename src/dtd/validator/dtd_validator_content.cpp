#include "dtd/dtd_validator.hpp"
#include "i18n/i18n.hpp"
#include "content/particle.hpp"

namespace beresta::dtd
{
    // Любая разметка внутри EMPTY нарушает VC: Element Valid (уточнено errata E15)
    // Any markup inside EMPTY violates VC: Element Valid (clarified by errata E15)
    void DtdValidator::markup()
    {
        if (_stack.empty() || _stack.back().declaration == nullptr)
            return;
        const OpenState& state = _stack.back();

        if (state.declaration->kind == ContentKind::EMPTY_CONTENT)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_EMPTY_NOT_EMPTY, {state.name}));
    }

    // Виртуальный деструктор интерфейса
    // The virtual destructor of the interface
    ValidityReporter::~ValidityReporter()
    {
    }

    /*
    ** Немедленные проверки ребёнка: в EMPTY детей быть не может, в MIXED
    ** допустимы только перечисленные элементы. Для CHILDREN имя лишь
    ** запоминается — модель проверяется целиком при закрытии родителя.
    **
    ** Immediate checks of a child: EMPTY cannot have children, in MIXED only
    ** the listed elements are allowed. For CHILDREN the name is only
    ** remembered — the model is checked as a whole when the parent is closed.
    */
    void DtdValidator::checkChild(OpenState& parent, std::string_view child)
    {
        if (parent.declaration == nullptr)
            return;
        ContentKind kind = parent.declaration->kind;
        if (kind == ContentKind::EMPTY_CONTENT)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_EMPTY_WITH_CHILD, {parent.name, child}));
        if (kind == ContentKind::CHILDREN)
            parent.children.push_back(std::string(child));
        if (kind != ContentKind::MIXED)
            return;
        for (const std::string& allowed : parent.declaration->mixedNames)
        {
            if (allowed == child)
                return;
        }
        _reporter.reportValidity(i18n::format(i18n::Message::DTD_CHILD_NOT_ALLOWED, {child, parent.name}));
    }

    // Сопоставление последовательности детей с моделью общим алгоритмом content::matchContent
    // Matches the sequence of children against the model with the common content::matchContent algorithm
    void DtdValidator::checkContent(const OpenState& state)
    {
        std::vector<content::ChildKey> keys;

        keys.reserve(state.children.size());
        for (const std::string& child : state.children)
            keys.push_back(content::ChildKey{child, child, std::string_view()});
        content::MatchResult result = content::matchContent(state.declaration->model, keys);
        if (result.matched)
            return;
        std::string model = content::describeParticle(state.declaration->model);
        if (result.failIndex < keys.size())
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_CHILD_POSITION, {state.children[result.failIndex], state.name, model}));
        else
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_CHILDREN_MISSING, {state.name, model}));
    }
}
