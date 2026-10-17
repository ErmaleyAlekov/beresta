#ifndef DTD_VALIDATOR_HPP
#define DTD_VALIDATOR_HPP

#include "core/attribute.hpp"
#include "dtd/dtd_model.hpp"

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::dtd
{
    /*
    ** Получатель сообщений о нарушениях валидности. Реализуется сессией
    ** разбора: она знает текущую позицию и решает, бросать ли исключение
    ** или передать ошибку в ErrorHandler и продолжить (паттерн Observer).
    */
    class ValidityReporter
    {
    public:
        virtual ~ValidityReporter();

        virtual void reportValidity(const std::string& message) = 0;
    };

    /*
    ** DtdValidator — проверка документа по DTD «на лету», по событиям разбора.
    ** Проверяет: соответствие корня DOCTYPE, объявленность элементов и
    ** атрибутов, обязательные и фиксированные атрибуты, типы значений
    ** (ID, IDREF(S), NMTOKEN(S), ENTITY(IES), перечисления), уникальность ID,
    ** существование целей IDREF и модели содержимого (EMPTY, MIXED, CHILDREN).
    */
    class DtdValidator
    {
    public:
        DtdValidator(const DtdModel& model, std::string rootName, ValidityReporter& reporter);

        void startElement(std::string_view name, const std::vector<Attribute>& attributes);
        void text(bool significant);
        void endElement();

        // Проверки, возможные только в конце документа (цели IDREF)
        void finish();

    private:
        // Открытый элемент: объявление и имена детей для проверки модели
        struct OpenState
        {
            std::string name;
            const ElementDeclaration* declaration;
            std::vector<std::string> children;
        };

        // ---- Содержимое (dtd_validator_content.cpp) ----
        void checkChild(OpenState& parent, std::string_view child);
        void checkContent(const OpenState& state);

        // ---- Атрибуты (dtd_validator_attributes.cpp) ----
        void checkAttributes(std::string_view element, const std::vector<Attribute>& attributes);
        void checkDeclaredAttribute(const AttributeDeclaration& declaration,
            const Attribute* present);
        void checkValueByType(const AttributeDeclaration& declaration, std::string_view value);
        void checkToken(const AttributeDeclaration& declaration, std::string_view token);

        const DtdModel& _model;
        std::string _rootName;
        ValidityReporter& _reporter;
        std::vector<OpenState> _stack;
        std::set<std::string, std::less<>> _ids;
        std::vector<std::string> _references;
        bool _rootChecked;
    };
}

#endif
