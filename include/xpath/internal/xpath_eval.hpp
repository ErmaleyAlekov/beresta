#ifndef XPATH_EVAL_HPP
#define XPATH_EVAL_HPP

#include "xpath/internal/xpath_ast.hpp"

#include <string>
#include <vector>

namespace beresta::xpath::detail
{
    // ---- Оси (xpath_axes*.cpp) ----
    // ---- Axes (xpath_axes*.cpp) ----

    // Дописывает узлы оси в output в порядке оси (для обратных осей — от ближайшего к дальнему)
    // Appends the axis nodes to output in axis order (for reverse axes — from the nearest to the farthest)
    void appendAxisItems(Axis axis, const XPathItem& item, std::vector<XPathItem>& output);

    // Подходит ли узел под проверку шага (имя, тип) с учётом главного типа оси
    // Whether a node passes the step test (name, type) taking the principal node type of the axis into account
    bool matchesTest(const Step& step, const XPathItem& item, const XPathContext& user);

    // Корень дерева узла (документ или самый верхний элемент автономного дерева)
    // The root of the node's tree (the document or the topmost element of a standalone tree)
    XPathItem rootOf(const XPathItem& item);

    // ---- Шаги и предикаты (xpath_steps.cpp) ----
    // ---- Steps and predicates (xpath_steps.cpp) ----

    // Применяет шаг ко всем входным узлам и возвращает результат в порядке документа
    // Applies a step to all input nodes and returns the result in document order
    std::vector<XPathItem> applyStep(const Step& step, const std::vector<XPathItem>& inputs,
        const XPathContext& user);

    // Фильтрует узлы предикатами (позиции считаются в порядке списка)
    // Filters nodes with predicates (positions are counted in list order)
    std::vector<XPathItem> applyPredicates(const std::vector<ExprPtr>& predicates,
        std::vector<XPathItem> items, const XPathContext& user);

    // ---- Порядок документа (xpath_document_order.cpp) ----
    // ---- Document order (xpath_document_order.cpp) ----

    // Сортирует в порядке документа и убирает повторы
    // Sorts in document order and removes duplicates
    void sortDocumentOrder(std::vector<XPathItem>& items);

    // Ключ порядка документа: сравнение ключей = сравнение позиций в документе
    // The document order key: comparing keys = comparing positions in the document
    std::vector<std::size_t> documentOrderKey(const XPathItem& item);

    // Уже ли элементы строго по порядку документа и без повторов (xpath_document_order_check.cpp)
    // Whether the items are already strictly in document order and without duplicates
    bool isDocumentOrdered(const std::vector<XPathItem>& items);

    // ---- Сравнения (xpath_compare.cpp) ----
    // ---- Comparisons (xpath_compare.cpp) ----

    // =, !=, <, <=, >, >= с правилами XPath для множеств узлов
    // =, !=, <, <=, >, >= with the XPath rules for node sets
    bool compareValues(BinaryOperator op, const XPathValue& left, const XPathValue& right);

    // ---- Функции (xpath_functions*.cpp) ----
    // ---- Functions (xpath_functions*.cpp) ----

    // Вызов функции базовой библиотеки; неизвестная функция — XPATH_ERROR
    // Calls a core library function; an unknown function is XPATH_ERROR
    XPathValue callFunction(const std::string& name, const std::vector<ExprPtr>& arguments,
        const EvalContext& context);

    // Ошибка вычисления XPath
    // An XPath evaluation error
    [[noreturn]] void failEvaluation(const std::string& message);
}

#endif
