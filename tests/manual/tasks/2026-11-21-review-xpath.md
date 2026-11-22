# Ревью кода: XPath, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-15. Ревью: XPath: лексер, разбор и дерево выражения

- **Зачем.** Выражение XPath приходит от пользователя или из схемы (`xs:assert`, identity). Лексер и парсер — первые, кто видит строку: ошибки — неверная токенизация (`*` как оператор или имя, `-` в именах) и рекурсия без предела.
- **Приоритет:** желательно до следующего выпуска · ~80–130 минут (файлов: 21, строк: ≈ 1980 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-15/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xpath/internal/xpath_lexer.hpp` (106)
  2. `include/xpath/internal/xpath_parser.hpp` (97)
  3. `include/xpath/internal/xpath_ast.hpp` (322)
  4. `src/xpath/lexer/xpath_lexer.cpp` (96)
  5. `src/xpath/lexer/xpath_lexer_names.cpp` (116)
  6. `src/xpath/lexer/xpath_lexer_operators.cpp` (36)
  7. `src/xpath/lexer/xpath_lexer_symbols.cpp` (92)
  8. `src/xpath/parser/xpath_parser.cpp` (117)
  9. `src/xpath/parser/xpath_parser_errors.cpp` (55)
  10. `src/xpath/parser/xpath_parser_functions.cpp` (82)
  11. `src/xpath/parser/xpath_parser_paths.cpp` (136)
  12. `src/xpath/parser/xpath_parser_primary.cpp` (105)
  13. `src/xpath/parser/xpath_parser_tokens.cpp` (70)
  14. `src/xpath/parser/xpath_parser_xpath2.cpp` (124)
  15. `src/xpath/ast/xpath_binary.cpp` (91)
  16. `src/xpath/ast/xpath_binary_chain.cpp` (78)
  17. `src/xpath/ast/xpath_complex_expressions.cpp` (62)
  18. `src/xpath/ast/xpath_conditional.cpp` (60)
  19. `src/xpath/ast/xpath_path.cpp` (44)
  20. `src/xpath/ast/xpath_simple_expressions.cpp` (45)
  21. `src/xpath/ast/xpath_typed_expressions.cpp` (46)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Неоднозначности: `*` (умножение / любое имя), `div`/`mod`/`and` как имена элементов, `a-b` (имя или вычитание), `..`, `.5`, `5.`.
  2. Глубина: `((((...))))` и `-----1` на 100 000 уровней — переполнение стека рекурсивного спуска? Есть ли предел.
  3. Строки: незакрытая кавычка, `''` внутри строки (XPath 2.0), очень длинный литерал.
  4. `xpath_parser_xpath2`: части XPath 2.0 (if/for/some/every, `instance of`, `cast as`) не включаются в режиме 1.0.
  5. AST: владение узлами дерева (unique_ptr), нет ли копирования огромных поддеревьев.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат.** По ревью пользователя исправлены три ошибки.
  1. Литералы в утверждениях XSD теперь декодируют удвоенную одинарную или двойную кавычку. Обычный XPath 1.0 по-прежнему отклоняет такую запись (`src/xpath/lexer/xpath_lexer.cpp:95`).
  2. Каждая дополнительная привязка `some` или `every` занимает уровень вложенности до окончания разбора условия. Предел 128 охватывает привязки в одном списке и во вложенных кванторах (`src/xpath/parser/xpath_parser_xpath2.cpp:110`).
  3. Конструктор внутреннего парсера дополняет ручной поток лексемой `END_OF_INPUT`. Пустой поток даёт ошибку разбора, одиночное число или имя разбирается до конца (`src/xpath/parser/xpath_parser.cpp:76`).
- **Что показало ревью.** Прежний тест загрузки схемы требовал отклонять литерал с удвоенной кавычкой. Тесты глубины проверяли скобки и цепочки операторов, а лексер штатно выдавал терминатор. Добавлены четыре теста в `tests/src/review/review_xpath_tests.cpp`; ожидание загрузки схемы обновлено в `tests/src/xsd/xsd_assert_schema_tests.cpp`. Модульные тесты прошли, 253 из 253. Снимок `tests/manual/review/T-15/` удалён.

## Т-16. Ревью: XPath: вычисление, оси, порядок документа, значения

- **Зачем.** Вычисление выражения над деревом: оси, шаги, сравнения, порядок документа, преобразование значений. Риски — квадратичное время (сортировка/удаление дублей на больших наборах), неверный порядок у атрибутов и namespace-узлов.
- **Приоритет:** желательно до следующего выпуска · ~70–110 минут (файлов: 21, строк: ≈ 1707 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-16/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xpath/xpath.hpp` (91)
  2. `include/xpath/xpath_value.hpp` (142)
  3. `include/xpath/internal/xpath_eval.hpp` (100)
  4. `include/xpath/internal/xpath_axes.hpp` (62)
  5. `src/xpath/eval/xpath_axes.cpp` (73)
  6. `src/xpath/eval/xpath_axes_attributes.cpp` (136)
  7. `src/xpath/eval/xpath_axes_horizontal.cpp` (87)
  8. `src/xpath/eval/xpath_axes_namespace.cpp` (56)
  9. `src/xpath/eval/xpath_axes_tree.cpp` (69)
  10. `src/xpath/eval/xpath_axes_vertical.cpp` (51)
  11. `src/xpath/eval/xpath_compare.cpp` (114)
  12. `src/xpath/eval/xpath_document_order.cpp` (122)
  13. `src/xpath/eval/xpath_document_order_check.cpp` (44)
  14. `src/xpath/eval/xpath_steps.cpp` (117)
  15. `src/xpath/value/xpath_numbers.cpp` (66)
  16. `src/xpath/value/xpath_strings.cpp` (97)
  17. `src/xpath/value/xpath_value.cpp` (57)
  18. `src/xpath/value/xpath_value_convert.cpp` (85)
  19. `src/xpath/value/xpath_value_typed.cpp` (43)
  20. `src/xpath/xpath_expression.cpp` (70)
  21. `src/xpath/xpath_shortcuts.cpp` (25)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `xpath_document_order*`: порядок атрибутов и namespace-узлов относительно детей; узлы из разных документов; сложность сортировки и удаления дублей.
  2. Оси: `preceding`/`following` на глубоком дереве — время; `namespace::` для унаследованных объявлений.
  3. `xpath_compare`: сравнение набора с набором (существование пары), NaN, `"" = 0`.
  4. `xpath_numbers` / `xpath_strings`: `string(-0)`, `string(1e21)`, `number(" 1 ")`, бесконечности, округление `round(-0.5)`.
  5. Изменение дерева между компиляцией и вычислением выражения — кэшированные указатели.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** src/xpath/eval/xpath_compare.cpp:95
В compareValues() при сравнении двух множеств (leftSet && rightSet) выполняется вложенный цикл по всем парам узлов с повторным вычислением stringValue(item) (обходом всего поддерева) на каждой итерации внутреннего цикла без кэширования строковых значений. На документе с тысячами вложенных элементов выражение вроде //* != //* (когда все узлы имеют одинаковый текст и ни одна пара не срабатывает досрочно) имеет сложность \(O(\vert{}L\vert{} \cdot \vert{}R\vert{} \cdot H)\) и вешает поток на десятки секунд.
src/xpath/eval/xpath_axes_horizontal.cpp:58
В precedingAxis() вызов appendSubtreeReversed() выделяет временный вектор на каждое поддерево предшествующих братьев.
src/xpath/xpath_expression.cpp:47
В XPathExpression::selectElements() обращение item.node->getType() выполняется без проверки item.node != nullptr. Если в контекст или переменную передать пустой XPathItem{} с node == nullptr (например, $empty), вызов selectElements приведёт к разыменованию нулевого указателя.

- **Разбор.** Сравнение наборов узлов учитывает существование подходящей пары, а selectElements пропускает пустые узлы. Для оси preceding результат каждого поддерева разворачивается на месте. Порядок узлов проверен позиционными предикатами. Обход following и preceding уже линеен по числу посещённых узлов и глубине. Встроенный префикс xml разрешается в Element::lookupNamespaceURI.

## Т-17. Ревью: XPath: функции и типизированные значения

- **Зачем.** Библиотека функций XPath 1.0 и подмножества 2.0, типизированные атомарные значения для `xs:assert`. Ошибки — неверное число аргументов, индексы строк в байтах вместо символов, переполнения при приведении типов.
- **Приоритет:** когда будет время · ~60–90 минут (файлов: 18, строк: ≈ 1416 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-17/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/xpath/internal/xpath_functions.hpp` (100)
  2. `include/xpath/internal/xpath_typed.hpp` (122)
  3. `src/xpath/functions/xpath_function_args.cpp` (54)
  4. `src/xpath/functions/xpath_function_table.cpp` (83)
  5. `src/xpath/functions/xpath_functions_logic.cpp` (32)
  6. `src/xpath/functions/xpath_functions_names.cpp` (98)
  7. `src/xpath/functions/xpath_functions_nodes.cpp` (75)
  8. `src/xpath/functions/xpath_functions_numbers.cpp` (73)
  9. `src/xpath/functions/xpath_functions_strings.cpp` (54)
  10. `src/xpath/functions/xpath_functions_strings_extra.cpp` (105)
  11. `src/xpath/functions/xpath_functions_typed.cpp` (83)
  12. `src/xpath/functions/xpath_text_codes.cpp` (37)
  13. `src/xpath/typed/xpath_typed_arithmetic.cpp` (82)
  14. `src/xpath/typed/xpath_typed_atoms.cpp` (99)
  15. `src/xpath/typed/xpath_typed_cast.cpp` (121)
  16. `src/xpath/typed/xpath_typed_compare.cpp` (62)
  17. `src/xpath/typed/xpath_typed_compare_atoms.cpp` (109)
  18. `src/xpath/typed/xpath_untyped_source.cpp` (27)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Строковые функции: `substring` с NaN/бесконечностями/дробями, индексы по символам Unicode (а не байтам UTF-8), `translate` с повторами.
  2. `xpath_function_args` / `_table`: проверка числа и типов аргументов до доступа к ним.
  3. Числа: `floor`/`ceiling`/`round` для `-0`, NaN; `sum` пустого набора; деление на 0 в decimal и double.
  4. `xpath_typed_cast`: приведения между `xs:integer`/`decimal`/`double`/`string`/даты — переполнение, потеря точности, неверная ошибка.
  5. `xpath_typed_compare*`: сравнение несравнимых типов (дата и число) — ошибка типа, а не `false`.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** src/xpath/functions/xpath_functions_numbers.cpp:64
В functionRound() формула std::floor(value + 0.5) нарушает спецификацию XPath 1.0 на числе 0.49999999999999994 (наибольший double меньше 0.5), так как 0.49999999999999994 + 0.5 округляется до 1.0 и возвращает 1 вместо 0, а для чисел из интервала (-0.5, -0.0] (например, round(-0.2) или round(-0)) std::floor возвращает +0.0 вместо отрицательного нуля -0. Воспроизводится выражениями round(0.49999999999999994) и round(-0.2).
src/xpath/typed/xpath_typed_compare_atoms.cpp:80
В compareTypedAtoms() проверка left.getType() == ValueType::BOOLEAN || right.getType() == ValueType::BOOLEAN срабатывает раньше проверки совместимости типов и вызывает compareBooleanAtoms(), который через castAtomic(AtomicType::BOOLEAN, ...) молча приводит числа и строки к boolean. Из-за этого в режиме XSD_ASSERTION выражения с несовместимыми типами вроде true() eq 1 или true() = '1' возвращают true вместо ошибки типа XPTY0004 по спецификации XPath 2.0

- **Разбор.** round сохраняет знак отрицательного нуля и корректно округляет представимое число непосредственно ниже 0.5. Сравнения типизированных логических значений проверяют совместимость типов, включая приведение нетипизированного атрибута. Для substring с началом −∞ и длиной +∞ верхняя граница равна NaN, поэтому пустая строка соответствует XPath 1.0.
