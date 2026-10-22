# Отладчик провалов XSTS

Инструмент для ручной отладки случаев W3C XML Schema Test Suite (XSTS), которые
«Берёста» сейчас не проходит. Прогонщик [`tests/xsts`](../xsts/README.md) прогоняет
весь набор (около 39 тысяч случаев) и строит отчёт `xsts_report.md`. Эта папка
берёт из отчёта только **провалы** (вердикт FAILED, 1 229 на 22.10.2026) и
позволяет выполнять их по одному в отладчике: с фильтром, по номеру,
с остановкой на первом.

Код прогонщика не дублируется. Каталог набора, загрузка схемы, проверка
экземпляра и вердикты берутся из `tests/xsts/src` (собираются все его файлы,
кроме `main.cpp`). Здесь лежат только список провалов, выполнение одного случая и
командная строка.

## Состав

```
tests/xsts_failing/
├── failing_cases.tsv         список провалов (создаёт программа, руками не править)
├── Makefile, CMakeLists.txt  сборка (по умолчанию отладочная: -g -O0)
├── include/failing/          заголовки
└── src/
    ├── main.cpp              запуск: прогон списка или его пересборка
    ├── cli/options.cpp       аргументы командной строки
    ├── list/                 чтение и запись списка, пересборка, сводка
    ├── runner/               выполнение одного случая (точка останова), прогон списка
    └── common/               своё исключение ListError
```

Набор берётся из `../xsts/xmlschema2006-11-06` (если его нет, выполните `make fetch`
в `tests/xsts`), список ограничений — из `../xsts/known_limitations.txt`.

## Сборка и запуск

Make (GCC / Clang / MinGW), из этой папки:

```bash
make
```

```bash
make run
```

```bash
make run ARGS="MS-Notations --stop-on-first"
```

`make` собирает отладочную программу `bin/debug/xsts_failing` и компилирует в неё
библиотеку **из исходников** с `-g -O0`, так что пошагово можно пройти и код самой
«Берёсты». Объекты лежат отдельно: `obj/debug/library`, `obj/debug/xsts`,
`obj/debug/failing`. Первая сборка занимает около минуты, `-j` её ускоряет.
`make BUILD=release` собирает оптимизированную программу в `bin/release/`.

CMake (MSVC, MinGW, GCC, Clang), из корня репозитория:

```bash
cmake -S tests/xsts_failing -B build-xsts-failing
```

```bash
cmake --build build-xsts-failing --config Debug
```

По умолчанию тип сборки — Debug. Цели `xsts_failing_run` и `xsts_failing_regen`
делают то же, что `make run` и `make regen`. Путь к этой папке зашит в программу
(макрос `XSTS_FAILING_HOME`), поэтому список и набор находятся, из какой бы папки
IDE ни запустила программу. В Visual Studio рабочая папка отладки уже задана
(`VS_DEBUGGER_WORKING_DIRECTORY`).

## Аргументы

```
xsts_failing [префикс] [--case N] [--stop-on-first]
             [--list файл.tsv] [--suite папка] [--limitations файл]
xsts_failing --regenerate [--list файл.tsv] [--suite папка] [--limitations файл]
```

| Аргумент | Что делает |
|---|---|
| `префикс` | выполнять только случаи, чей id начинается с префикса: `MS-Notations`, `MS-ComplexType2006-07-15/ctB`, `MGroup/particles00103m1` |
| `--case N` | выполнить только запись номер N (номер печатается в квадратных скобках, считается с 1) |
| `--stop-on-first` | остановиться на первом случае, который всё ещё проваливается |
| `--regenerate` | пересобрать список полным прогоном набора (см. ниже) |
| `--list`, `--suite`, `--limitations` | другие пути вместо путей по умолчанию |

Вывод по каждому случаю:

```
[  77] instanceTest  MS-Additional2006-07-15/addB034/addB034.v
       описание:  TEST :Adhoc XSD: : id="71818" description="xsd: when an attribute is prohibited, ...
       документы: msData/additional/test71818.xml
       схема:     msData/additional/test71818.xsd
       ожидалось: valid, получено: invalid — провал
       сообщение: ложная ошибка: /root: атрибут "attr" запрещён (use="prohibited")
```

В конце печатается «Прошло X из N». Случай, который уже проходит, помечается
`ИСПРАВЛЕН`. Код возврата: 0 — все выбранные случаи проходят, 1 — хотя бы один
проваливается, 2 — ошибка окружения (нет набора или списка, неверный аргумент,
случая из списка нет в наборе).

Пути в строке «документы» и «схема» заданы относительно `../xsts/xmlschema2006-11-06`.
Файлы можно сразу открыть в редакторе рядом с отладчиком.

## Список провалов `failing_cases.tsv`

Текст в UTF-8. Строки на `#` — шапка. Остальные строки — 8 полей через табуляцию
(файл открывается и в табличном редакторе):

| Поле | Значение |
|---|---|
| `id` | `набор/группа/случай` — ключ для поиска в наборе и для фильтра |
| `kind` | `schemaTest` (загрузить схему) или `instanceTest` (проверить экземпляр) |
| `expected` | ожидание набора: `valid` / `invalid` |
| `actual` | что сделала библиотека: `valid`, `invalid`, `нет схемы` (схема группы или схема по `xsi:schemaLocation` не загрузилась), `сбой` (чужое исключение) |
| `documents` | документы случая через `;` |
| `schema` | для `instanceTest` — документы схемы группы, иначе `-` |
| `message` | причина провала, как в `xsts_report.md` |
| `description` | первая `documentation` группы: что проверяет случай |

### Пересборка списка

```bash
make regen
```

Цель собирает оптимизированную программу и запускает `--regenerate`. Весь набор
прогоняется **тем же** `xsts::GroupRunner`, что и `make run` в `tests/xsts`,
с `known_limitations.txt` и правилами пропуска. Поэтому в список попадают ровно
провалы отчёта: случаи из списка ограничений и пропущенные случаи в него не входят.
Затем каждый провал выполняется отдельно, чтобы записать фактическое поведение.
Полный прогон занимает несколько минут. В конце печатается сводка по видам и наборам.

Пересобирайте список после исправлений в библиотеке и после правки
`known_limitations.txt`. Номера `--case` после пересборки сдвигаются, а префиксы
id остаются прежними. Перед пересборкой список стоит закоммитить: `git diff`
покажет, какие случаи ушли и какие добавились (регрессии).

## Как отлаживать один случай

1. Найдите случай: `make run ARGS="MS-Notations"` или поиск по `failing_cases.tsv`.
   Запомните номер или id.
2. Поставьте точку останова:
   - `failing::executeCase` в [src/runner/case_executor.cpp](src/runner/case_executor.cpp) —
     вход в выполнение случая; дальше шагайте внутрь;
   - для schemaTest — `beresta::xsd::Schema::fromDocuments`
     ([src/xsd/schema/schema.cpp](../../src/xsd/schema/schema.cpp)), сборка схемы
     идёт в `src/xsd/schema/schema_builder_*.cpp`;
   - для instanceTest — `beresta::xsd::SchemaValidator::validate`
     ([src/xsd/validation/schema_validator.cpp](../../src/xsd/validation/schema_validator.cpp)).
3. Запустите программу под отладчиком с `--case N`: выполнится ровно один случай.
   У instanceTest сначала загружается схема группы, потом проверяется экземпляр,
   поэтому точка в коде схемы может сработать и на этом шаге.

gdb из этой папки:

```bash
gdb --args bin/debug/xsts_failing --case 77
```

VS Code (расширение C/C++). Пример `.vscode/launch.json` с `cwd` на эту папку:

```json
{
    "name": "xsts_failing",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/tests/xsts_failing/bin/debug/xsts_failing.exe",
    "args": ["--case", "77"],
    "cwd": "${workspaceFolder}/tests/xsts_failing",
    "MIMode": "gdb"
}
```

На Linux и macOS у программы нет `.exe`, а на macOS вместо `gdb` работает `lldb`.
Visual Studio и CLion открывают `CMakeLists.txt` этой папки напрямую. Аргументы
задаются в настройках запуска цели `xsts_failing`.

Как проверить исправление: сначала `make run ARGS="<префикс группы>"`, где все
случаи группы должны получить пометку `ИСПРАВЛЕН`. Затем `make test` в корне
и `make run` в `tests/xsts`, чтобы не появилось новых провалов. После этого `make regen`.

## Сводка провалов (22.10.2026, 1 229)

Первый список (21.10.2026) насчитывал 1 463 провала. Ручная отладка по этой
папке закрыла 234 случая без новых провалов: большие `minOccurs`/`maxOccurs`,
`id` (NCName, уникальность в документе), место `xs:annotation` в глобальных
компонентах и частицах, место и состав `xs:group`, NCName в `name` глобальных
компонентов, границы `xs:all`. Ниже — то, что осталось.

### По видам: ожидание → факт

| Количество | Вид | Что это значит |
|---:|---|---|
| 1 198 | schemaTest invalid → valid | ошибочная схема **принята** |
| 3 | schemaTest valid → invalid | корректная схема отвергнута (`schZ012_a/b/c`) |
| 12 | instanceTest valid → invalid | ложная ошибка в валидном экземпляре |
| 11 | instanceTest invalid → valid | ошибка в экземпляре не найдена |
| 5 | instanceTest invalid → нет схемы | схема по `xsi:schemaLocation` не загрузилась |

97 % провалов — один вид: библиотека загружает схему, которую стандарт считает
ошибочной. Ни сообщения, ни исключения при этом нет. Отлаживать такие случаи
нужно в коде сборки схемы: ищите место, где проверка должна была бросить
`SCHEMA_ERROR`, но не бросила.

### По темам (провалы вида «ошибочная схема принята»)

Темы определены по ключевым словам описания (`description`). Темы пересекаются,
поэтому числа примерные.

| ≈ Случаев | Тема | Примеры | Где смотреть |
|---:|---|---|---|
| 440 | **Сочетания и значения фасетов**: `length` вместе с `minLength`/`maxLength`, `minLength` > `maxLength`, `maxInclusive` вместе с `maxExclusive`, пустое или недопустимое для базового типа значение фасета, фасет, неприменимый к list/union | `MS-DataTypes2006-07-15/string_length006_5`, `decimal_maxInclusive003_28`, `MS-SimpleType2006-07-15/stF009` | `schema_builder_facets.cpp`, `datatypes/facet_checks.cpp` |
| 330 | **Порядок и состав дочерних элементов XSD** (модель содержимого «схемы для схем»): `complexType` с `simpleContent` и следом `attribute`, два `sequence` подряд, `notation` внутри `element`, `selector` внутри `selector` | `MS-ComplexType2006-07-15/ctB011`, `ctG015`, `MS-Notations2006-07-15/notatF023`, `MS-IdentityConstraint2006-07-15/idD018` | правила для `xs:group` уже есть в `schema_builder_structure.cpp` (`checkGroupChild`) — туда же ложатся правила для остальных элементов |
| 130 | **Значения остальных атрибутов XSD**: `form="foo"`, `use="default"`, `processContents="all"`, список `namespace="##any ##other"`, неверный `default`/`fixed` для типа, `default` вместе с `use="required"` | `MS-Attribute2006-07-15/attA003`, `attF009`, `MS-Wildcards2006-07-15/wildC049`, `wildD079`, `ElemDecl/valueconstraint00401m2`, `AttrDecl/ad_valconstr00201m2` | разбор атрибутов в `schema_builder_attributes.cpp`, `schema_builder_wildcards.cpp`, `schema_builder_elements.cpp` |
| 70 | **Аннотации** там, где `checkChildOrder` ещё не вызывается: `anyAttribute`, `complexContent`/`simpleContent`, локальные типы, `annotation` внутри `annotation` | `MS-Annotations2006-07-15/annotB001`, `annotB007`, `annotB012` | вызвать `checkChildOrder` и для этих элементов |
| 35 | **Атрибут `name`** вне глобальных компонентов: `name` у локального `complexType`/`simpleType`, локальные объявления, повтор имени | `MS-ComplexType2006-07-15/ctA042`, `MS-Attribute2006-07-15/attC004`…`attC010` | `schema_builder_names.cpp`, `checkNCName` в `schema_builder_structure.cpp` |
| 30 | **`id`** в оставшихся местах (по описаниям; часть — другие ошибки той же группы) | `make run ARGS=MS-IdentityConstraint` | `checkDocumentIds` в `schema_builder_structure.cpp` |

### По наборам

| Набор | Провалов | Что не так (по первым примерам) |
|---|---:|---|
| MS-DataTypes2006-07-15 | 416 | фасеты: несовместимые сочетания и значения (`Facets/Schemas/*.xsd`) |
| MS-ComplexType2006-07-15 | 216 | порядок дочерних элементов `complexType`/`simpleContent`/`complexContent` (серии `ctB`, `ctD`, `ctE`, `ctG`, `ctH`) |
| MS-Attribute2006-07-15 | 102 | значения `form`/`use`/`default`/`fixed`, `name`, правила представления `attKa`…`attKc`; ложная ошибка `use="prohibited"` при `anyAttribute` (`attZ002.v`) |
| MS-IdentityConstraint2006-07-15 | 97 | `unique`/`key`/`keyref`: место, лишние `selector`/`field`, недопустимые XPath в `selector`/`field` (`idI`, `idJ`) |
| MS-Wildcards2006-07-15 | 73 | значения `namespace` и `processContents`; 3 ложные ошибки `xs:any` (`wildI005`…`wildI007`) |
| MS-SimpleType2006-07-15 | 71 | состав `simpleType`/`restriction`/`list`/`union`, фасеты у list/union; 4 ложные ошибки перечисления с пробелами (`stE050`, `stE055`, `stE065`, `stE066`) |
| MS-Notations2006-07-15 | 67 | `notation` без `name`/`public`, повтор, неверное место и содержимое (`notatF`) |
| MS-ModelGroups2006-07-15 | 41 | `all`/`sequence`/`choice`: недопустимое место и состав |
| MS-Additional2006-07-15 | 40 | 23 schemaTest; 9 instanceTest `addB149`…`addB167` (ошибка не найдена: порядок `schemaLocation` и уже встреченных компонентов); схемы по подсказкам не разбираются |
| MS-Annotations2006-07-15 | 19 | `annotation` дважды в элементах, которые ещё не проверяются, или внутри `annotation` |
| ElemDecl | 17 | `default`/`fixed` не по типу, `type` вместе с вложенным типом |
| MS-Schema2006-07-15 | 16 | состав `schema`; `schZ012_a/b/c` — ложное «повторное определение xs:element»; `schA2.i`, `schA5.i` — порядок схем по подсказкам |
| MS-Element2006-07-15 | 14 | атрибуты `element`; `QFE1700c2.v` — ложная ошибка `"" не соответствует типу int` |
| MS-AttributeGroup2006-07-15, MS-Particles2006-07-15 | 9 | `attributeGroup`: `ref` вместе с содержимым; частицы |
| IdConstrDefs, MGroup, MS-Group2006-07-15, Notation | 3–4 | аннотации, `notation`, `group` |
| SType, AttrDecl, Wildcard, MS-Errata102006-07-15, suntest | 1–2 | `default` вместе с `use="required"`; `suntest/xsd020.e` — ссылка на необъявленный простой тип |

## С чего начать

1. **Аннотации в остальных элементах: около 70 случаев.** `checkChildOrder`
   (`schema_builder_structure.cpp`) уже проверяет место `xs:annotation`, но
   вызывается только для глобальных компонентов и частиц. Его нужно вызвать и для
   `anyAttribute`, `attribute`, `complexContent`/`simpleContent`,
   `restriction`/`extension`, локальных типов, `selector`/`field`.
   Проверка: `make run ARGS=MS-Annotations`.
2. **Модель содержимого и значения атрибутов XSD: около 450 случаев.** Это главный
   источник провалов. Системное решение — таблица «элемент XSD → допустимые
   дочерние элементы в порядке и атрибуты с типами» (выжимка из схемы для схем)
   и одна проверка по ней. `checkGroupChild` — её первый, частный кусок.
   Так закрываются сразу `ctB`/`ctG`/`ctH`, `notatF`, `idD`/`idE`, `stB`,
   `form`/`use`/`processContents`. Начать удобно с `MS-Notations` (67 случаев,
   у `notation` простая модель) и `MS-ComplexType2006-07-15/ctB`.
3. **Фасеты: около 440 случаев, одна область кода.** Правила согласованности
   фасетов (XSD 1.0, часть 2, §4.3: `length` против `minLength`/`maxLength`,
   `min*` ≤ `max*`, значения фасетов допустимы для базового типа). Префикс:
   `MS-DataTypes2006-07-15`, примеры именуются `тип_фасетNNN`.
4. **Остальные 31 случай — точечные ошибки**, каждую лучше разбирать отдельно:
   `use="prohibited"` при `anyAttribute` (3), перечисления с пробелами (4),
   `xs:any` (3), `schZ012` (3), `addB149`…`addB167` (9), схемы по подсказкам (5).
