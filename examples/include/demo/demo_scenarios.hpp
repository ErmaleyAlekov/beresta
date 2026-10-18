#ifndef DEMO_SCENARIOS_HPP
#define DEMO_SCENARIOS_HPP

namespace demo
{
    // ---- Базовые возможности (demo_scenarios.cpp) ----
    // ---- Basic features (demo_scenarios.cpp) ----

    // Отформатированный и «сплошной» XML дают одинаковый монолитный вывод
    // Formatted and "solid" XML give the same monolithic output
    void showFormatIndependence();

    // Глубокое копирование, clone() и автономность извлечённых узлов
    // Deep copying, clone() and the independence of extracted nodes
    void showSafeCopying();

    // Построение дерева вручную, экранирование, CDATA и комментарии
    // Building a tree by hand, escaping, CDATA and comments
    void showTreeBuilding();

    // Понятные исключения с позицией ошибки
    // Clear exceptions with the error position
    void showErrorReporting();

    // ---- Возможности уровня Xerces (demo_advanced.cpp) ----
    // ---- Xerces-level features (demo_advanced.cpp) ----

    // Поиск по XPath 1.0 с пространствами имён
    // XPath 1.0 search with namespaces
    void showXPath();

    // Проверка по XML Schema со сбором всех ошибок
    // Validation against an XML Schema collecting all errors
    void showSchemaValidation();

    // Потоковый разбор SAX без построения дерева
    // Streaming SAX parsing without building a tree
    void showSaxStreaming();

    // Кодировки: чтение windows-1251 и красивый вывод в UTF-8
    // Encodings: reading windows-1251 and pretty output in UTF-8
    void showEncodings();
}

#endif
