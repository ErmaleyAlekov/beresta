# Прогон W3C XML Conformance Test Suite

Прогонщик разбирает каждый случай официального набора W3C/OASIS
(`xmlts20130923`, 2585 случаев) через `beresta::XmlParser` и сравнивает
результат с ожидаемым. Сам набор в репозиторий не кладётся
(`.gitignore` рядом), его скачивает `make fetch`.

## Запуск

```bash
cd tests/conformance
make fetch      # скачать и распаковать набор в xmlconf/ (curl + tar)
make run        # собрать библиотеку и прогонщик, прогнать, записать conformance_report.md
```

То же через CMake (MSVC, MinGW, GCC, Clang):

```bash
cmake -S tests/conformance -B build-conformance
cmake --build build-conformance --config Release --target conformance_run
```

Отладка одного случая: `bin/conformance_runner xmlconf --only <ID> --verbose`.
Код возврата 0 — прогон выполнен (провалы — в отчёте), 2 — сбой окружения.

## Как выносится вердикт

| TYPE | Настройки разбора | Случай пройден, если |
|---|---|---|
| `valid` | проверка DTD, внешний DTD и сущности | нет ни фатальной ошибки, ни ошибки валидности; при наличии OUTPUT канонический вывод совпал побайтно |
| `invalid` | проверка DTD | документ принят и найдена хотя бы одна ошибка валидности |
| `not-wf` | внешний DTD и сущности (без проверки DTD) | документ отвергнут фатальной ошибкой |
| `error` | как `not-wf` | любой штатный исход (стандарт допускает и сообщение, и молчание) |

Каталог `xmlconf.xml` читается самой «Берёстой» (внешние сущности, `xml:base`).
Канонический вывод — форма Джеймса Кларка; у второй формы (`<!DOCTYPE … <!NOTATION`)
сравнивается тело документа, PI из внутреннего подмножества, которые выводы IBM
выносят перед DOCTYPE, допускаются (у DOM для них нет места).

Не применимы и пропускаются: случаи XML 1.1 и Namespaces 1.1 (библиотека
реализует XML 1.0) и случаи с `EDITION="1 2 3 4"` (библиотека следует
5-й редакции). У девяти случаев `hst-*` каталог 2013 года указывает неверный
`xml:base`; файл ищется рядом с подкаталогом, где объявлен случай, — как того
требует XML Base для элементов внешней сущности.

## Что нашёл первый прогон и что исправлено

Первый прогон: **90,2 %** применимых случаев (valid 95,1 %, invalid 69,4 %,
not-wf 91,1 %). После исправлений в `src/` — **100 %** (см. `conformance_report.md`).
Регрессионные тесты: `tests/src/parser/w3c_syntax_tests.cpp`,
`tests/src/dtd/w3c_entity_tests.cpp`, `tests/src/dtd/w3c_validity_tests.cpp`,
`tests/src/encoding/w3c_encoding_tests.cpp`.

| Расхождение | Случаи W3C | Исправление |
|---|---|---|
| `<?xml version='1.0' standalone='yes'?>` без encoding отвергался: пробел перед `standalone` «съедался» | sa01, not-sa01…, o-p32pass1 (40 шт.) | `parser/internal/declaration_reader.cpp` |
| Концы строк нормализовались поблочно: `&#13;` из сущности превращался в `\n`, `\r\n` в CDATA не нормализовался | valid-sa-068, -110, -116 | нормализация один раз на входе: `encoding/decoding_steps.cpp` |
| Хвостовые пробелы данных PI отбрасывались | valid-sa-017, ibm16v03 | `parser/session/parse_markup.cpp` |
| Любой байт ≥ 0x80 считался буквой имени | ibm04n*, ibm85n*–89n*, o-p05fail4 | таблицы NameStartChar/NameChar 5-й редакции: `utils/xml_name_chars.cpp` |
| PI и комментарии в DTD не проверялись (`<?xml?>` внутри DTD, `--` в комментарии, неверная цель) | not-wf-sa-149, pi, ibm29n01, ibm85n01 | `dtd/parser/dtd_parser_markup.cpp` |
| Текстовая декларация внешней сущности не проверялась (encoding обязателен, standalone запрещён, порядок, версия 1.1) | dtd07, encoding07, decl01, ibm77n01, ibm79n02, E38 | `parser/internal/declaration_reader.cpp` |
| Символы PUBLIC-идентификатора не проверялись | pubid01…04, o-p12fail* | `utils/xml_tokens.cpp` |
| standalone="yes" не запрещал внешне объявленные сущности | not-sa03, ibm32n09, ibm68n06 | `parse_text_reference.cpp`, `attribute_value_decoder.cpp` |
| Объявление, начатое в PE и законченное вне её, принималось | ibm28an01 | номера кадров PE: `dtd/reader/dtd_reader_frames.cpp` |
| `:` в целях PI, именах сущностей и нотаций, префикс `xmlns` у элемента | rmt-ns10-042…044, NE13c | `parse_markup.cpp`, `dtd_parser_markup.cpp`, `parse_namespaces.cpp` |
| Внешняя PE во внутреннем подмножестве не считалась внешним текстом; systemId вложенной сущности разрешался от документа | E18, E60 | `dtd_reader_entities.cpp` (база — URI объявления) |
| Необъявленная сущность при `%pe;`/внешнем DTD была фатальной | E13 | ошибка валидности + пропуск ссылки |
| `SYSTEM ""` считался отсутствующим идентификатором | ibm11v01, ibm11v02 | `ExternalId::present` |
| BOM противоречит encoding декларации — молча побеждал BOM | hst-lhs-007, -008 | `checkDeclaredEncoding` |
| VC объявлений DTD не проверялись: дубли элементов и типов в mixed, два ID, умолчание ID, NOTATION-атрибуты, дубли перечислений, лексика умолчаний, нотация NDATA | el04, dtd01, id03, id05, attr04, attr11–16, E2a/b, E06*, E9b, ibm45i01… | `dtd/validator/dtd_declaration_checks.cpp`, `dtd_attribute_checks.cpp` |
| VC содержимого: разметка в EMPTY, NMTOKENS с `&#9;`, `:` в ID при NS | E15a–c, E20, rmt-ns10-045/046 | `dtd_validator_content.cpp`, `dtd_validator_attributes.cpp` |
| VC Standalone Document Declaration (умолчания, нормализация, пробелы из внешнего DTD) | not-sa01…14, ibm32i01… | `dtd_validator_standalone.cpp` |
| VC вложенности групп, объявлений и условных секций в PE | invalid--002, -005, -006, not-sa-022, ibm49i01, ibm50i01 | `dtd_parser_elements.cpp`, `dtd_parser_misc.cpp` |
