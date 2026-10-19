/* SPDX-License-Identifier: MIT — Берёста, см. файл LICENSE в корне проекта */
/* SPDX-License-Identifier: MIT — Beresta, see the LICENSE file in the project root */
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
**         beresta_node_move_to передаёт узел (с потомками) другому
**         документу — дескрипторы не меняются, но теперь их срок жизни
**         определяет новый документ;
**       - набор узлов XPath ссылается на узлы документа и действителен,
**         пока документ не изменён и не освобождён.
**  9. Потоки: разные документы можно обрабатывать в разных потоках
**     одновременно. Один документ — только из одного потока за раз.
**     Схема после загрузки неизменяема и потокобезопасна.
** ============================================================================
**
** ============================================================================
** Beresta — the flat C interface (extern "C")
** ============================================================================
** This header is the only entry point into the library from other languages:
** Java (Project Panama / FFM API), C# (P/Invoke), Python (ctypes), and also
** from pure C. It is deliberately written in a subset of C99 so that it can be
** included both in a .c file and in a .cpp file.
** Why flat C and not C++ classes exposed:
**  - extern "C" disables name mangling: the function name in the export
**    table matches the name in the source, and it can be found through
**    dlsym / GetProcAddress / SymbolLookup from any language;
**  - the C ABI is stable and the same for all compilers of a platform,
**    while the C++ ABI (vtable, exceptions, std::string) differs even
**    between versions of one compiler;
**  - C++ exceptions never cross the library boundary: every function
**    catches them and turns them into a status code.
** Pattern: Facade (GoF) — a hundred core classes are hidden behind a small set
** of functions over opaque handles. The internals can be changed without
** breaking any of the wrappers.
** ---------------------------------------------------------------------------
** Conventions (the same for ALL functions — easy to repeat in a wrapper)
** ---------------------------------------------------------------------------
** 1. Objects are passed out as raw void* pointers (handles).
** 2. Every function first resets the "last error" of the thread. On failure
**    it records a code and a message, which are read by the functions
**    beresta_last_error_code() / beresta_last_error_message().
** 3. Functions returning an int status return BERESTA_OK (0) on success.
** 4. Functions returning a handle or char* return NULL on failure.
**    NULL without an error (code 0) means "no such object" — for example,
**    the last node has no next sibling.
** 5. A char* string returned by the library belongs to the caller:
**    it must be freed through beresta_string_free().
**    A const char* string is borrowed and must not be freed.
** 6. All strings are UTF-8, null-terminated.
** 7. Sizes and indexes are int64_t (the same type in all languages and on all
**    64-bit platforms; -1 as a result means an error).
** 8. Ownership:
**    - the caller frees a document, a schema, an XPath expression, a node set
**      and a diagnostics list (beresta_*_free);
**    - a node belongs to its document: its handle is valid
**      while the document is alive and the node is not removed from the tree;
**      beresta_node_move_to hands the node (with its descendants) over to another
**      document — the handles do not change, but their lifetime is now
**      determined by the new document;
**    - an XPath node set refers to the document nodes and is valid
**      while the document is not changed or freed.
** 9. Threads: different documents can be processed in different threads
**    at the same time. One document — only from one thread at a time.
**    A schema is immutable and thread-safe after loading.
** ============================================================================
*/

#include <stdint.h>

/* Версия библиотеки (та же, что в project() файла CMakeLists.txt) */
/* The library version (the same as in project() of CMakeLists.txt) */
#define BERESTA_VERSION_STRING "2.1.0"

/*
** ---- Видимость символов ------------------------------------------------------
** При сборке динамической библиотеки (BERESTA_BUILDING_SHARED) функции
** помечаются как экспортируемые. Все остальные символы библиотеки (классы
** ядра C++) скрыты: на Windows они не помечены dllexport, на Linux/macOS
** библиотека собирается с -fvisibility=hidden. Наружу видны только эти
** функции — меньше таблица символов, быстрее загрузка, нет конфликтов имён.
**
** ---- Symbol visibility ------------------------------------------------------
** When the shared library is built (BERESTA_BUILDING_SHARED) the functions
** are marked as exported. All other library symbols (the C++ core classes)
** are hidden: on Windows they are not marked dllexport, on Linux/macOS the
** library is built with -fvisibility=hidden. Only these functions are visible
** outside — a smaller symbol table, faster loading, no name conflicts.
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
/* ---- Opaque handles (raw void* pointers) --------------------- */
typedef void* beresta_document;    /* документ (Document), владеет деревом / a document (Document), owns the tree */
typedef void* beresta_node;        /* любой узел дерева, в том числе документ  */
                                   /* any tree node, including the document */
typedef void* beresta_xpath;       /* скомпилированное выражение XPath 1.0 / a compiled XPath 1.0 expression */
typedef void* beresta_nodeset;     /* результат XPath: упорядоченные узлы / an XPath result: ordered nodes */
typedef void* beresta_schema;      /* загруженная XML Schema / a loaded XML Schema */
typedef void* beresta_diagnostics; /* список ошибок проверки по схеме / a list of schema validation errors */

/*
** ---- Коды состояния --------------------------------------------------------
** Значения 1..15 совпадают с beresta::DomErrorCode + 1 — обёртки могут
** показывать одинаковые имена ошибок во всех языках.
**
** ---- Status codes --------------------------------------------------------
** Values 1..15 match beresta::DomErrorCode + 1 — the wrappers can
** show the same error names in all languages.
*/
typedef enum beresta_status
{
    BERESTA_OK                  = 0,
    BERESTA_INVALID_NAME        = 1,  /* недопустимое XML-имя / an invalid XML name */
    BERESTA_INVALID_CONTENT     = 2,  /* недопустимое содержимое узла / invalid node content */
    BERESTA_HIERARCHY_VIOLATION = 3,  /* нарушение структуры дерева / a tree structure violation */
    BERESTA_INDEX_OUT_OF_RANGE  = 4,  /* индекс вне диапазона / an index out of range */
    BERESTA_NULL_NODE           = 5,  /* пустой узел внутри ядра / an empty node inside the core */
    BERESTA_PARSE_ERROR         = 6,  /* синтаксическая ошибка XML / an XML syntax error */
    BERESTA_NOT_FOUND           = 7,  /* объект не найден / the object is not found */
    BERESTA_NAMESPACE_ERROR     = 8,  /* нарушение правил пространств имён / a namespace rules violation */
    BERESTA_IO_ERROR            = 9,  /* ошибка чтения или записи файла / a file read or write error */
    BERESTA_ENCODING_ERROR      = 10, /* неизвестная или неверная кодировка / an unknown or invalid encoding */
    BERESTA_VALIDATION_ERROR    = 11, /* документ не соответствует DTD / XSD   */
                                      /* the document does not conform to the DTD / XSD */
    BERESTA_XPATH_ERROR         = 12, /* ошибка XPath / an XPath error */
    BERESTA_SCHEMA_ERROR        = 13, /* некорректная XML Schema / an invalid XML Schema */
    BERESTA_RESOURCE_ERROR      = 14, /* внешний ресурс не загружен / an external resource is not loaded */
    BERESTA_LIMIT_EXCEEDED      = 15, /* превышен защитный лимит / a protective limit is exceeded */
    BERESTA_INVALID_ARGUMENT    = 16, /* NULL или дескриптор не того типа / NULL or a handle of the wrong type */
    BERESTA_OUT_OF_MEMORY       = 17, /* не хватило памяти / out of memory */
    BERESTA_UNKNOWN_ERROR       = 18  /* непредвиденное исключение / an unexpected exception */
} beresta_status;

/* ---- Типы узлов (порядок совпадает с beresta::NodeType) ------------------- */
/* ---- Node types (the order matches beresta::NodeType) ------------------- */
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
**
** ---- Parsing flags (a bit mask, combined with |) -------------------
** A mask instead of a settings structure was chosen deliberately: an integer is
** passed the same way from any language, while a structure layout would have to
** be repeated by hand in every wrapper (watching the field alignment).
*/
#define BERESTA_PARSE_DEFAULT             0u
#define BERESTA_PARSE_PRESERVE_WHITESPACE (1u << 0) /* хранить пробельный текст между тегами */
                                                    /* keep whitespace text between tags */
#define BERESTA_PARSE_DISCARD_COMMENTS    (1u << 1) /* не создавать узлы комментариев / do not create comment nodes */
#define BERESTA_PARSE_COALESCE_CDATA      (1u << 2) /* CDATA превращать в обычный текст     */
                                                    /* turn CDATA into ordinary text */
#define BERESTA_PARSE_VALIDATE_DTD        (1u << 3) /* проверять документ по DTD            */
                                                    /* validate the document against the DTD */
#define BERESTA_PARSE_NO_NAMESPACES       (1u << 4) /* отключить обработку xmlns / disable xmlns processing */
#define BERESTA_PARSE_XINCLUDE            (1u << 5) /* выполнять xi:include / perform xi:include */
#define BERESTA_PARSE_FILE_RESOURCES      (1u << 6) /* разрешить внешние DTD/сущности/XInclude
                                                       из каталога разбираемого файла       */
                                                    /* allow external DTDs/entities/XInclude
                                                       from the directory of the parsed file */
#define BERESTA_PARSE_TRACK_LOCATIONS     (1u << 7) /* запоминать строку и столбец узлов: тогда
                                                       beresta_schema_validate даёт места нарушений
                                                       (разбор ~8 % медленнее; потоковой проверке
                                                       флаг не нужен — места известны и так) */
                                                    /* remember the line and column of nodes: then
                                                       beresta_schema_validate gives the places of
                                                       violations (parsing is ~8 % slower; streaming
                                                       validation does not need the flag — the
                                                       places are known anyway) */

/* ---- Флаги вывода ----------------------------------------------------------- */
/* ---- Output flags ----------------------------------------------------------- */
#define BERESTA_WRITE_DEFAULT             0u        /* компактно, без лишних пробелов */
                                                    /* compact, without extra spaces */
#define BERESTA_WRITE_PRETTY              (1u << 0) /* отступы и переводы строк / indentation and line breaks */
#define BERESTA_WRITE_DECLARATION         (1u << 1) /* всегда писать <?xml ...?> / always write <?xml ...?> */

/* =========================================================================
**  Библиотека и ошибки / Library and errors
** ========================================================================= */

/* Версия библиотеки, например "2.1.0" (заимствованная строка) */
/* The library version, for example "2.1.0" (a borrowed string) */
BERESTA_API const char* beresta_version(void);

/* Код последней ошибки текущего потока (BERESTA_OK, если ошибки не было) */
/* The last error code of the current thread (BERESTA_OK if there was no error) */
BERESTA_API int beresta_last_error_code(void);

/* Сообщение последней ошибки текущего потока ("" — ошибки не было).
   Заимствованная строка: действительна до следующего вызова в этом потоке
   The last error message of the current thread ("" — there was no error).
   A borrowed string: valid until the next call in this thread */
BERESTA_API const char* beresta_last_error_message(void);

/* Имя кода состояния, например "PARSE_ERROR" (заимствованная строка) */
/* The status code name, for example "PARSE_ERROR" (a borrowed string) */
BERESTA_API const char* beresta_status_name(int status);

/* Освобождение строки, которую вернула библиотека (NULL допустим) */
/* Frees a string returned by the library (NULL is allowed) */
BERESTA_API void beresta_string_free(char* text);

/* =========================================================================
**  Языки сообщений об ошибках / Error message languages
** ========================================================================= */

/* Смена языка сообщений для всех потоков ("ru", "en", ...).
   BERESTA_INVALID_ARGUMENT — такого языка нет, язык не меняется.
   Changes the message language for all threads ("ru", "en", ...).
   BERESTA_INVALID_ARGUMENT means there is no such language; it stays unchanged. */
BERESTA_API int beresta_set_language(const char* code);

/* Код текущего языка (заимствованная строка) / The current language code (borrowed) */
BERESTA_API const char* beresta_language(void);

/* Все языки через запятую, например "ru,en" (заимствованная строка)
   All languages separated by commas, for example "ru,en" (borrowed) */
BERESTA_API const char* beresta_languages(void);

/* Сообщения, которые показывают сами обёртки, — тоже из общего каталога:
   новый язык достаточно добавить в библиотеку, и обёртки заговорят на нём.
   Messages shown by the bindings themselves also come from the common catalog:
   a new language added to the library is picked up by the bindings at once. */
typedef enum beresta_message
{
    BERESTA_MESSAGE_DOCUMENT_CLOSED   = 1, /* документ закрыт / the document is closed        */
    BERESTA_MESSAGE_XPATH_CLOSED      = 2, /* выражение закрыто / the expression is closed    */
    BERESTA_MESSAGE_SCHEMA_CLOSED     = 3, /* схема закрыта / the schema is closed            */
    BERESTA_MESSAGE_NATIVE_CALL_FAILED = 4, /* сбой вызова / a native call failed             */
    BERESTA_MESSAGE_UNKNOWN_NODE_TYPE = 5, /* неизвестный тип узла {0} / unknown node type {0} */
    BERESTA_MESSAGE_INVALID_ARGUMENT  = 6, /* неверный аргумент №{0} функции {1} / invalid argument #{0} of {1} */
    BERESTA_MESSAGE_NO_FILE_SYSTEM    = 7  /* нет файловой системы для {0} / no file system for {0} */
} beresta_message;

/* Текст сообщения на текущем языке (заимствованная строка; {0} — место аргумента)
   The message text in the current language (borrowed; {0} marks an argument) */
BERESTA_API const char* beresta_message_text(int message);

/* =========================================================================
**  Документы / Documents
** ========================================================================= */

/* Новый документ; root_name == NULL — документ без корневого элемента */
/* A new document; root_name == NULL means a document without a root element */
BERESTA_API int beresta_document_create(const char* root_name, beresta_document* out);

/* Разбор байтов в любой поддерживаемой кодировке (UTF-8/16, windows-1251...).
   schema == NULL — без проверки по схеме
   Parses bytes in any supported encoding (UTF-8/16, windows-1251...).
   schema == NULL means no schema validation */
BERESTA_API int beresta_document_parse(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_document* out);

/* Разбор файла (кодировка определяется автоматически) / Parses a file (the encoding is detected automatically) */
BERESTA_API int beresta_document_parse_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_document* out);

/* Освобождение документа вместе со всеми его узлами (NULL допустим) */
/* Frees a document together with all its nodes (NULL is allowed) */
BERESTA_API void beresta_document_free(beresta_document document);

/* Глубокая копия документа — новый независимый документ / A deep copy of the document — a new independent document */
BERESTA_API int beresta_document_clone(beresta_document document, beresta_document* out);

/* Корневой элемент или NULL, если его нет / The root element or NULL if there is none */
BERESTA_API beresta_node beresta_document_root(beresta_document document);

/* Элемент с атрибутом типа ID (по DTD) или с атрибутом xml:id */
/* An element with an ID-typed attribute (by DTD) or with an xml:id attribute */
BERESTA_API beresta_node beresta_document_element_by_id(beresta_document document, const char* id);

/* Запись документа в файл; encoding == NULL — UTF-8 / Writes the document to a file; encoding == NULL means UTF-8 */
BERESTA_API int beresta_document_save(beresta_document document, const char* path,
    uint32_t flags, const char* encoding);

/* =========================================================================
**  Узлы: свойства и навигация / Nodes: properties and navigation
** ========================================================================= */

/* Тип узла (значение beresta_node_kind) или -1 при ошибке */
/* The node type (a beresta_node_kind value) or -1 on error */
BERESTA_API int beresta_node_type(beresta_node node);

/* Имя: тег элемента, цель PI, имя DOCTYPE; "#text", "#comment" и т.п. */
/* The name: the element tag, the PI target, the DOCTYPE name; "#text", "#comment" and so on */
BERESTA_API char* beresta_node_name(beresta_node node);

/* Текстовое содержимое узла (у элемента — весь текст потомков) */
/* The text content of the node (for an element — all text of its descendants) */
BERESTA_API char* beresta_node_text(beresta_node node);

/* Сериализация узла и его потомков в строку UTF-8 / Serializes the node and its descendants into a UTF-8 string */
BERESTA_API char* beresta_node_to_string(beresta_node node, uint32_t flags);

BERESTA_API beresta_node beresta_node_parent(beresta_node node);
BERESTA_API beresta_node beresta_node_first_child(beresta_node node);
BERESTA_API beresta_node beresta_node_last_child(beresta_node node);
BERESTA_API beresta_node beresta_node_next_sibling(beresta_node node);
BERESTA_API beresta_node beresta_node_previous_sibling(beresta_node node);

/* Документ, которому принадлежит узел (у документа — он сам) */
/* The document the node belongs to (for a document — itself) */
BERESTA_API beresta_document beresta_node_owner_document(beresta_node node);

/* Число дочерних узлов (0 у листовых узлов) или -1 при ошибке */
/* The number of children (0 for leaf nodes) or -1 on error */
BERESTA_API int64_t beresta_node_child_count(beresta_node node);

/* Дочерний узел по индексу / A child node by index */
BERESTA_API beresta_node beresta_node_child(beresta_node node, int64_t index);

/* =========================================================================
**  Узлы: изменение дерева / Nodes: changing the tree
** ========================================================================= */

/* Замена текста: у элемента — все дети заменяются одним текстовым узлом,
   у текста / комментария / CDATA / PI — новое значение
   Replaces the text: for an element all children are replaced with one text node,
   for a text / comment / CDATA / PI — a new value */
BERESTA_API int beresta_node_set_text(beresta_node node, const char* text);

/* Удаление узла из дерева вместе с потомками; дескриптор становится недействительным */
/* Removes the node from the tree with its descendants; the handle becomes invalid */
BERESTA_API int beresta_node_remove(beresta_node node);

/* Глубокая копия source в конец детей parent (можно между документами) */
/* A deep copy of source appended to the children of parent (between documents too) */
BERESTA_API int beresta_node_copy_to(beresta_node source, beresta_node parent, beresta_node* out);

/* Перенос узла (без копирования) в конец детей parent, в том числе в другой
   документ. Дескрипторы узла и его потомков остаются действительными и
   теперь принадлежат документу parent: освобождение прежнего документа их
   не затрагивает. Ошибки (дерево при этом не меняется):
     BERESTA_INVALID_ARGUMENT    — NULL, parent не элемент и не документ,
                                   node — сам документ;
     BERESTA_HIERARCHY_VIOLATION — node — корневой элемент документа, parent —
                                   сам node или его потомок, или parent не
                                   принимает узел такого типа (текст в документ).
   Moves a node (without copying) to the end of the children of parent, including
   into another document. The handles of the node and its descendants stay valid
   and now belong to the document of parent: freeing the former document does not
   touch them. Errors (the tree does not change then):
     BERESTA_INVALID_ARGUMENT    — NULL, parent is neither an element nor a document,
                                   node is a document itself;
     BERESTA_HIERARCHY_VIOLATION — node is the root element of a document, parent is
                                   node itself or its descendant, or parent does not
                                   accept a node of this type (text into a document). */
BERESTA_API int beresta_node_move_to(beresta_node node, beresta_node parent);

/* Добавление нового узла в конец детей parent (элемента или документа).
   У документа append_element задаёт корневой элемент
   Appends a new node to the end of the children of parent (an element or a document).
   For a document append_element sets the root element */
BERESTA_API beresta_node beresta_node_append_element(beresta_node parent, const char* name);
BERESTA_API beresta_node beresta_node_append_text(beresta_node parent, const char* text);
BERESTA_API beresta_node beresta_node_append_comment(beresta_node parent, const char* text);
BERESTA_API beresta_node beresta_node_append_cdata(beresta_node parent, const char* text);
BERESTA_API beresta_node beresta_node_append_pi(beresta_node parent, const char* target, const char* data);

/* =========================================================================
**  Элементы и атрибуты / Elements and attributes
** ========================================================================= */

/* Значение атрибута или NULL (код 0), если атрибута нет */
/* The attribute value or NULL (code 0) if there is no such attribute */
BERESTA_API char* beresta_element_get_attribute(beresta_node element, const char* name);

BERESTA_API int beresta_element_set_attribute(beresta_node element, const char* name, const char* value);

/* 1 — атрибут есть, 0 — нет, -1 — ошибка / 1 — the attribute exists, 0 — it does not, -1 — an error */
BERESTA_API int beresta_element_has_attribute(beresta_node element, const char* name);

/* 1 — атрибут удалён, 0 — его не было, -1 — ошибка */
/* 1 — the attribute is removed, 0 — it did not exist, -1 — an error */
BERESTA_API int beresta_element_remove_attribute(beresta_node element, const char* name);

/* Перебор атрибутов в порядке документа / Iterating attributes in document order */
BERESTA_API int64_t beresta_element_attribute_count(beresta_node element);
BERESTA_API char* beresta_element_attribute_name(beresta_node element, int64_t index);
BERESTA_API char* beresta_element_attribute_value(beresta_node element, int64_t index);

/* Локальное имя (без префикса) / The local name (without a prefix) */
BERESTA_API char* beresta_element_local_name(beresta_node element);

/* URI пространства имён или NULL (код 0), если элемент вне пространств имён */
/* The namespace URI or NULL (code 0) if the element is in no namespace */
BERESTA_API char* beresta_element_namespace_uri(beresta_node element);

/* =========================================================================
**  XPath 1.0
** ========================================================================= */

/* Компиляция выражения (один раз — вычислять можно многократно) */
/* Compiles an expression (once — it can be evaluated many times) */
BERESTA_API int beresta_xpath_compile(const char* expression, beresta_xpath* out);
BERESTA_API void beresta_xpath_free(beresta_xpath xpath);

/* Привязка префикса к URI для имён вида p:name в выражении */
/* Binds a prefix to a URI for p:name names in the expression */
BERESTA_API int beresta_xpath_bind_namespace(beresta_xpath xpath, const char* prefix, const char* uri);

/* Значения переменных $name / Values of the $name variables */
BERESTA_API int beresta_xpath_set_string(beresta_xpath xpath, const char* name, const char* value);
BERESTA_API int beresta_xpath_set_number(beresta_xpath xpath, const char* name, double value);

/* Вычисление: множество узлов / строка / число / логическое (1, 0, -1 — ошибка) */
/* Evaluation: a node set / a string / a number / a boolean (1, 0, -1 — an error) */
BERESTA_API int beresta_xpath_select(beresta_xpath xpath, beresta_node context, beresta_nodeset* out);
BERESTA_API char* beresta_xpath_string(beresta_xpath xpath, beresta_node context);
BERESTA_API int beresta_xpath_number(beresta_xpath xpath, beresta_node context, double* out);
BERESTA_API int beresta_xpath_boolean(beresta_xpath xpath, beresta_node context);

/* Результат select: размер, узел (NULL для атрибута), строковое значение */
/* The select result: the size, a node (NULL for an attribute), a string value */
BERESTA_API int64_t beresta_nodeset_size(beresta_nodeset nodeset);
BERESTA_API beresta_node beresta_nodeset_node(beresta_nodeset nodeset, int64_t index);
BERESTA_API char* beresta_nodeset_value(beresta_nodeset nodeset, int64_t index);
BERESTA_API void beresta_nodeset_free(beresta_nodeset nodeset);

/* =========================================================================
**  XML Schema
** ========================================================================= */

/* Загрузка схемы из байтов XSD (xs:include / xs:import недоступны) */
/* Loads a schema from XSD bytes (xs:include / xs:import are unavailable) */
BERESTA_API int beresta_schema_load(const void* data, int64_t size, beresta_schema* out);

/* Загрузка из файла: xs:include / xs:import ищутся в каталоге этого файла */
/* Loads from a file: xs:include / xs:import are looked up in the directory of this file */
BERESTA_API int beresta_schema_load_file(const char* path, beresta_schema* out);
BERESTA_API void beresta_schema_free(beresta_schema schema);

/* Проверка документа: все нарушения собираются за один проход в out */
/* Validates the document: all violations are collected in one pass into out */
BERESTA_API int beresta_schema_validate(beresta_schema schema, beresta_document document,
    beresta_diagnostics* out);

/* Потоковая проверка без дерева (SAX + XSD): аргументы те же, что у
   beresta_document_parse / beresta_document_parse_file, но вместо документа
   в out — список нарушений. Память — O(глубины документа) плюс значения ID
   и таблицы ключей; файл отображается в память, а не копируется, поэтому
   так проверяются выгрузки в гигабайты. Нарушение схемы — не ошибка вызова
   (BERESTA_OK, нарушения в out); синтаксическая ошибка XML — BERESTA_PARSE_ERROR.
   Streaming validation without a tree (SAX + XSD): the arguments are the same as
   for beresta_document_parse / beresta_document_parse_file, but out receives a
   list of violations instead of a document. The memory is O(document depth) plus
   ID values and key tables; the file is mapped into memory, not copied, so
   exports of gigabytes are validated this way. A schema violation is not a call
   error (BERESTA_OK, the violations are in out); an XML syntax error is BERESTA_PARSE_ERROR. */
BERESTA_API int beresta_schema_validate_bytes(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_diagnostics* out);
BERESTA_API int beresta_schema_validate_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_diagnostics* out);

/* Список нарушений: число, текст, строка и столбец в исходнике (0 — неизвестно) */
/* The list of violations: the count, the text, the line and column in the source (0 — unknown) */
BERESTA_API int64_t beresta_diagnostics_count(beresta_diagnostics diagnostics);
BERESTA_API char* beresta_diagnostics_message(beresta_diagnostics diagnostics, int64_t index);
BERESTA_API int64_t beresta_diagnostics_line(beresta_diagnostics diagnostics, int64_t index);
BERESTA_API int64_t beresta_diagnostics_column(beresta_diagnostics diagnostics, int64_t index);
BERESTA_API void beresta_diagnostics_free(beresta_diagnostics diagnostics);

#ifdef __cplusplus
}
#endif

#endif
