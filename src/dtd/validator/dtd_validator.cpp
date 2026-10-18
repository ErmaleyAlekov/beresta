#include "dtd/dtd_validator.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::dtd
{
    // Валидатор не владеет моделью: её время жизни обеспечивает сессия разбора
    // The validator does not own the model: the parse session ensures its lifetime
    DtdValidator::DtdValidator(const DtdModel& model, std::string rootName,
        ValidityReporter& reporter, bool namespaceAware, bool standalone)
        : _model(model), _rootName(std::move(rootName)), _reporter(reporter),
          _stack(), _ids(), _references(), _rootChecked(false), _namespaceAware(namespaceAware),
          _standalone(standalone)
    {
    }

    /*
    ** Открытие элемента: проверка корня, допустимости в родителе,
    ** объявленности и атрибутов; затем элемент становится текущим.
    **
    ** Opening an element: checks the root, whether it is allowed in the parent,
    ** whether it is declared, and the attributes; then the element becomes current.
    */
    void DtdValidator::startElement(std::string_view name, const std::vector<Attribute>& attributes)
    {
        if (!_rootChecked && name != _rootName)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_ROOT_MISMATCH, {name, _rootName}));
        _rootChecked = true;
        if (!_stack.empty())
            checkChild(_stack.back(), name);
        const ElementDeclaration* declaration = _model.findElement(name);
        if (declaration == nullptr)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_ELEMENT_UNDECLARED, {name}));
        checkAttributes(name, attributes);
        _stack.push_back(OpenState{std::string(name), declaration, {}});
    }

    /*
    ** Текст: в EMPTY запрещён любой (даже пробельный), в модели из
    ** элементов (CHILDREN) — любой непробельный.
    **
    ** Text: in EMPTY any text (even whitespace) is forbidden, in a model of
    ** elements (CHILDREN) — any non-whitespace text.
    */
    void DtdValidator::text(bool significant)
    {
        if (_stack.empty() || _stack.back().declaration == nullptr)
            return;
        const OpenState& state = _stack.back();
        ContentKind kind = state.declaration->kind;
        if (kind == ContentKind::EMPTY_CONTENT)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_EMPTY_WITH_TEXT, {state.name}));
        else if (kind == ContentKind::CHILDREN && significant)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_CHILDREN_WITH_TEXT, {state.name}));
        else if (kind == ContentKind::CHILDREN && _standalone && state.declaration->externallyDeclared)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_STANDALONE_WHITESPACE, {state.name}));
    }

    // Закрытие: для модели из элементов проверяется вся последовательность детей
    // Closing: for a model of elements the whole sequence of children is checked
    void DtdValidator::endElement()
    {
        if (_stack.empty())
            return;
        const OpenState& state = _stack.back();
        if (state.declaration != nullptr && state.declaration->kind == ContentKind::CHILDREN)
            checkContent(state);
        _stack.pop_back();
    }

    // Каждый IDREF обязан указывать на существующий в документе ID
    // Every IDREF must point to an ID that exists in the document
    void DtdValidator::finish()
    {
        for (const std::string& reference : _references)
        {
            if (_ids.find(reference) == _ids.end())
                _reporter.reportValidity(i18n::format(i18n::Message::IDREF_DANGLING, {reference}));
        }
    }
}
