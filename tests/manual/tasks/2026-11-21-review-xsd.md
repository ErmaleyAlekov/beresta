# Ревью кода: XSD, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-18. Ревью: XSD: регулярные выражения

- **Зачем.** Свой движок регулярок XSD с гарантией линейного времени. Шаблон приходит из схемы (возможно, чужой). Риски — экспоненциальное время, взрыв числа состояний при `{n,m}`, неверные классы Unicode.
- **Приоритет:** желательно до следующего выпуска · ~70–120 минут (файлов: 18, строк: ≈ 1874 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-18/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/regex/char_set.hpp` (66)
  2. `include/xsd/regex/unicode_tables.hpp` (80)
  3. `include/xsd/regex/xsd_regex.hpp` (175)
  4. `src/xsd/regex/char_set.cpp` (87)
  5. `src/xsd/regex/char_set_normalize.cpp` (32)
  6. `src/xsd/regex/multi_char_escapes.cpp` (65)
  7. `src/xsd/regex/name_char_table.cpp` (88)
  8. `src/xsd/regex/unicode_block_table.cpp` (38)
  9. `src/xsd/regex/unicode_category_table.cpp` (412)
  10. `src/xsd/regex/unicode_properties.cpp` (113)
  11. `src/xsd/regex/xsd_regex_build.cpp` (85)
  12. `src/xsd/regex/xsd_regex_chars.cpp` (59)
  13. `src/xsd/regex/xsd_regex_classes.cpp` (125)
  14. `src/xsd/regex/xsd_regex_compile.cpp` (110)
  15. `src/xsd/regex/xsd_regex_escapes.cpp` (79)
  16. `src/xsd/regex/xsd_regex_match.cpp` (77)
  17. `src/xsd/regex/xsd_regex_parser.cpp` (86)
  18. `src/xsd/regex/xsd_regex_quantifier.cpp` (97)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Линейность: `(a*)*b`, `(a|a)*b`, `(a|aa)*c` на длинной строке; `a{1000}{1000}` — память и время построения, есть ли предел.
  2. `xsd_regex_quantifier`: `{n,m}` с n > m, огромные числа, переполнение `int`.
  3. Классы: вычитание `[a-z-[aeiou]]`, вложенное вычитание, `\p{Is...}` блоки, `\i`/`\c`, `.` (без `\n`/`\r`).
  4. Таблицы Unicode (`unicode_*_table.cpp`, `name_char_table.cpp` — сгенерированы): только поиск по ним — двоичный поиск на краях, кодовые точки > U+FFFF.
  5. `char_set_normalize`: слияние пересекающихся диапазонов, пустой набор.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** src/xsd/regex/xsd_regex_classes.cpp:74-83 В parseClassItem не проверяется, что началом диапазона first не является неэкранированный дефис -, из-за чего выражения вида [--a] и [a-b-c] не отклоняются как синтаксическая ошибка по грамматике XSD 1.0 (продукция XmlChar), а молча компилируются в диапазон от '-' (0x2D) до 'a' или 'c', включая туда цифры и знаки препинания. Воспроизводится на любом шаблоне с двойным дефисом или цепочкой диапазонов без экранирования, например Regex("[a-b-c]").matches("0") вернёт true.
src/xsd/regex/xsd_regex_classes.cpp:36-47 В parseClassGroup вычитание -[...] разрешается в самом начале класса при пустом левом множестве ([- [a]] или [-[a]]), так как цикл while вызывает applySubtraction до проверки, что до -[ был разобран хотя бы один элемент posCharGroup. Воспроизводится на шаблоне Regex("[-[a]]"), который по спецификации XSD 1.0 невалиден, но успешно компилируется в пустое множество.
src/xsd/regex/xsd_regex_escapes.cpp:54-60 В parseProperty закрывающая скобка } ищется через _pattern.find('}', _position) по всей оставшейся строке шаблона, поэтому незакрытый блок \p{Lu при наличии квантификатора дальше по шаблону (например, \p{Lu}a{1,2} без первой }: \p{Lu a{1,2}) захватывает чужую скобку и выбрасывает ошибку неизвестного свойства вместо синтаксической ошибки незакрытого \p{...}.
src/xsd/regex/xsd_regex_match.cpp:22 В addReachable при каждом переходе по SPLIT или EPSILON создаётся локальный std::vector<long> stack{state}, что вызывает динамическую аллокацию в куче на каждый активный переход на каждом символе входной строки при вызове matches()

## Т-19. Ревью: XSD: простые типы, лексика значений и фасеты

- **Зачем.** Проверка значений встроенных типов (числа, даты, длительности, строки, QName) и фасетов (длины, границы, шаблоны, перечисления). Риски — переполнения в датах и числах, неверные границы, сравнение несравнимого.
- **Приоритет:** желательно до следующего выпуска · ~100–160 минут (файлов: 24, строк: ≈ 2376 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-19/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/simple_type.hpp` (162)
  2. `include/xsd/prefix_resolver.hpp` (49)
  3. `include/xsd/internal/datatype_checks.hpp` (104)
  4. `include/xsd/internal/facet_rules.hpp` (82)
  5. `src/xsd/datatypes/builtin_types.cpp` (150)
  6. `src/xsd/datatypes/duration_order.cpp` (154)
  7. `src/xsd/datatypes/facet_checks.cpp` (125)
  8. `src/xsd/datatypes/lexical_date_parts.cpp` (99)
  9. `src/xsd/datatypes/lexical_dates.cpp` (103)
  10. `src/xsd/datatypes/lexical_duration.cpp` (69)
  11. `src/xsd/datatypes/lexical_numbers.cpp` (91)
  12. `src/xsd/datatypes/lexical_strings.cpp` (74)
  13. `src/xsd/datatypes/pattern_enumeration.cpp` (57)
  14. `src/xsd/datatypes/prefix_resolver.cpp` (31)
  15. `src/xsd/datatypes/simple_value.cpp` (142)
  16. `src/xsd/datatypes/value_canonical.cpp` (155)
  17. `src/xsd/datatypes/value_compare.cpp` (17)
  18. `src/xsd/datatypes/value_order.cpp` (107)
  19. `src/xsd/datatypes/whitespace.cpp` (110)
  20. `src/xsd/schema/facet_rules_bounds.cpp` (152)
  21. `src/xsd/schema/facet_rules_check.cpp` (93)
  22. `src/xsd/schema/facet_rules_kinds.cpp` (62)
  23. `src/xsd/schema/facet_rules_lengths.cpp` (100)
  24. `src/xsd/schema/facet_values.cpp` (88)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `lexical_dates` / `lexical_date_parts`: год 0000, отрицательные и > 9999 годы, 24:00:00, часовой пояс ±14:00 и ±14:01, 29 февраля.
  2. `lexical_duration` / `duration_order`: частичный порядок длительностей (P1M и P30D несравнимы), переполнение при переводе в секунды.
  3. `lexical_numbers`: `INF`, `-INF`, `NaN`, `+INF` (недопустим в 1.0), ведущие нули, длинные integer за пределами 64 бит.
  4. `facet_rules_*`: ограничение фасета у производного типа (minLength базы ≤ производного), `totalDigits`/`fractionDigits` на граничных значениях, фасеты на списках и объединениях.
  5. `whitespace`: preserve/replace/collapse применяются до проверки шаблона и перечисления.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** value_order.cpp:43 — При парсинге float или double со значением ноль и огромной экспонентой вроде "0e5000" мантисса 0.0L умножается на HUGE_VALL от std::pow(10.0L, 5000), что даёт NaN вместо 0.0L. На входе "0e5000" при проверке фасета minInclusive="0" сравнение вернёт 2 (несравнимы) и отклонит валидный ноль.
value_order.cpp:25 и value_order.cpp:43 — При парсинге float или double с очень длинной мантиссой и отрицательной экспонентой вроде "1" с 5000 нулями и "e-5000" parseUnsigned переполняет мантиссу в HUGE_VALL до умножения на отрицательную степень, возвращая INF вместо 1.0. Вход "1000...0e-5000" (5000 нулей) для xs:double с фасетом maxInclusive="10" ошибочно посчитается бесконечностью и не пройдёт проверку.
duration_order.cpp:102 — При сравнении двух огромных длительностей с компонентами более 308–4932 цифр parts.months или parts.seconds переполняются в inf, а вычитание shiftedMoment(..., a) - shiftedMoment(..., b) даёт inf - inf = NaN. Вход "P" с 5000 девятками и "Y" для xs:duration с фасетом maxInclusive на это же значение вернёт 2 (несравнимы) и отклонит валидное граничное значение.
simple_value.cpp:58 — В validateList указатель type.itemType разыменовывается без проверки на nullptr, тогда как в value_canonical.cpp:109 аналогичная проверка присутствует. Вызов validateSimpleValue на SimpleType с variety == Variety::LIST и itemType == nullptr для любой непустой строки "a" приведёт к разыменовке нулевого указателя.
pattern_enumeration.cpp:25 — В checkPatternsAndEnumeration при пустом вложенном векторе шаблонов group цикл не выполняется и group.front() вызывается на пустом векторе для формирования текста ошибки. Вызов validateSimpleValue на типе, где в facets.patterns добавлен пустой std::vector<std::shared_ptr<const Regex>>, вызовет неопределённое поведение при обращении к group.front()

## Т-20. Ревью: XSD: сборка схемы — основа, имена, элементы, атрибуты, частицы

- **Зачем.** Схема — тоже недоверенный вход (схемы скачивают из реестров и присылают с документами). Сборка читает XML схемы в модель: имена, ссылки, вложенность, элементы, атрибуты, частицы и occurs. Риски — бесконечные циклы по ссылкам, неверный порядок разрешения отложенных ссылок.
- **Приоритет:** желательно до следующего выпуска · ~110–190 минут (файлов: 22, строк: ≈ 2830 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-20/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/schema.hpp` (312)
  2. `include/xsd/internal/schema_builder.hpp` (451)
  3. `include/xsd/internal/attribute_wildcards.hpp` (40)
  4. `src/xsd/schema/schema.cpp` (82)
  5. `src/xsd/schema/schema_lookup.cpp` (67)
  6. `src/xsd/schema/schema_versioning.cpp` (47)
  7. `src/xsd/schema/schema_xml_helpers.cpp` (71)
  8. `src/xsd/schema/schema_builder.cpp` (119)
  9. `src/xsd/schema/schema_builder_names.cpp` (114)
  10. `src/xsd/schema/schema_builder_register.cpp` (116)
  11. `src/xsd/schema/schema_builder_pending.cpp` (158)
  12. `src/xsd/schema/schema_builder_structure.cpp` (178)
  13. `src/xsd/schema/schema_builder_placement.cpp` (61)
  14. `src/xsd/schema/schema_builder_nesting.cpp` (25)
  15. `src/xsd/schema/schema_builder_elements.cpp` (142)
  16. `src/xsd/schema/schema_builder_attributes.cpp` (133)
  17. `src/xsd/schema/schema_builder_attribute_form.cpp` (98)
  18. `src/xsd/schema/schema_builder_complex.cpp` (126)
  19. `src/xsd/schema/schema_builder_particles.cpp` (152)
  20. `src/xsd/schema/schema_builder_occurs.cpp` (65)
  21. `src/xsd/schema/schema_builder_wildcards.cpp` (114)
  22. `src/xsd/schema/attribute_wildcards.cpp` (159)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `schema_builder_pending` / `_register`: ссылка на компонент, объявленный позже; дубликат имени в одном пространстве; ссылка на несуществующий компонент — ошибка, а не пустой указатель.
  2. `schema_builder_nesting` / `_placement`: элемент в недопустимом месте схемы, глубина вложенности схемы (предел).
  3. `schema_builder_occurs`: `maxOccurs="unbounded"`, `minOccurs > maxOccurs`, числа больше 64 бит, ведущие `+`/пробелы.
  4. `schema_builder_complex` / `_particles` / `_wildcards`: пустой `sequence`, `any` с `##other` и отсутствующим targetNamespace, `processContents`.
  5. `schema_lookup`: поиск по QName с пустым и непустым пространством, `schema_versioning` (vc:minVersion).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** schema_builder_occurs.cpp:32: parseOccurs отклоняет ведущий знак + (разрешённый для xs:nonNegativeInteger), поэтому <xs:element name="a" minOccurs="+1"/> ошибочно выбрасывает SCHEMA_MIN_OCCURS.
schema_builder_occurs.cpp:54: для xs:all проверяется только maxOccurs > 1, из-за чего <xs:all minOccurs="2" maxOccurs="2"> выбрасывает SCHEMA_MAX_BELOW_MIN вместо SCHEMA_ALL_OCCURS.
schema_builder_complex.cpp:64: наличие любой частицы безусловно ставит ContentType::ELEMENT_ONLY, поэтому <xs:complexType name="T"><xs:sequence/></xs:complexType> получает ELEMENT_ONLY вместо EMPTY_CONTENT.
schema_builder_wildcards.cpp:35: readNamespaceConstraint не запрещает ##any и ##other внутри списка пространств имён и не убирает дубликаты, поэтому <xs:any namespace="##any ##local"/> проходит без ошибки.
schema_builder_elements.cpp:15, schema_builder_names.cpp:71: при ссылке на несуществующий глобальный элемент или тип fail вызывается с *_sources.front().root, из-за чего диагностика указывает на первую строку первого документа вместо строки элемента с ошибочным ref или type.
schema.cpp:25: в Schema::fromDocuments нет проверки элементов входного вектора на nullptr, поэтому вызов Schema::fromDocuments({nullptr}) приводит к разыменованию нулевого указателя.
schema_xml_helpers.cpp:58: parseProcessContents любое значение, отличное от "lax" и "skip", молча превращает в STRICT_CHECK, поэтому <xs:any processContents="invalid"/> не вызывает ошибку схемы.
schema_versioning.cpp:30: compareVersion молча игнорирует невалидный xs:decimal в vc:minVersion / vc:maxVersion (возвращая 2), поэтому <xs:element name="x" vc:minVersion="abc"/> включается в схему без ошибки

## Т-21. Ревью: XSD: сборка схемы — наследование, redefine, подстановки, final, значения

- **Зачем.** Самая сложная часть сборки: вывод типов (extension/restriction), `xs:redefine`, группы подстановки, `final`/`block`, значения по умолчанию и фиксированные, identity и assertions на уровне схемы. Здесь уже были ошибки с циклическим наследованием.
- **Приоритет:** желательно до следующего выпуска · ~80–130 минут (файлов: 16, строк: ≈ 1957 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-21/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/internal/schema_builder.hpp` (451)
  2. `include/xsd/internal/type_derivation.hpp` (32)
  3. `src/xsd/schema/type_derivation.cpp` (96)
  4. `src/xsd/schema/schema_builder_derivation.cpp` (99)
  5. `src/xsd/schema/schema_builder_redefine.cpp` (152)
  6. `src/xsd/schema/schema_builder_redefine_rules.cpp` (152)
  7. `src/xsd/schema/schema_builder_original.cpp` (78)
  8. `src/xsd/schema/schema_builder_substitution.cpp` (115)
  9. `src/xsd/schema/schema_builder_final.cpp` (95)
  10. `src/xsd/schema/schema_builder_simple.cpp` (144)
  11. `src/xsd/schema/schema_builder_facets.cpp` (76)
  12. `src/xsd/schema/schema_builder_lexical.cpp` (98)
  13. `src/xsd/schema/schema_builder_values.cpp` (77)
  14. `src/xsd/schema/schema_builder_identity.cpp` (122)
  15. `src/xsd/schema/schema_builder_identity_refs.cpp` (66)
  16. `src/xsd/schema/schema_builder_assertions.cpp` (104)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Циклы: тип наследует сам себя (прямо, через группу, через redefine), группа подстановки по кругу, атрибутная группа ссылается на себя — где обнаруживается, нет ли бесконечной рекурсии.
  2. `schema_builder_redefine*` / `_original`: redefine, ссылающийся на исходный компонент (self-reference), redefine несуществующего, цепочка redefine.
  3. `type_derivation`: проверка допустимости restriction (частицы, атрибуты, wildcard) — какие ограничения XSD 1.0 не проверяются (известные пробелы — опишите, а не только ошибки).
  4. `schema_builder_values`: `default` и `fixed` одновременно, значение не проходит тип, `fixed` у элемента со сложным типом.
  5. `schema_builder_final` / `_substitution`: `final="#all"`, `block` на элементе и типе, `finalDefault` схемы.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** schema_builder_simple.cpp:119 В buildMembers для xs:union вызывается resolveType вместо referencedType, из-за чего ссылка на собственное имя внутри xs:redefine ведёт на новый незавершённый тип вместо исходного. Воспроизводится через <xs:redefine> с <xs:simpleType name="T"><xs:union memberTypes="tns:T xs:string"/></xs:simpleType>.
schema_builder_redefine_rules.cpp:64-65 Атрибуты minOccurs и maxOccurs у ссылки на себя внутри xs:redefine сравниваются со строкой "1" без обрезки пробелов. Валидный XML <xs:group ref="tns:G" minOccurs=" 1 " maxOccurs=" 1 "/> внутри xs:redefine ошибочно падает с SCHEMA_REDEFINE_SELF_OCCURS.
schema_builder_facets.cpp:38 Отсутствие обязательного атрибута value у <xs:pattern/> или <xs:enumeration/> подменяется пустой строкой через value_or("") вместо ошибки схемы. Схема с пустым тегом <xs:pattern/> внутри <xs:restriction> молча принимается как регулярное выражение для пустой строки.
schema_builder_facets.cpp:60 В applyNamedFacet отсутствие атрибута value у фасета также маскируется через value_or(""), поэтому на <xs:minInclusive/> без value выдаётся ошибка неверного значения SCHEMA_FACET_INVALID вместо ошибки отсутствия обязательного атрибута.
type_derivation.cpp:40 В simpleDerivedFrom рекурсивный обход memberTypes у xs:union проверяет только глубину, но не посещённые узлы, что даёт экспоненциальное время работы O(2^N) на алмазовидном графе объединений. Воспроизводится цепочкой из 30 типов U_i, где каждый U_i объявлен как <xs:union memberTypes="tns:U_{i+1} tns:U_{i+1}"/>.
schema_builder_derivation.cpp:36-40 При построении xs:simpleContent/xs:restriction игнорируется вложенный анонимный <xs:simpleType>, допустимый по XSD 1.0 при ограничении xs:anySimpleType или другого simpleContent. Схема <xs:simpleContent><xs:restriction base="xs:anySimpleType"><xs:simpleType><xs:restriction base="xs:int"/></xs:simpleType></xs:restriction></xs:simpleContent> оставит типом текста xs:anySimpleType.
schema_builder_values.cpp:62 Для составного типа с MIXED_CONTENT проверка default/fixed сразу завершается без проверки того, допускает ли частица типа пустое содержимое (cos-valid-default.2.2.2). Элемент со смешанным типом, содержащим обязательный дочерний элемент minOccurs="1", ошибочно разрешает задавать default или fixed.
schema_builder_assertions.cpp:25-26 В defaultElementNamespace атрибут xpathDefaultNamespace ищется только на самом xs:assert и корневом xs:schema, игнорируя промежуточные элементы (xs:complexType, xs:simpleType, xs:restriction, xs:extension), где он разрешён по XSD 1.1. Если задать xpathDefaultNamespace="##targetNamespace" на xs:complexType, выражение внутри xs:assert будет искать элементы без пространства имён.
schema_builder_redefine.cpp:74-75 Проверка nested разрешает повторное переопределение, если владелец загружен внутри source.baseUri, не проверяя, что переопределение в самом source.baseUri уже выполнялось ранее. Если файл A подключает C через xs:include и переопределяет тип T, а затем в том же A переопределяет T ещё раз через xs:redefine файла B (который тоже включает C), ошибка SCHEMA_REDEFINE_TWICE не сработает.

## Т-22. Ревью: XSD: потоковая проверка документа и диагностика

- **Зачем.** Валидатор идёт по событиям документа: открытие/закрытие элемента, атрибуты, xsi:type/nil/schemaLocation, значения, ID. Риски — неверный стек при ошибке, `xsi:type` на неразрешённый тип, утечка состояния между документами.
- **Приоритет:** желательно до следующего выпуска · ~100–170 минут (файлов: 25, строк: ≈ 2499 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-22/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/schema_validator.hpp` (48)
  2. `include/xsd/internal/stream_validator.hpp` (265)
  3. `include/xsd/internal/id_registry.hpp` (73)
  4. `include/xsd/internal/diagnostic_log.hpp` (143)
  5. `include/xsd/internal/path_tracker.hpp` (119)
  6. `src/xsd/validation/id_registry.cpp` (95)
  7. `src/xsd/validation/schema_validator.cpp` (64)
  8. `src/xsd/validation/stream_assertions.cpp` (79)
  9. `src/xsd/validation/stream_attribute_rules.cpp` (96)
  10. `src/xsd/validation/stream_attributes.cpp` (154)
  11. `src/xsd/validation/stream_begin.cpp` (166)
  12. `src/xsd/validation/stream_child_declaration.cpp` (102)
  13. `src/xsd/validation/stream_close.cpp` (114)
  14. `src/xsd/validation/stream_events.cpp` (112)
  15. `src/xsd/validation/stream_identity_bridge.cpp` (75)
  16. `src/xsd/validation/stream_namespaces.cpp` (41)
  17. `src/xsd/validation/stream_open.cpp` (128)
  18. `src/xsd/validation/stream_report.cpp` (41)
  19. `src/xsd/validation/stream_validator.cpp` (51)
  20. `src/xsd/validation/stream_values.cpp` (103)
  21. `src/xsd/validation/stream_xsi.cpp` (82)
  22. `src/xsd/diagnostics/diagnostic_log.cpp` (76)
  23. `src/xsd/diagnostics/diagnostic_positions.cpp` (53)
  24. `src/xsd/diagnostics/path_tracker.cpp` (110)
  25. `src/xsd/diagnostics/path_tracker_counts.cpp` (109)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `stream_xsi`: `xsi:type` на тип, не выводимый из объявленного, на `block`-тип, на неизвестный тип; `xsi:nil` с содержимым; `xsi:schemaLocation` — загружается ли что-то без разрешения.
  2. `stream_open` / `stream_close`: стек состояний после ошибки в середине (продолжение проверки), лишний `close`.
  3. `stream_values` / `stream_attribute_rules`: значение по умолчанию, `fixed`, атрибут из wildcard с `processContents="strict"`.
  4. `id_registry`: ID/IDREF в XSD (тип `xs:ID`) — проверка в конце документа, дубликаты, сброс между документами.
  5. `diagnostic_log` / `path_tracker`: предел числа диагностик, путь к элементу на большой глубине (время построения пути), позиции.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** src/xsd/validation/stream_events.cpp:58 В StreamValidator::endElement() нет проверки if (_depth == 0) return; (в отличие от text() и otherChild()), поэтому лишнее событие закрытия элемента при пустом стеке обращается к _frames[static_cast<std::size_t>(-1)] и вызывает неопределённое поведение. Воспроизводится прямым вызовом endElement() без парного startElement() или повторным endElement() после закрытия корневого элемента.
src/xsd/validation/id_registry.cpp:25 Поле lengthPlusOne структуры IdSlot имеет тип std::uint32_t и инициализируется через static_cast<std::uint32_t>(value.size() + 1) без проверки верхнего предела. На 64-битной платформе атрибут или элемент типа xs:ID длиной 2^32 - 1 байт обернёт lengthPlusOne в 0 (признак пустого слота), что при заполнении таблицы приведёт к бесконечному циклу или потере записей, а длина 2^32 + k исказит срез в _bytes.
src/xsd/diagnostics/diagnostic_log.cpp:54 В DiagnosticLog (а также в StreamValidator::_references в src/xsd/validation/stream_values.cpp:64) отсутствует верхний предел на число накапливаемых ошибок и ссылок. Враждебный документ с миллионами невалидных элементов или висячих атрибутов xs:IDREF вызывает неограниченный рост векторов _pending, _references и шагов в PathTracker::_steps вплоть до исчерпания памяти.
src/xsd/validation/stream_xsi.cpp:25 Функция findXsi запоминает только первый встреченный атрибут type и nil из http://w3.org, игнорируя повторные. Если на одном элементе объявлены два разных префикса на этот URI (например, <el xmlns:a="http://w3.org" xmlns:b="http://w3.org" a:type="xs:int" b:type="xs:string"/>), второй атрибут молча отбрасывается без ошибки.

## Т-23. Ревью: XSD: события дерева, обработчик проверки и xs:assert

- **Зачем.** Связка валидатора с DOM и SAX (источник событий из дерева, ValidatingHandler) и проверка `xs:assert` через XPath. Риски — расхождение потоковой проверки и проверки дерева, запись поддерева для assert без предела памяти.
- **Приоритет:** когда будет время · ~60–100 минут (файлов: 21, строк: ≈ 1425 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-23/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/validating_handler.hpp` (116)
  2. `include/xsd/assertion.hpp` (34)
  3. `include/xsd/internal/tree_event_source.hpp` (53)
  4. `include/xsd/internal/prefix_bindings.hpp` (73)
  5. `include/xsd/internal/assertion_recorder.hpp` (153)
  6. `include/xsd/internal/assertion_values.hpp` (65)
  7. `include/xpath/internal/xpath_assertion.hpp` (54)
  8. `src/xsd/events/prefix_bindings.cpp` (88)
  9. `src/xsd/events/prefix_bindings_scope.cpp` (40)
  10. `src/xsd/events/tree_event_nodes.cpp` (76)
  11. `src/xsd/events/tree_event_source.cpp` (46)
  12. `src/xsd/events/validating_handler.cpp` (49)
  13. `src/xsd/events/validating_handler_content.cpp` (49)
  14. `src/xsd/events/validating_handler_document.cpp` (47)
  15. `src/xsd/events/validating_handler_misc.cpp` (31)
  16. `src/xsd/assertions/assertion_capture.cpp` (118)
  17. `src/xsd/assertions/assertion_facets.cpp` (57)
  18. `src/xsd/assertions/assertion_recorder.cpp` (80)
  19. `src/xsd/assertions/assertion_types.cpp` (71)
  20. `src/xsd/assertions/assertion_values.cpp` (78)
  21. `src/xpath/xpath_assertion.cpp` (47)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `tree_event_source` / `tree_event_nodes`: те же события, что дал бы разбор (пространства имён, пробельные узлы, CDATA).
  2. `validating_handler*`: исключение из обработчика пользователя за ValidatingHandler, порядок вызова вложенного обработчика.
  3. `assertion_recorder` / `assertion_capture`: память при записи огромного поддерева для assert — есть ли предел.
  4. `assertion_values` / `assertion_types`: `$value` для простого и сложного содержимого (недавняя правка 2.2.1), типизация атрибутов в assert.
  5. `xpath_assertion`: выражение assert компилируется один раз на схему, а не на каждый элемент.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-24. Ревью: XSD: ограничения идентичности (key, keyref, unique)

- **Зачем.** Две реализации — потоковая и по дереву — должны давать одинаковый ответ. Риски — квадратичное время на больших таблицах ключей, неверное сравнение значений разных типов, области видимости keyref.
- **Приоритет:** желательно до следующего выпуска · ~90–150 минут (файлов: 22, строк: ≈ 2214 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-24/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xsd/internal/identity_common.hpp` (33)
  2. `include/xsd/internal/identity_path.hpp` (119)
  3. `include/xsd/internal/identity_tracker.hpp` (112)
  4. `include/xsd/internal/identity_values.hpp` (63)
  5. `include/xsd/internal/stream_identity.hpp` (175)
  6. `include/xsd/internal/tree_identity.hpp` (215)
  7. `src/xsd/identity/identity_common.cpp` (93)
  8. `src/xsd/identity/identity_path_match.cpp` (58)
  9. `src/xsd/identity/identity_path_names.cpp` (79)
  10. `src/xsd/identity/identity_path_reader.cpp` (92)
  11. `src/xsd/identity/identity_values.cpp` (83)
  12. `src/xsd/identity/stream_identity.cpp` (104)
  13. `src/xsd/identity/stream_identity_fields.cpp` (85)
  14. `src/xsd/identity/stream_identity_match.cpp` (89)
  15. `src/xsd/identity/stream_identity_rows.cpp` (118)
  16. `src/xsd/identity/stream_identity_tables.cpp` (104)
  17. `src/xsd/identity/tree_identity_bind.cpp` (76)
  18. `src/xsd/identity/tree_identity_defaults.cpp` (120)
  19. `src/xsd/identity/tree_identity_recorder.cpp` (87)
  20. `src/xsd/identity/tree_identity_report.cpp` (67)
  21. `src/xsd/identity/tree_identity_rows.cpp` (153)
  22. `src/xsd/identity/tree_identity_tables.cpp` (89)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `identity_path_*`: разбор ограниченного XPath селектора/поля (`.//`, `|`, атрибуты, `child::`), отказ на недопустимых формах.
  2. `identity_values`: сравнение значений ключа по типу (`1` и `1.0` как decimal равны, как string — нет), пустое поле, поле с несколькими узлами — ошибка.
  3. `stream_identity_*` / `tree_identity_*`: одинаковые ответы для одних и тех же входов; keyref на key из внешней области видимости.
  4. Сложность: 100 000 ключей — хеш-таблица с ключом из `seeded_hash`, а не линейный поиск.
  5. Освобождение таблиц при выходе из области элемента — нет ли роста памяти на потоке.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 
