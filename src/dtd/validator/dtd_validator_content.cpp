#include "dtd/dtd_validator.hpp"
#include "content/particle.hpp"

namespace beresta::dtd
{
    // Виртуальный деструктор интерфейса
    ValidityReporter::~ValidityReporter()
    {
    }

    /*
    ** Немедленные проверки ребёнка: в EMPTY детей быть не может, в MIXED
    ** допустимы только перечисленные элементы. Для CHILDREN имя лишь
    ** запоминается — модель проверяется целиком при закрытии родителя.
    */
    void DtdValidator::checkChild(OpenState& parent, std::string_view child)
    {
        if (parent.declaration == nullptr)
            return;
        ContentKind kind = parent.declaration->kind;
        if (kind == ContentKind::EMPTY_CONTENT)
            _reporter.reportValidity("элемент <" + parent.name + "> объявлен EMPTY, но содержит <"
                + std::string(child) + ">");
        if (kind == ContentKind::CHILDREN)
            parent.children.push_back(std::string(child));
        if (kind != ContentKind::MIXED)
            return;
        for (const std::string& allowed : parent.declaration->mixedNames)
        {
            if (allowed == child)
                return;
        }
        _reporter.reportValidity("элемент <" + std::string(child) + "> недопустим внутри <"
            + parent.name + ">");
    }

    // Сопоставление последовательности детей с моделью общим алгоритмом content::matchContent
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
            _reporter.reportValidity("элемент <" + state.children[result.failIndex]
                + "> недопустим в этой позиции внутри <" + state.name + ">; модель: " + model);
        else
            _reporter.reportValidity("внутри <" + state.name
                + "> не хватает обязательных элементов; модель: " + model);
    }
}
