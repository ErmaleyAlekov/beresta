/*
** Демонстрация Си-интерфейса «Берёсты» из программы на чистом C99.
**
** Программа подключает только beresta_c.h и динамическую библиотеку —
** ровно так же к библиотеке обращаются Java (Panama), C# (P/Invoke) и
** Python (ctypes). Если этот файл компилируется компилятором C, значит
** заголовок не содержит ничего специфичного для C++.
*/
#include "capi/beresta_c.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#endif

/* Печать последней ошибки библиотеки и код возврата 1 для main */
static int report_failure(const char* step)
{
    int code = beresta_last_error_code();

    fprintf(stderr, "%s: [%s] %s\n", step, beresta_status_name(code), beresta_last_error_message());
    return 1;
}

/* Печать строки, полученной от библиотеки, с её освобождением */
static void print_owned(const char* label, char* text)
{
    if (text == NULL)
        printf("%s: (нет)\n", label);
    else
        printf("%s: %s\n", label, text);
    beresta_string_free(text);
}

/* Разбор, навигация по дереву и вычисление XPath */
static int show_parsing(void)
{
    const char* xml = "<shop><item code=\"A1\">Чай</item><item code=\"B2\">Кофе</item></shop>";
    beresta_document document = NULL;
    beresta_xpath xpath = NULL;

    if (beresta_document_parse(xml, (int64_t)strlen(xml), BERESTA_PARSE_DEFAULT, NULL, &document) != BERESTA_OK)
        return report_failure("разбор");
    print_owned("первый товар", beresta_node_text(beresta_node_first_child(beresta_document_root(document))));
    if (beresta_xpath_compile("//item[@code = $code]", &xpath) != BERESTA_OK)
    {
        beresta_document_free(document);
        return report_failure("XPath");
    }
    beresta_xpath_set_string(xpath, "code", "B2");
    print_owned("товар B2", beresta_xpath_string(xpath, document));
    beresta_xpath_free(xpath);
    beresta_document_free(document);
    return 0;
}

/* Построение документа с нуля и сериализация с отступами */
static int show_building(void)
{
    beresta_document document = NULL;
    beresta_node item;

    if (beresta_document_create("order", &document) != BERESTA_OK)
        return report_failure("создание");
    item = beresta_node_append_element(beresta_document_root(document), "item");
    beresta_element_set_attribute(item, "qty", "2");
    beresta_node_append_text(item, "Мёд & орехи");
    print_owned("документ", beresta_node_to_string(document, BERESTA_WRITE_PRETTY));
    beresta_document_free(document);
    return 0;
}

/* Точка входа: версия, два сценария и пример обработки ошибки */
int main(void)
{
    beresta_document broken = NULL;

#ifdef _WIN32
    /* Строки библиотеки — UTF-8: консоль Windows переводится в ту же кодировку */
    SetConsoleOutputCP(CP_UTF8);
#endif
    printf("Берёста %s (Си-интерфейс)\n", beresta_version());
    if (show_parsing() != 0 || show_building() != 0)
        return 1;
    if (beresta_document_parse("<a><b></a>", 10, BERESTA_PARSE_DEFAULT, NULL, &broken) != BERESTA_OK)
        printf("ожидаемая ошибка: [%s] %s\n", beresta_status_name(beresta_last_error_code()),
            beresta_last_error_message());
    return 0;
}
