#include "dtd/dtd_validator.hpp"

#include <utility>

namespace beresta::dtd
{
    // Валидатор не владеет моделью: её время жизни обеспечивает сессия разбора
    DtdValidator::DtdValidator(const DtdModel& model, std::string rootName,
        ValidityReporter& reporter)
        : _model(model), _rootName(std::move(rootName)), _reporter(reporter),
          _stack(), _ids(), _references(), _rootChecked(false)
    {
    }

    /*
    ** Открытие элемента: проверка корня, допустимости в родителе,
    ** объявленности и атрибутов; затем элемент становится текущим.
    */
    void DtdValidator::startElement(std::string_view name, const std::vector<Attribute>& attributes)
    {
        if (!_rootChecked && name != _rootName)
            _reporter.reportValidity("корневой элемент <" + std::string(name)
                + "> не совпадает с объявленным в DOCTYPE <" + _rootName + ">");
        _rootChecked = true;
        if (!_stack.empty())
            checkChild(_stack.back(), name);
        const ElementDeclaration* declaration = _model.findElement(name);
        if (declaration == nullptr)
            _reporter.reportValidity("элемент <" + std::string(name) + "> не объявлен в DTD");
        checkAttributes(name, attributes);
        _stack.push_back(OpenState{std::string(name), declaration, {}});
    }

    /*
    ** Текст: в EMPTY запрещён любой (даже пробельный), в модели из
    ** элементов (CHILDREN) — любой непробельный.
    */
    void DtdValidator::text(bool significant)
    {
        if (_stack.empty() || _stack.back().declaration == nullptr)
            return;
        const OpenState& state = _stack.back();
        ContentKind kind = state.declaration->kind;
        if (kind == ContentKind::EMPTY_CONTENT)
            _reporter.reportValidity("элемент <" + state.name + "> объявлен EMPTY, но содержит текст");
        else if (kind == ContentKind::CHILDREN && significant)
            _reporter.reportValidity("элемент <" + state.name
                + "> может содержать только элементы, но содержит текст");
    }

    // Закрытие: для модели из элементов проверяется вся последовательность детей
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
    void DtdValidator::finish()
    {
        for (const std::string& reference : _references)
        {
            if (_ids.find(reference) == _ids.end())
                _reporter.reportValidity("IDREF \"" + reference + "\" ссылается на несуществующий ID");
        }
    }
}
