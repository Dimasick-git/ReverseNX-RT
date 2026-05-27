# Changelog — ReverseNX-RT (Ryazhenka)

Заметные изменения форка относительно [masagrator/ReverseNX-RT](https://github.com/masagrator/ReverseNX-RT) (и базы [ppkantorski/ReverseNX-RT](https://github.com/ppkantorski/ReverseNX-RT)) документируются здесь.
Формат — [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/).

## [2.2.1+ryazha] — 2026-05-27

Первый релиз форка под брендом Ряженка. База — ppkantorski v2.2.1+
(коммит `7fdefd3`, "Comment out USING_WIDGET_DIRECTIVE in Makefile"),
которая в свою очередь синхронизирована с masagrator v2.2.1.

### Добавлено

- **`libryazhahand` вместо `libultrahand`** как submodule
  (`Overlay/libs/libryazhahand/`). Source-совместимая замена.
- **Подпись `.ovl` файла `ULTR` → `RYZH`** в `Overlay/Makefile`. Последние
  4 байта overlay'я теперь = `0x52 0x59 0x5A 0x48` (Ryazhahand) вместо
  `0x55 0x4C 0x54 0x52` (Ultrahand). `nx-ovlloader` подпись не
  валидирует (подтверждено инспекцией `source/main.c`), поэтому
  загрузка не ломается; overlay-managers Ряженки (Ryazhahand-Overlay)
  теперь узнают этот файл как "свой".
- **Локализация EN/RU.** Новый модуль `Overlay/source/rnxs_lang.{hpp,cpp}`
  грузит `/config/ReverseNX-RT/lang/<code>.json` при старте оверлея.
  30 UI-ключей: заголовок, подзаголовки разрешений, пункты меню,
  статусные сообщения, режим, DDR. Английские дефолты скомпилированы
  в бинарник (fallback при отсутствии JSON).
- **`lang/en.json` и `lang/ru.json`** — английский (= upstream defaults)
  + русский. Распаковываются в `/config/ReverseNX-RT/lang/`.
- **`Overlay/source/main.cpp::initServices`** теперь резолвит
  `setGetSystemLanguage` и вызывает `rnxs::loadLanguage("ru"|"en")`
  ДО запуска основной sm-сессии (отдельной `doWithSmSession`).
- **CI/CD набор**:
  - `build.yml` — devkitPro Docker сборка на push в `main` + PR;
  - `release.yml` — авто-релиз на push тега / изменение `Overlay/Makefile`;
  - `sync_upstream.yml` — **ежедневный sync с `masagrator/master`**
    (НЕ с ppkantorski!), защищая `.github/sync-protected-paths.txt`;
  - `verify_build.yml` — PR smoke-тест + проверка RYZH сигнатуры.
- **`scripts/build.ps1`** — локальная Docker-сборка под Windows.
- **`docs/RU/{i18n,build}.md`** — расширенная русская документация.
- **`.gitattributes`** — `* text=auto eol=lf` для воспроизводимых
  сборок между Windows-разработкой и Linux CI.

### Изменено

- `Overlay/Makefile`:
  - `APP_TITLE := "ReverseNX-RT (Ryazhenka)"`, добавлен `APP_AUTHOR`.
  - `APP_VERSION := "2.2.1+ryazha"`.
  - `include ... libultrahand/ultrahand.mk` → `libryazhahand/ryazhahand.mk`.
  - `LIBS := -lpng16 -lcurl -lz -lminizip -lmbedtls -lmbedx509 -lmbedcrypto -lnx`
    (было `-lnx`, libryazhahand тащит PNG/curl/zip).
- `Overlay/source/main.cpp`: все user-visible string literals заменены на
  ссылки в `rnxs::` namespace. Поведение функционально идентично
  upstream (только источник строк теперь runtime-loadable).
- Все упоминания "Ultrahand" в комментариях/документации → "Ryazhahand".

### Не изменилось (наследуется из upstream)

- TID, путь установки `/switch/.overlays/ReverseNX-RT-ovl.ovl`.
- Config path `/config/ReverseNX-RT/` (uppercase, project-specific —
  не /config/ultrahand/, поэтому переименовывать не за что).
- Зависимость от SaltyNX, IPC контракт с плагином в игре.
- Save format `/SaltySD/plugins/ReverseNX-RT/<TID>.dat`.
- Все upstream-фичи: real-time mode toggle, system control flag,
  DDR override, settings save per-game.

### Базовая совместимость

- SaltyNX 1.4.3+.
- Atmosphère последней версии.
- Совместимо с [Dimasick-git/Atmosphere](https://github.com/Dimasick-git/Atmosphere).

[2.2.1+ryazha]: https://github.com/Dimasick-git/ReverseNX-RT/releases/tag/v2.2.1-ryazha
