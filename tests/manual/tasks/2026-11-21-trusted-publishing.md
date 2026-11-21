# Публикация в npm и NuGet без ключей (доверенная публикация, OIDC), 2026-11-21

Срок ключей `NPM_TOKEN` и `NUGET_API_KEY` истёк. Вместо новых ключей `release.yml` переведён на **доверенную
публикацию** (trusted publishing): GitHub выдаёт задаче `packages` короткоживущий подписанный токен OIDC
(`permissions: id-token: write` уже был — для provenance), реестр проверяет, что токен выдан файлу
`.github/workflows/release.yml` репозитория `ErmaleyAlekov/beresta`, и меняет его на одноразовый ключ.
Украсть нечего: долгоживущих ключей в секретах больше нет, а из форка или другого workflow опубликовать нельзя.

Что изменено в коде (не закоммичено):

- `.github/workflows/release.yml`, шаг **npm publish** — без `NODE_AUTH_TOKEN`; перед публикацией ставится npm 11.5.1+
  (в Node 22 — npm 10, он не умеет OIDC); provenance подписывается автоматически. Условие запуска — тег, github.com и
  репозиторий `ErmaleyAlekov/beresta` (в форках шаг не запускается).
- Шаг **NuGet** — сначала `NuGet/login@v1` (меняет токен OIDC на ключ, живущий 1 час), затем тот же
  `dotnet nuget push` с этим ключом. Условие — секрет `NUGET_USER` (имя профиля nuget.org, **не ключ**).
- `docs/building.md`, `docs/building.en.md` — раздел «Выпуск версии»: таблица секретов и настройка реестров.
- PyPI и Maven Central не трогал: PyPI тоже умеет доверенную публикацию (скажите — переведу так же), у Maven Central
  OIDC нет, там остаётся токен пользователя Central Portal.

## Т-1. Настроить доверенную публикацию на npmjs.com и nuget.org

- **Зачем.** Без настройки на стороне реестров следующий выпуск не опубликует пакеты `beresta`, `beresta-wasm`
  (npm) и `Beresta` (NuGet): npm ответит 404/403, а шаг NuGet пропустится без секрета `NUGET_USER`. Сделать это может
  только владелец пакетов — вход в реестры и подтверждение 2FA.
- **Приоритет:** блокирует выпуск · ~15 минут.
- **Подготовка.** Вход на <https://www.npmjs.com> и <https://www.nuget.org> под учётной записью — владельцем пакетов;
  доступ к настройкам репозитория GitHub `ErmaleyAlekov/beresta`.
- **Шаги.**
  1. **npm, пакет `beresta`:** npmjs.com → пакет `beresta` → **Settings** → раздел **Trusted Publisher** →
     **GitHub Actions**. Заполнить: Organization or user — `ErmaleyAlekov`; Repository — `beresta`;
     Workflow filename — `release.yml` (только имя файла, без `.github/workflows/`); Environment name — пусто.
     Сохранить (npm попросит 2FA).
  2. То же для пакета **`beresta-wasm`**.
  3. (Рекомендуется, после первого успешного выпуска.) В тех же Settings → **Publishing access** выбрать
     «Require two-factor authentication and disallow tokens» — тогда пакеты нельзя опубликовать никаким ключом,
     только этим workflow или вручную с 2FA.
  4. **NuGet:** nuget.org → меню профиля (имя пользователя справа вверху) → **Trusted Publishing** → **Add policy**
     (или «Create»). Заполнить: Policy name — `beresta-release`; Package owner — ваш профиль (владелец пакета
     `Beresta`); Repository owner — `ErmaleyAlekov`; Repository — `beresta`; Workflow file — `release.yml`;
     Environment — пусто. Сохранить.
  5. **GitHub:** репозиторий → Settings → Secrets and variables → Actions → **New repository secret**:
     имя `NUGET_USER`, значение — **имя профиля nuget.org** (то, что в адресе `nuget.org/profiles/<имя>`, не e-mail
     и не ключ).
  6. Там же **удалить** старые секреты `NPM_TOKEN` и `NUGET_API_KEY` (workflow их больше не читает).
  7. Отозвать сами истёкшие ключи, если они ещё числятся: npmjs.com → Access Tokens; nuget.org → API Keys.
- **На что смотреть.**
  - Шаги 1–2: на странице пакета в Trusted Publisher появилась запись «GitHub Actions · ErmaleyAlekov/beresta ·
    release.yml».
  - Шаг 4: политика в списке активна (не «Pending»/«Inactive»). Для публичного репозитория она активна сразу.
  - Проверка по-настоящему — только выпуском: тег `vX.Y.Z` → задача `packages`. В логе шага **npm publish** —
    `Signed provenance statement ... published` без ошибок `ENEEDAUTH`/`E404`; в шаге **NuGet login (OIDC)** — успех,
    в шаге **NuGet** — `Your package was pushed`. Если упало из-за настройки — исправить настройку и перезапустить
    задачу `packages` (уже опубликованное каждый шаг пропускает сам).
- **Что прислать.** «Шаги 1–7 сделаны» (или на каком шаге что не так — текст ошибки/скриншот формы); имя профиля
  NuGet присылать не нужно, оно только в секрете. После этого я закоммичу изменения `release.yml` и документации,
  а при следующем выпуске проверю логи задачи `packages`.
- **Результат:** готово (2026-11-21, скриншоты): секрет `NUGET_USER` добавлен, `NPM_TOKEN` и `NUGET_API_KEY` удалены;
  политика NuGet `beresta-release` — Active (ErmaleyAlekov/beresta, `release.yml`); у `beresta` и `beresta-wasm` — доверенный
  издатель ErmaleyAlekov/beresta, `release.yml`, право «npm stage publish», статус «Pending validation» (до 23.11.2026
  12:04 UTC); «Publishing access» — 2FA и запрет обходящих её ключей уже включены.
- **Разбор (2026-11-21).** NuGet настроен полностью. По npm два замечания (по документации docs.npmjs.com — trusted-publishers,
  staged-publishing):
  1. Издатели npm, созданные после 03.09.2026, по умолчанию имеют только право **`npm stage publish`** (отложенная
     публикация: CI кладёт версию «на проверку», в реестре она появляется после одобрения владельцем с 2FA на npmjs.com,
     вкладка «Staged Packages», или `npm stage approve <id>`). Шаг workflow вызывает `npm publish` — с таким правом он
     будет отклонён. Нужно выбрать: либо workflow переходит на `npm stage publish` (npm 11.15.0+) и каждый выпуск вы
     одобряете вручную, либо у издателя включается ещё и право `npm publish` (полностью автоматически, как раньше).
  2. Новая настройка должна пройти первую успешную публикацию **за 2 дня**, иначе она истекает: её нельзя ни
     использовать, ни править — только удалить и создать заново. Выпуска до 23.11 не планируется, поэтому перед следующим
     выпуском (в тот же день) издателей npm нужно пересоздать; заодно в этот момент выбирается право из пункта 1.
     Политики NuGet это не касается.

  `repository.url` в обоих `package.json` совпадает с репозиторием — это npm тоже проверяет.

  **Решение пользователя (2026-11-21): публикация автоматическая.** `release.yml` не меняется (`npm publish`).
  **Перед следующим выпуском** (в тот же день, до пуша тега): у `beresta` и `beresta-wasm` удалить истёкшего издателя,
  «Add trusted publisher» → GitHub Actions (ErmaleyAlekov / beresta / `release.yml`), в «Allowed actions» отметить
  **`npm publish`**; первая публикация должна пройти в течение 2 дней.

  **Дополнение (2026-11-21):** выпуск решено делать сегодня, поэтому издатели не пересоздавались, а были отредактированы:
  у `beresta` и `beresta-wasm` права **`npm publish` + `npm stage publish`** (скриншоты), статус «Pending validation» —
  выпуск должен пройти до 23.11.2026 12:04 UTC. Задача закрыта; проверка — лог шага npm publish при выпуске.
