# Сайт продукта «Берёста»

Статический сайт на русском языке: описание, скачивание, документация, лицензия и стоимость. Без сборки, без сторонних библиотек и шрифтов — только HTML и CSS. Адрес: <https://ermaleyalekov.gitverse.site/beresta>.

| Файл | Страница |
|---|---|
| `index.html` | о продукте: возможности, стандарты, стек, правообладатель |
| `download.html` | релизы Gitverse и GitHub, исходный текст, пакеты, проверочный стенд |
| `documentation.html` | руководства по установке и эксплуатации, жизненный цикл (HTML и PDF), ссылки на документацию репозитория |
| `license.html` | «Предоставляется на безвозмездной основе на условиях лицензии MIT», текст лицензии |
| `docs/*.html`, `docs/*.pdf` | собираются из `docs/registry/*.md` командой ниже |
| `style.css` | общие стили (светлая и тёмная темы) |

Пересборка документов после правки `docs/registry/*.md` или `include/capi/beresta_c.h`:

```bash
python tools/registry/build_docs.py
```

Нужен Python 3.8+ и Chrome, Edge или Chromium для печати PDF (`--browser путь`); `--html-only` собирает только HTML. Просмотр локально:

```bash
python -m http.server 8766 --directory site
```

Сайт публикуется на Gitverse Pages workflow `.gitverse/workflows/pages.yml` при изменении `site/` в ветке `main`.
