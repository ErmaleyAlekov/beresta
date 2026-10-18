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
    **
    ** The receiver of validity violation messages. Implemented by the parse
    ** session: it knows the current position and decides whether to throw an exception
    ** or pass the error to the ErrorHandler and continue (the Observer pattern).
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
    **
    ** DtdValidator — validates a document against a DTD "on the fly", by parse events.
    ** Checks: the root matches the DOCTYPE, elements and attributes are declared,
    ** required and fixed attributes, value types (ID, IDREF(S), NMTOKEN(S),
    ** ENTITY(IES), enumerations), ID uniqueness, the existence of IDREF targets
    ** and content models (EMPTY, MIXED, CHILDREN).
    */
    class DtdValidator
    {
    public:
        /*
        ** namespaceAware — значения ID/IDREF/ENTITY обязаны быть NCName (Namespaces 1.0, разд. 7);
        ** standalone — документ объявил standalone="yes" (VC: Standalone Document Declaration).
        **
        ** namespaceAware — ID/IDREF/ENTITY values must be NCNames (Namespaces 1.0, section 7);
        ** standalone — the document declared standalone="yes" (VC: Standalone Document Declaration).
        */
        DtdValidator(const DtdModel& model, std::string rootName, ValidityReporter& reporter,
            bool namespaceAware, bool standalone);

        /*
        ** Для standalone="yes": атрибуты ДО применения <!ATTLIST>. Внешне объявленные
        ** умолчания не должны подставляться, а нормализация внешне объявленных
        ** не-CDATA атрибутов не должна менять значение (раздел 2.9).
        **
        ** For standalone="yes": the attributes BEFORE <!ATTLIST> is applied. Externally declared
        ** defaults must not be substituted, and normalization of externally declared
        ** non-CDATA attributes must not change the value (section 2.9).
        */
        void checkStandalone(std::string_view element, const std::vector<Attribute>& attributes);

        void startElement(std::string_view name, const std::vector<Attribute>& attributes);
        void text(bool significant);

        // Комментарий, PI или ссылка на сущность в содержимом: в EMPTY-элементе запрещены (errata E15)
        // A comment, a PI or an entity reference in content: forbidden in an EMPTY element (errata E15)
        void markup();
        void endElement();

        // Проверки, возможные только в конце документа (цели IDREF)
        // Checks possible only at the end of the document (IDREF targets)
        void finish();

    private:
        // Открытый элемент: объявление и имена детей для проверки модели
        // An open element: the declaration and the child names for the model check
        struct OpenState
        {
            std::string name;
            const ElementDeclaration* declaration;
            std::vector<std::string> children;
        };

        // ---- Содержимое (dtd_validator_content.cpp) ----
        // ---- Content (dtd_validator_content.cpp) ----
        void checkChild(OpenState& parent, std::string_view child);
        void checkContent(const OpenState& state);

        // ---- Атрибуты (dtd_validator_attributes.cpp) ----
        // ---- Attributes (dtd_validator_attributes.cpp) ----
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
        bool _namespaceAware;
        bool _standalone;
    };
}

#endif
