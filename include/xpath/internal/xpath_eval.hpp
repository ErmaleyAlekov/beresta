#ifndef XPATH_EVAL_HPP
#define XPATH_EVAL_HPP

#include "xpath/internal/xpath_ast.hpp"

#include <string>
#include <vector>

namespace beresta::xpath::detail
{
    // ---- Оси (xpath_axes*.cpp) ----

    // Узлы оси в порядке оси (для обратных осей — от ближайшего к дальнему)
    std::vector<XPathItem> axisItems(Axis axis, const XPathItem& item);

    // Подходит ли узел под проверку шага (имя, тип) с учётом главного типа оси
    bool matchesTest(const Step& step, const XPathItem& item, const XPathContext& user);

    // Корень дерева узла (документ или самый верхний элемент автономного дерева)
    XPathItem rootOf(const XPathItem& item);

    // ---- Шаги и предикаты (xpath_steps.cpp) ----

    // Применяет шаг ко всем входным узлам и возвращает результат в порядке документа
    std::vector<XPathItem> applyStep(const Step& step, const std::vector<XPathItem>& inputs,
        const XPathContext& user);

    // Фильтрует узлы предикатами (позиции считаются в порядке списка)
    std::vector<XPathItem> applyPredicates(const std::vector<ExprPtr>& predicates,
        std::vector<XPathItem> items, const XPathContext& user);

    // ---- Порядок документа (xpath_document_order.cpp) ----

    // Сортирует в порядке документа и убирает повторы
    void sortDocumentOrder(std::vector<XPathItem>& items);

    // ---- Сравнения (xpath_compare.cpp) ----

    // =, !=, <, <=, >, >= с правилами XPath для множеств узлов
    bool compareValues(BinaryOperator op, const XPathValue& left, const XPathValue& right);

    // ---- Функции (xpath_functions*.cpp) ----

    // Вызов функции базовой библиотеки; неизвестная функция — XPATH_ERROR
    XPathValue callFunction(const std::string& name, const std::vector<ExprPtr>& arguments,
        const EvalContext& context);

    // Ошибка вычисления XPath
    [[noreturn]] void failEvaluation(const std::string& message);
}

#endif
