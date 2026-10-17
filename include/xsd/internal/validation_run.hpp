#ifndef XSD_VALIDATION_RUN_HPP
#define XSD_VALIDATION_RUN_HPP

#include "parser/error_handler.hpp"
#include "xsd/schema.hpp"

#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** ValidationRun — один проход проверки документа по схеме.
    ** Обходит дерево сверху вниз: для каждого элемента определяется тип
    ** (из объявления или xsi:type), затем проверяются атрибуты, модель
    ** содержимого (общим алгоритмом content::matchContent) и текст.
    ** Ошибки не прерывают обход — собирается полный список.
    */
    class ValidationRun
    {
    public:
        explicit ValidationRun(const Schema& schema);

        std::vector<Diagnostic> run(const Document& document);

    private:
        // ---- Элементы (xsd_validation_run.cpp) ----
        void validateElement(const Element& element, TypeRef type, const ElementDecl* declaration,
            const std::string& path);
        bool applyXsiType(const Element& element, TypeRef& type, const std::string& path);
        bool isNil(const Element& element, const ElementDecl* declaration, const std::string& path);
        void report(const Element& element, const std::string& path, const std::string& message);

        // ---- Простые значения (xsd_validation_values.cpp) ----
        void validateSimpleElement(const Element& element, const SimpleType& type,
            const ElementDecl* declaration, const std::string& path);
        void checkValue(const Element& element, const SimpleType& type, std::string value,
            const ElementDecl* declaration, const std::string& path);
        void recordIdentity(const SimpleType& type, std::string_view value, const Element& element,
            const std::string& path);
        void finish(const Element& root);

        // ---- Атрибуты (xsd_validation_attributes.cpp) ----
        void validateAttributes(const Element& element, const ComplexType& type, const std::string& path);
        void checkAttribute(const Element& element, const AttributeUse& use, const std::string& value,
            const std::string& path);
        void checkRequiredAttributes(const Element& element, const ComplexType& type,
            const std::string& path);

        // ---- Содержимое (xsd_validation_content.cpp) ----
        void validateContent(const Element& element, const ComplexType& type,
            const ElementDecl* declaration, const std::string& path);
        void checkModel(const Element& element, const ComplexType& type,
            const std::vector<const Element*>& children, const std::string& path);
        void validateChildren(const ComplexType& type, const std::vector<const Element*>& children,
            const std::string& path);
        const ElementDecl* findDeclaration(const ComplexType& type, const Element& child) const;

        const Schema& _schema;
        std::vector<Diagnostic> _diagnostics;
        std::set<std::string> _ids;
        std::vector<std::pair<std::string, std::string>> _references;
    };

    // Путь к ребёнку: /родитель/имя или /родитель/имя[k], если одноимённых соседей несколько
    std::string childPath(const std::string& parentPath, const Element& child);

    // Пространство имён и локальное имя атрибута с учётом префикса
    std::pair<std::string, std::string> attributeKey(const Element& element, const Attribute& attribute);
}

#endif
