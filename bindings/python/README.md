# Берёста для Python

Обёртка над Си-интерфейсом «Берёсты» через стандартный модуль `ctypes`. Зависимостей нет.

```python
import beresta

with beresta.parse("<shop><item code='A1'>Чай</item></shop>") as document:
    item = document.root.first_child
    print(item.get_attribute("code"), item.text)
```

Перед использованием соберите нативную библиотеку: `make PYTHON=1` или
`cmake -DBERESTA_BUILD_PYTHON=ON`. Бинарник окажется в `beresta/_native/<os>-<arch>/`.
Подробности — в README.md в корне репозитория.
