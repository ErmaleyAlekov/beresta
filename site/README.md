# Сайт продукта «Берёста»

Статический сайт на русском языке для реестра российского ПО: описание, скачивание, документация, лицензия и стоимость. Без сборки, без сторонних библиотек и шрифтов — только HTML и CSS.

| Файл | Страница |
|---|---|
| `index.html` | о продукте: возможности, стандарты, стек, правообладатель |
| `download.html` | релизы Gitverse и GitHub, исходный текст, пакеты, проверочный стенд |
| `documentation.html` | три документа для реестра (HTML и PDF) и ссылки на документацию репозитория |
| `license.html` | «Предоставляется на безвозмездной основе на условиях лицензии MIT», текст лицензии |
| `docs/*.html`, `docs/*.pdf` | собираются из `docs/registry/*.md` командой ниже — вручную не править |
| `style.css` | общие стили (светлая и тёмная темы) |

Пересборка документов после правки `docs/registry/*.md` или `include/capi/beresta_c.h`:

```bash
python tools/registry/build_docs.py
```

Нужен Python 3.8+ и Chrome, Edge или Chromium для печати PDF (`--browser путь`); `--html-only` собирает только HTML. Просмотр локально:

```bash
python -m http.server 8766 --directory site
```

Перед публикацией заменить заполнители `[ФИО правообладателя]`, `[адрес электронной почты]` и т. п. в `index.html`, `license.html` и `docs/registry/*.md`.

## Где публиковать

**Рекомендуется Gitverse Pages** (раздел «Страницы» в настройках репозитория): площадка российская, рядом с репозиторием, на который ссылается заявка в реестр. Адрес сайта: `https://ermaleyalekov.gitverse.site/beresta`. GitHub Pages подойдёт как зеркало, но доступность иностранной площадки из России и для экспертов не гарантирована.

Публикация на Gitverse — через workflow ([документация Gitverse Pages](https://gitverse.ru/docs/pages/docs/from-workflow)). Готовый файл — `site/gitverse-pages.yml.example`; он **не подключён**, пока пользователь не решит публиковать:

1. Скопировать `site/gitverse-pages.yml.example` в `.gitverse/workflows/pages.yml`, закоммитить, отправить в `main` на Gitverse.
2. Настройки репозитория → «Страницы» → включить, «Источник» — «Воркфлоу».
3. Запустить workflow «Pages» вручную (CI/CD) или следующим push в `main`.

GitHub Pages (если нужно зеркало): Settings → Pages → Source «GitHub Actions» и workflow с `actions/upload-pages-artifact` (`path: site`) и `actions/deploy-pages`.
