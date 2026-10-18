#include "demo/demo_scenarios.hpp"
#include "platform/console.hpp"
#include "beresta.hpp"

#include <iostream>

/*
** Точка входа демонстрации «Берёсты»: последовательно запускает сценарии.
** Любое исключение библиотеки перехватывается здесь и выводится
** в читаемом виде вместо аварийного завершения программы.
**
** The Beresta demo entry point: runs the scenarios one after another.
** Any library exception is caught here and printed in a readable form
** instead of crashing the program.
*/
int main()
{
    platform::enableUtf8Console();
    try
    {
        demo::showFormatIndependence();
        demo::showSafeCopying();
        demo::showTreeBuilding();
        demo::showErrorReporting();
        demo::showXPath();
        demo::showSchemaValidation();
        demo::showSaxStreaming();
        demo::showEncodings();
    }
    catch (const beresta::DomException& error)
    {
        std::cerr << "Непредвиденная ошибка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
