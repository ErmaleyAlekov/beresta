#ifndef DEMO_SCENARIOS_HPP
#define DEMO_SCENARIOS_HPP

namespace demo
{
    // ---- Базовые возможности (demo_scenarios.cpp) ----

    // Отформатированный и «сплошной» XML дают одинаковый монолитный вывод
    void showFormatIndependence();

    // Глубокое копирование, clone() и автономность извлечённых узлов
    void showSafeCopying();

    // Построение дерева вручную, экранирование, CDATA и комментарии
    void showTreeBuilding();

    // Понятные исключения с позицией ошибки
    void showErrorReporting();

    // ---- Возможности уровня Xerces (demo_advanced.cpp) ----

    // Поиск по XPath 1.0 с пространствами имён
    void showXPath();

    // Проверка по XML Schema со сбором всех ошибок
    void showSchemaValidation();

    // Потоковый разбор SAX без построения дерева
    void showSaxStreaming();

    // Кодировки: чтение windows-1251 и красивый вывод в UTF-8
    void showEncodings();
}

#endif
