/* SPDX-License-Identifier: MIT — Берёста, см. файл LICENSE в корне проекта */
/* SPDX-License-Identifier: MIT — Beresta, see the LICENSE file in the project root */

/*
** Объявления Си-интерфейса для PHP FFI (FFI::cdef / FFI::load).
**
** Это те же 71 функция, что и в include/capi/beresta_c.h, но без
** препроцессора: разборщик PHP FFI не понимает #include, #ifdef и макросы
** (BERESTA_API), поэтому заголовок повторён в «чистом» виде. Константы
** (коды состояния, флаги, типы узлов) живут на стороне PHP — в перечислениях
** и классах флагов. Совпадение имён с beresta_c.h проверяет тест
** tests/HeaderSyncTest.php, а наличие каждой функции в библиотеке — сам
** FFI::cdef: он ищет все символы сразу при загрузке.
**
** Declarations of the C interface for PHP FFI (FFI::cdef / FFI::load).
**
** These are the same 71 functions as in include/capi/beresta_c.h, but without
** the preprocessor: the PHP FFI parser does not understand #include, #ifdef and
** macros (BERESTA_API), so the header is repeated in a "clean" form. The constants
** (status codes, flags, node types) live on the PHP side — in enums and flag
** classes. The test tests/HeaderSyncTest.php checks that the names match
** beresta_c.h, and FFI::cdef itself checks that every function exists in the
** library: it looks up all symbols at once when loading.
*/

/* Непрозрачные дескрипторы / Opaque handles */
typedef void* beresta_document;
typedef void* beresta_node;
typedef void* beresta_xpath;
typedef void* beresta_nodeset;
typedef void* beresta_schema;
typedef void* beresta_diagnostics;

/* Библиотека и ошибки / Library and errors */
const char* beresta_version(void);
int beresta_last_error_code(void);
const char* beresta_last_error_message(void);
const char* beresta_status_name(int status);
void beresta_string_free(char* text);

/* Языки сообщений / Message languages */
int beresta_set_language(const char* code);
const char* beresta_language(void);
const char* beresta_languages(void);
const char* beresta_message_text(int message);

/* Документы / Documents */
int beresta_document_create(const char* root_name, beresta_document* out);
int beresta_document_parse(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_document* out);
int beresta_document_parse_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_document* out);
void beresta_document_free(beresta_document document);
int beresta_document_clone(beresta_document document, beresta_document* out);
beresta_node beresta_document_root(beresta_document document);
beresta_node beresta_document_element_by_id(beresta_document document, const char* id);
int beresta_document_save(beresta_document document, const char* path,
    uint32_t flags, const char* encoding);

/* Узлы: свойства и навигация / Nodes: properties and navigation */
int beresta_node_type(beresta_node node);
char* beresta_node_name(beresta_node node);
char* beresta_node_text(beresta_node node);
char* beresta_node_to_string(beresta_node node, uint32_t flags);
beresta_node beresta_node_parent(beresta_node node);
beresta_node beresta_node_first_child(beresta_node node);
beresta_node beresta_node_last_child(beresta_node node);
beresta_node beresta_node_next_sibling(beresta_node node);
beresta_node beresta_node_previous_sibling(beresta_node node);
beresta_document beresta_node_owner_document(beresta_node node);
int64_t beresta_node_child_count(beresta_node node);
beresta_node beresta_node_child(beresta_node node, int64_t index);

/* Узлы: изменение дерева / Nodes: changing the tree */
int beresta_node_set_text(beresta_node node, const char* text);
int beresta_node_remove(beresta_node node);
int beresta_node_copy_to(beresta_node source, beresta_node parent, beresta_node* out);
int beresta_node_move_to(beresta_node node, beresta_node parent);
beresta_node beresta_node_append_element(beresta_node parent, const char* name);
beresta_node beresta_node_append_text(beresta_node parent, const char* text);
beresta_node beresta_node_append_comment(beresta_node parent, const char* text);
beresta_node beresta_node_append_cdata(beresta_node parent, const char* text);
beresta_node beresta_node_append_pi(beresta_node parent, const char* target, const char* data);

/* Элементы и атрибуты / Elements and attributes */
char* beresta_element_get_attribute(beresta_node element, const char* name);
int beresta_element_set_attribute(beresta_node element, const char* name, const char* value);
int beresta_element_has_attribute(beresta_node element, const char* name);
int beresta_element_remove_attribute(beresta_node element, const char* name);
int64_t beresta_element_attribute_count(beresta_node element);
char* beresta_element_attribute_name(beresta_node element, int64_t index);
char* beresta_element_attribute_value(beresta_node element, int64_t index);
char* beresta_element_local_name(beresta_node element);
char* beresta_element_namespace_uri(beresta_node element);

/* XPath 1.0 */
int beresta_xpath_compile(const char* expression, beresta_xpath* out);
void beresta_xpath_free(beresta_xpath xpath);
int beresta_xpath_bind_namespace(beresta_xpath xpath, const char* prefix, const char* uri);
int beresta_xpath_set_string(beresta_xpath xpath, const char* name, const char* value);
int beresta_xpath_set_number(beresta_xpath xpath, const char* name, double value);
int beresta_xpath_select(beresta_xpath xpath, beresta_node context, beresta_nodeset* out);
char* beresta_xpath_string(beresta_xpath xpath, beresta_node context);
int beresta_xpath_number(beresta_xpath xpath, beresta_node context, double* out);
int beresta_xpath_boolean(beresta_xpath xpath, beresta_node context);
int64_t beresta_nodeset_size(beresta_nodeset nodeset);
beresta_node beresta_nodeset_node(beresta_nodeset nodeset, int64_t index);
char* beresta_nodeset_value(beresta_nodeset nodeset, int64_t index);
void beresta_nodeset_free(beresta_nodeset nodeset);

/* XML Schema */
int beresta_schema_load(const void* data, int64_t size, beresta_schema* out);
int beresta_schema_load_file(const char* path, beresta_schema* out);
void beresta_schema_free(beresta_schema schema);
int beresta_schema_validate(beresta_schema schema, beresta_document document,
    beresta_diagnostics* out);
int beresta_schema_validate_bytes(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_diagnostics* out);
int beresta_schema_validate_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_diagnostics* out);
int64_t beresta_diagnostics_count(beresta_diagnostics diagnostics);
char* beresta_diagnostics_message(beresta_diagnostics diagnostics, int64_t index);
int64_t beresta_diagnostics_line(beresta_diagnostics diagnostics, int64_t index);
int64_t beresta_diagnostics_column(beresta_diagnostics diagnostics, int64_t index);
void beresta_diagnostics_free(beresta_diagnostics diagnostics);
