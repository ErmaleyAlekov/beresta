#include "demo/demo_scenarios.hpp"
#include "beresta.hpp"

#include <iostream>
#include <memory>

namespace demo
{
    /*
    ** Один и тот же документ в двух видах: с отступами и одной строкой.
    ** После разбора пробельные отступы отброшены, поэтому вывод совпадает.
    */
    void showFormatIndependence()
    {
        const char* pretty =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<library>\n"
            "\t<book id=\"1\"   lang = 'ru'>\n"
            "\t\t<title>Война и мир</title>\n"
            "\t\t<author>Л. Н. Толстой</author>\n"
            "\t</book>\n"
            "</library>\n";
        const char* compact = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><library>"
            "<book id=\"1\" lang=\"ru\"><title>Война и мир</title>"
            "<author>Л. Н. Толстой</author></book></library>";
        beresta::XmlParser parser;
        std::string first = parser.parse(pretty).toString();
        std::string second = parser.parse(compact).toString();

        std::cout << "== 1. Независимость от форматирования ==\n" << first << '\n';
        if (first == second)
            std::cout << "Результаты идентичны: да\n\n";
        else
            std::cout << "Результаты идентичны: НЕТ\n\n";
    }

    /*
    ** Копия независима от оригинала, а узел, извлечённый из документа,
    ** продолжает жить после уничтожения самого документа.
    */
    void showSafeCopying()
    {
        beresta::XmlParser parser;
        std::unique_ptr<beresta::Element> detached;
        {
            beresta::Document document = parser.parse("<a><b x=\"1\"><c/></b></a>");
            beresta::Element copy = *document.getRoot();
            copy.setAttribute("copied", "yes");
            std::cout << "== 2. Безопасное копирование ==\n"
                      << "оригинал: " << document.toString() << '\n'
                      << "копия:    " << copy.toString() << '\n';
            detached = document.releaseRoot();
        }
        std::unique_ptr<beresta::Node> clone = detached->clone();
        std::cout << "извлечённый узел пережил документ: " << detached->toString() << '\n'
                  << "его клон: " << clone->toString() << "\n\n";
    }

    // Ручное построение дерева: спецсимволы экранируются автоматически
    void showTreeBuilding()
    {
        beresta::Document document;
        document.setDeclaration(beresta::XmlDeclaration{"1.0", "UTF-8", std::nullopt});
        beresta::Element* root = document.setRoot(std::make_unique<beresta::Element>("note"));
        root->setAttribute("title", "Q&A \"FAQ\"");
        root->appendChild(std::make_unique<beresta::Comment>(" пример "));
        root->appendChild(std::make_unique<beresta::Text>("1 < 2 && 3 > 2"));
        root->appendChild(std::make_unique<beresta::CDataSection>("<raw> & </raw>"));
        root->appendChild(std::make_unique<beresta::Element>("empty"));
        std::cout << "== 3. Построение дерева ==\n" << document.toString() << '\n'
                  << "текст элемента: " << root->getTextContent() << "\n\n";
    }

    // Ошибки разбора и нарушения структуры дерева
    void showErrorReporting()
    {
        std::cout << "== 4. Сообщения об ошибках ==\n";
        try
        {
            beresta::XmlParser().parse("<root>\n  <item>\n  </itme>\n</root>");
        }
        catch (const beresta::ParseException& error)
        {
            std::cout << error.what() << '\n';
        }
        try
        {
            beresta::Document document;
            document.appendChild(std::make_unique<beresta::Element>("first"));
            document.appendChild(std::make_unique<beresta::Element>("second"));
        }
        catch (const beresta::DomException& error)
        {
            std::cout << error.what() << '\n';
        }
    }
}
