/* SPDX-License-Identifier: MIT — Берёста, см. файл LICENSE в корне проекта */
#ifndef BERESTA_C_H
#define BERESTA_C_H

/*
** ============================================================================
**  Берёста — плоский Си-интерфейс (extern "C")
** ============================================================================
**
** Этот заголовок — единственная точка входа в библиотеку из других языков:
** Java (Project Panama / FFM API), C# (P/Invoke), Python (ctypes), а также
** из чистого C. Он намеренно написан на подмножестве C99, чтобы его можно
** было подключить и в .c-файл, и в .cpp-файл.
**
** Почему именно плоский C, а не C++-классы наружу:
**   - extern "C" отключает декорирование имён (name mangling): имя функции
**     в таблице экспорта совпадает с именем в исходнике, и его можно найти
**     через dlsym / GetProcAddress / SymbolLookup из любого языка;
**   - ABI языка C стабилен и одинаков для всех компиляторов платформы,
**     тогда как ABI C++ (vtable, исключения, std::string) различается даже
**     между версиями одного компилятора;
**   - исключения C++ никогда не пересекают границу библиотеки: каждая
**     функция перехватывает их и превращает в код состояния.
**
** Паттерн: Facade (GoF) — сотня классов ядра скрыта за небольшим набором
** функций над непрозрачными дескрипторами (opaque handle). Внутреннее
** устройство можно менять, не ломая ни одну из обёрток.
**
** ---------------------------------------------------------------------------
**  Соглашения (одинаковые для ВСЕХ функций — их легко повторить в обёртке)
** ---------------------------------------------------------------------------
**  1. Объекты передаются наружу как сырые указатели void* (дескрипторы).
**  2. Каждая функция в начале сбрасывает «последнюю ошибку» потока. При сбое
**     она записывает код и сообщение, которые читаются функциями
**     beresta_last_error_code() / beresta_last_error_message().
**  3. Функции, возвращающие int-статус, отдают BERESTA_OK (0) при успехе.
**  4. Функции, возвращающие дескриптор или char*, отдают NULL при сбое.
**     NULL без ошибки (код 0) означает «нет такого объекта» — например,
**     у последнего узла нет следующего соседа.
**  5. Строка char*, возвращённая библиотекой, принадлежит вызывающему:
**     её обязательно нужно освободить через beresta_string_free().
**     Строка const char* — заимствованная, освобождать её нельзя.
**  6. Все строки — UTF-8, завершённые нулём.
**  7. Размеры и индексы — int64_t (одинаковый тип во всех языках и на всех
**     64-битных платформах; -1 в качестве результата означает ошибку).
**  8. Владение:
**       - документ, схему, XPath-выражение, набор узлов и список
**         диагностик освобождает вызывающий (beresta_*_free);
**       - узел принадлежит своему документу: его дескриптор действителен,
**         пока жив документ и пока узел не удалён из дерева;
**       - набор узлов XPath ссылается на узлы документа и действителен,
**         пока документ не изменён и не освобождён.
**  9. Потоки: разные документы можно обрабатывать в разных потоках
**     одновременно. Один документ — только из одного потока за раз.
**     Схема после загрузки неизменяема и потокобезопасна.
** ============================================================================
*/

#include <stdint.h>

/* Версия библиотеки (та же, что в project() файла CMakeLists.txt) */
#define BERESTA_VERSION_STRING "2.1.0"

/*
** ---- Видимость символов ------------------------------------------------------
** При сборке динамической библиотеки (BERESTA_BUILDING_SHARED) функции
** помечаются как экспортируемые. Все остальные символы библиотеки (классы
** ядра C++) скрыты: на Windows они не помечены dllexport, на Linux/macOS
** библиотека собирается с -fvisibility=hidden. Наружу видны только эти
** функции — меньше таблица символов, быстрее загрузка, нет конфликтов имён.
*/
#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(BERESTA_BUILDING_SHARED)
        #define BERESTA_API __declspec(dllexport)
    #elif defined(BERESTA_USING_SHARED)
        #define BERESTA_API __declspec(dllimport)
    #else
        #define BERESTA_API
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define BERESTA_API __attribute__((visibility("default")))
#else
    #define BERESTA_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Непрозрачные дескрипторы (сырые указатели void*) --------------------- */
typedef void* beresta_document;    /* документ (Document), владеет деревом     */
typedef void* beresta_node;        /* любой узел дерева, в том числе документ  */
typedef void* beresta_xpath;       /* скомпилированное выражение XPath 1.0     */
typedef void* beresta_nodeset;     /* результат XPath: упорядоченные узлы      */
typedef void* beresta_schema;      /* загруженная XML Schema                   */
typedef void* beresta_diagnostics; /* список ошибок проверки по схеме          */

/*
** ---- Коды состояния --------------------------------------------------------
** Значения 1..15 совпадают с beresta::DomErrorCode + 1 — обёртки могут
** показывать одинаковые имена ошибок во всех языках.
*/
typedef enum beresta_status
{
    BERESTA_OK                  = 0,
    BERESTA_INVALID_NAME        = 1,  /* недопустимое XML-имя                  */
    BERESTA_INVALID_CONTENT     = 2,  /* недопустимое содержимое узла          */
    BERESTA_HIERARCHY_VIOLATION = 3,  /* нарушение структуры дерева            */
    BERESTA_INDEX_OUT_OF_RANGE  = 4,  /* индекс вне диапазона                  */
    BERESTA_NULL_NODE           = 5,  /* пустой узел внутри ядра               */
    BERESTA_PARSE_ERROR         = 6,  /* синтаксическая ошибка XML             */
    BERESTA_NOT_FOUND           = 7,  /* объект не найден                      */
    BERESTA_NAMESPACE_ERROR     = 8,  /* нарушение правил пространств имён     */
    BERESTA_IO_ERROR            = 9,  /* ошибка чтения или записи файла        */
    BERESTA_ENCODING_ERROR      = 10, /* неизвестная или неверная кодировка    */
    BERESTA_VALIDATION_ERROR    = 11, /* документ не соответствует DTD / XSD   */
    BERESTA_XPATH_ERROR         = 12, /* ошибка XPath                          */
    BERESTA_SCHEMA_ERROR        = 13, /* некорректная XML Schema               */
    BERESTA_RESOURCE_ERROR      = 14, /* внешний ресурс не загружен            */
    BERESTA_LIMIT_EXCEEDED      = 15, /* превышен защитный лимит               */
    BERESTA_INVALID_ARGUMENT    = 16, /* NULL или дескриптор не того типа      */
    BERESTA_OUT_OF_MEMORY       = 17, /* не хватило памяти                     */
    BERESTA_UNKNOWN_ERROR       = 18  /* непредвиденное исключение             */
} beresta_status;

/* ---- Типы узлов (порядок совпадает с beresta::NodeType) ------------------- */
typedef enum beresta_node_kind
{
    BERESTA_NODE_DOCUMENT               = 0,
    BERESTA_NODE_DOCUMENT_TYPE          = 1,
    BERESTA_NODE_ELEMENT                = 2,
    BERESTA_NODE_TEXT                   = 3,
    BERESTA_NODE_CDATA_SECTION          = 4,
    BERESTA_NODE_COMMENT                = 5,
    BERESTA_NODE_PROCESSING_INSTRUCTION = 6
} beresta_node_kind;

/*
** ---- Флаги разбора (битовая маска, объединяются через |) -------------------
** Маска вместо структуры настроек выбрана сознательно: целое число одинаково
** передаётся из любого языка, а раскладку структуры пришлось бы повторять
** в каждой обёртке вручную (и следить за выравниванием полей).
*/
#define BERESTA_PARSE_DEFAULT             0u
#define BERESTA_PARSE_PRESERVE_WHITESPACE (1u << 0) /* хранить пробельный текст между тегами */
#define BERESTA_PARSE_DISCARD_COMMENTS    (1u << 1) /* не создавать узлы комментариев       */
#define BERESTA_PARSE_COALESCE_CDATA      (1u << 2) /* CDATA превращать в обычный текст     */
#define BERESTA_PARSE_VALIDATE_DTD        (1u << 3) /* проверять документ по DTD            */
#define BERESTA_PARSE_NO_NAMESPACES       (1u << 4) /* отключить обработку xmlns            */
#define BERESTA_PARSE_XINCLUDE            (1u << 5) /* выполнять xi:include                 */
#define BERESTA_PARSE_FILE_RESOURCES      (1u << 6) /* разрешить внешние DTD/сущности/XInclude
                                                       из каталога разбираемого файла       */

/* ---- Флаги вывода ----------------------------------------------------------- */
#define BERESTA_WRITE_DEFAULT             0u        /* компактно, без лишних пробелов */
#define BERESTA_WRITE_PRETTY              (1u << 0) /* отступы и переводы строк       */
#define BERESTA_WRITE_DECLARATION         (1u << 1) /* всегда писать <?xml ...?>      */

/* =========================================================================
**  Библиотека и ошибки
** ========================================================================= */

/* Версия библиотеки, например "2.1.0" (заимствованная строка) */
BERESTA_API const char* beresta_version(void);

/* Код последней ошибки текущего потока (BERESTA_OK, если ошибки не было) */
BERESTA_API int beresta_last_error_code(void);

/* Сообщение последней ошибки текущего потока ("" — ошибки не было).
   Заимствованная строка: действительна до следующего вызова в этом потоке */
BERESTA_API const char* beresta_last_error_message(void);

/* Имя кода состояния, например "PARSE_ERROR" (заимствованная строка) */
BERESTA_API const char* beresta_status_name(int status);

/* Освобождение строки, которую вернула библиотека (NULL допустим) */
BERESTA_API void beresta_string_free(char* text);

/* =========================================================================
**  Документы
** ========================================================================= */

/* Новый документ; root_name == NULL — документ без корневого элемента */
BERESTA_API int beresta_document_create(const char* root_name, beresta_document* out);

/* Разбор байтов в любой поддерживаемой кодировке (UTF-8/16, windows-1251...).
   schema == NULL — без проверки по схеме */
BERESTA_API int beresta_document_parse(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_document* out);

/* Разбор файла (кодировка определяется автоматически) */
BERESTA_API int beresta_document_parse_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_document* out);

/* Освобождение документа вместе со всеми его узлами (NULL допустим) */
BERESTA_API void beresta_document_free(beresta_document document);

/* Глубокая копия документа — новый независимый документ */
BERESTA_API int beresta_document_clone(beresta_document document, beresta_document* out);

/* Корневой элемент или NULL, если его нет */
BERESTA_API beresta_node beresta_document_root(beresta_document document);

/* Элемент с атрибутом типа ID (по DTD) или с атрибутом xml:id */
BERESTA_API beresta_node beresta_document_element_by_id(beresta_document document, const char* id);

/* Запись документа в файл; encoding == NULL — UTF-8 */
BERESTA_API int beresta_document_save(beresta_document document, const char* path,
    uint32_t flags, const char* encoding);

/* =========================================================================
**  Узлы: свойства и навигация
** ========================================================================= */

/* Тип узла (значение beresta_node_kind) или -1 при ошибке */
BERESTA_API int beresta_node_type(beresta_node node);

/* Имя: тег элемента, цель PI, имя DOCTYPE; "#text", "#comment" и т.п. */
BERESTA_API char* beresta_node_name(beresta_node node);

/* Текстовое содержимое узла (у элемента — весь текст потомков) */
BERESTA_API char* beresta_node_text(beresta_node node);

/* Сериализация узла и его потомков в строку UTF-8 */
BERESTA_API char* beresta_node_to_string(beresta_node node, uint32_t flags);

BERESTA_API beresta_node beresta_node_parent(beresta_node node);
BERESTA_API beresta_node beresta_node_first_child(beresta_node node);
BERESTA_API beresta_node beresta_node_last_child(beresta_node node);
BERESTA_API beresta_node beresta_node_next_sibling(beresta_node node);
BERESTA_API beresta_node beresta_node_previous_sibling(beresta_node node);

/* Документ, которому принадлежит узел (у документа — он сам) */
BERESTA_API beresta_document beresta_node_owner_document(beresta_node node);

/* Число дочерних узлов (0 у листовых узлов) или -1 при ошибке */
BERESTA_API int64_t beresta_node_child_count(beresta_node node);

/* Дочерний узел по индексу */
BERESTA_API beresta_node beresta_node_child(beresta_node node, int64_t index);

/* =========================================================================
**  Узлы: изменение дерева
** ========================================================================= */

/* Замена текста: у элемента — все дети заменяются одним текстовым узлом,
   у текста / комментария / CDATA / PI — новое значение */
BERESTA_API int beresta_node_set_text(beresta_node node, const char* text);

/* Удаление узла из дерева вместе с потомками; дескриптор становится недействительным */
BERESTA_API int beresta_node_remove(beresta_node node);

/* Глубокая копия source в конец детей parent (можно между документами) */
BERESTA_API int beresta_node_copy_to(beresta_node source, beresta_node parent, beresta_node* out);

/* Добавление нового узла в конец детей parent (элемента или документа).
   У документа append_element задаёт корневой элемент */
BERESTA_API beresta_node beresta_node_append_element(beresta_node parent, const char* name);
BERESTA_API beresta_node beresta_node_append_text(beresta_node parent, const char* text);
BERESTA_API beresta_node beresta_node_append_comment(beresta_node parent, const char* text);
BERESTA_API beresta_node beresta_node_append_cdata(beresta_node parent, const char* text);
BERESTA_API beresta_node beresta_node_append_pi(beresta_node parent, const char* target, const char* data);

/* =========================================================================
**  Элементы и атрибуты
** ========================================================================= */

/* Значение атрибута или NULL (код 0), если атрибута нет */
BERESTA_API char* beresta_element_get_attribute(beresta_node element, const char* name);

BERESTA_API int beresta_element_set_attribute(beresta_node element, const char* name, const char* value);

/* 1 — атрибут есть, 0 — нет, -1 — ошибка */
BERESTA_API int beresta_element_has_attribute(beresta_node element, const char* name);

/* 1 — атрибут удалён, 0 — его не было, -1 — ошибка */
BERESTA_API int beresta_element_remove_attribute(beresta_node element, const char* name);

/* Перебор атрибутов в порядке документа */
BERESTA_API int64_t beresta_element_attribute_count(beresta_node element);
BERESTA_API char* beresta_element_attribute_name(beresta_node element, int64_t index);
BERESTA_API char* beresta_element_attribute_value(beresta_node element, int64_t index);

/* Локальное имя (без префикса) */
BERESTA_API char* beresta_element_local_name(beresta_node element);

/* URI пространства имён или NULL (код 0), если элемент вне пространств имён */
BERESTA_API char* beresta_element_namespace_uri(beresta_node element);

/* =========================================================================
**  XPath 1.0
** ========================================================================= */

/* Компиляция выражения (один раз — вычислять можно многократно) */
BERESTA_API int beresta_xpath_compile(const char* expression, beresta_xpath* out);
BERESTA_API void beresta_xpath_free(beresta_xpath xpath);

/* Привязка префикса к URI для имён вида p:name в выражении */
BERESTA_API int beresta_xpath_bind_namespace(beresta_xpath xpath, const char* prefix, const char* uri);

/* Значения переменных $name */
BERESTA_API int beresta_xpath_set_string(beresta_xpath xpath, const char* name, const char* value);
BERESTA_API int beresta_xpath_set_number(beresta_xpath xpath, const char* name, double value);

/* Вычисление: множество узлов / строка / число / логическое (1, 0, -1 — ошибка) */
BERESTA_API int beresta_xpath_select(beresta_xpath xpath, beresta_node context, beresta_nodeset* out);
BERESTA_API char* beresta_xpath_string(beresta_xpath xpath, beresta_node context);
BERESTA_API int beresta_xpath_number(beresta_xpath xpath, beresta_node context, double* out);
BERESTA_API int beresta_xpath_boolean(beresta_xpath xpath, beresta_node context);

/* Результат select: размер, узел (NULL для атрибута), строковое значение */
BERESTA_API int64_t beresta_nodeset_size(beresta_nodeset nodeset);
BERESTA_API beresta_node beresta_nodeset_node(beresta_nodeset nodeset, int64_t index);
BERESTA_API char* beresta_nodeset_value(beresta_nodeset nodeset, int64_t index);
BERESTA_API void beresta_nodeset_free(beresta_nodeset nodeset);

/* =========================================================================
**  XML Schema
** ========================================================================= */

/* Загрузка схемы из байтов XSD (xs:include / xs:import недоступны) */
BERESTA_API int beresta_schema_load(const void* data, int64_t size, beresta_schema* out);

/* Загрузка из файла: xs:include / xs:import ищутся в каталоге этого файла */
BERESTA_API int beresta_schema_load_file(const char* path, beresta_schema* out);
BERESTA_API void beresta_schema_free(beresta_schema schema);

/* Проверка документа: все нарушения собираются за один проход в out */
BERESTA_API int beresta_schema_validate(beresta_schema schema, beresta_document document,
    beresta_diagnostics* out);

/* Список нарушений: число, текст, строка и столбец в исходнике (0 — неизвестно) */
BERESTA_API int64_t beresta_diagnostics_count(beresta_diagnostics diagnostics);
BERESTA_API char* beresta_diagnostics_message(beresta_diagnostics diagnostics, int64_t index);
BERESTA_API int64_t beresta_diagnostics_line(beresta_diagnostics diagnostics, int64_t index);
BERESTA_API int64_t beresta_diagnostics_column(beresta_diagnostics diagnostics, int64_t index);
BERESTA_API void beresta_diagnostics_free(beresta_diagnostics diagnostics);

#ifdef __cplusplus
}
#endif

#endif
