<p align="left">
<a href="https://github.com/Dimasick-git/ReverseNX-RT/blob/main/LICENSE"><img alt="License" src="https://img.shields.io/badge/license-GPLv2-blue.svg"></a>
<a href="https://github.com/Dimasick-git/ReverseNX-RT/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/Dimasick-git/ReverseNX-RT?include_prereleases"></a>
<a href="https://github.com/Dimasick-git/ReverseNX-RT/actions/workflows/build.yml"><img alt="Build" src="https://github.com/Dimasick-git/ReverseNX-RT/actions/workflows/build.yml/badge.svg"></a>
<a href="https://github.com/Dimasick-git/ReverseNX-RT/actions/workflows/sync_upstream.yml"><img alt="Upstream sync" src="https://github.com/Dimasick-git/ReverseNX-RT/actions/workflows/sync_upstream.yml/badge.svg"></a>
</p>

# ReverseNX-RT (Ryazhenka edition)

**EN:** Ryazhenka fork of [masagrator/ReverseNX-RT](https://github.com/masagrator/ReverseNX-RT) — the SaltyNX-based Tesla overlay that toggles between Switch handheld and docked mode in real time and overrides default display resolution per-game. Initial code base is [ppkantorski/ReverseNX-RT](https://github.com/ppkantorski/ReverseNX-RT) (his libultrahand rebuild), but daily upstream sync pulls from **masagrator** (the original author) so we follow the canonical project, not ppkantorski's branch. Four substantive changes vs. ppkantorski: (1) `libultrahand` submodule swapped for [`Dimasick-git/libryazhahand`](https://github.com/Dimasick-git/libryazhahand); (2) overlay magic `ULTR` → `RYZH` for Ryazha-stack recognition; (3) full Russian/English UI localisation via `lang/{en,ru}.json`; (4) all `Ultrahand` mentions in code/Makefile/docs renamed to `Ryazhahand`.

---

## Что это

Это форк [masagrator/ReverseNX-RT](https://github.com/masagrator/ReverseNX-RT) — Tesla-оверлей для Switch, который через SaltyNX плагин:

- переключает игру между портативным и доковым режимом в реальном времени (`Change mode`);
- управляет источником решения о режиме — система vs. оверлей (`Change system control`);
- меняет Default Display Resolution игры по умолчанию (для совместимых тайтлов);
- сохраняет настройки на игру в `/SaltySD/plugins/ReverseNX-RT/<TID>.dat`.

В Ryazhenka-стеке используется как штатный режимный оверлей рядом с
[RCU](https://github.com/Dimasick-git/RCU) (clock control), [ovlSysmodules](https://github.com/Dimasick-git/ovlSysmodules) (sysmodule toggle) и
[Ryazhahand-Overlay](https://github.com/Dimasick-git/Ryazhahand-Overlay) (общий Tesla launcher).

## Структура форков (важно понять что откуда)

```
masagrator/ReverseNX-RT       ←  canonical upstream (оригинальный автор)
        ↓
        ↓  (5 коммитов libultrahand-патчей)
        ↓
ppkantorski/ReverseNX-RT      ←  initial code base (откуда мы клонировали)
        ↓
        ↓  (libryazhahand + RYZH + i18n + branding)
        ↓
Dimasick-git/ReverseNX-RT     ←  этот форк (Ryazhenka edition)
        ↑
        |   daily 03:00 UTC sync через `sync_upstream.yml`
        |
masagrator/ReverseNX-RT       ←  pulls from HERE, NOT ppkantorski
```

Мы взяли ppkantorski'siglio libultrahand-патчи как стартовую базу (чтобы
не переписывать его работу заново), но дальше отслеживаем оригинал
masagrator. Если masagrator выпустит баг-фикс — он автоматически придёт
к нам через PR от sync-бота, не затрагивая наши Ryazha-патчи (защищены
через [`.github/sync-protected-paths.txt`](.github/sync-protected-paths.txt)).

## Главные отличия от ppkantorski/masagrator

| Что | Где живёт | Зачем |
|------|-----------|-------|
| **Submodule `libultrahand` → `libryazhahand`** | `.gitmodules`, `Overlay/Makefile:52` | Канонический submodule всей Ряженки. Source-совместим, `ryazhahand.mk` ≡ `ultrahand.mk`. |
| **Подпись `ULTR` → `RYZH`** | `Overlay/Makefile`, `printf 'RYZH' >> $@` | Последние 4 байта `.ovl` — overlay magic. Ryazhahand-Overlay узнаёт наши `.ovl` как Ryazha-native. nx-ovlloader сам сигнатуру не валидирует, загрузка не ломается. |
| **Полная локализация EN/RU** | `Overlay/source/rnxs_lang.{hpp,cpp}` + `lang/{en,ru}.json` | Все 30 UI-строк (заголовок, подзаголовки разрешений, пункты меню, статусные сообщения, режим, DDR) переводятся при старте оверлея. |
| **Все `Ultrahand` → `Ryazhahand`** | Comments в `Overlay/Makefile`, документация, magic, наш namespace `rnxs::` | Чтобы форк не маскировался под upstream Ultrahand-овский стек. |
| **Брендинг APP_TITLE** | `Overlay/Makefile:40` | `ReverseNX-RT (Ryazhenka)`, версия `2.2.1+ryazha`. |
| **Защищённый sync с masagrator** | `.github/workflows/sync_upstream.yml` | Ежедневно 03:00 UTC бот тянет `masagrator/master`, открывает PR. Наши файлы из `.github/sync-protected-paths.txt` остаются нашими. |

## Установка

1. Скачать свежий релиз: [Releases →](https://github.com/Dimasick-git/ReverseNX-RT/releases/latest), файл `ReverseNX-RT-*.zip`.
2. Распаковать в корень SD-карты с перезаписью. Содержимое лягет в:
   - `/switch/.overlays/ReverseNX-RT-ovl.ovl`
   - `/config/ReverseNX-RT/lang/{en,ru}.json`
3. Дополнительно нужно установить **SaltyNX** (плагин-инжектор для игр) — без него оверлей покажет `SaltyNX не работает!`.
4. Перезагрузка → открыть Tesla оверлей через `L+DPAD_DOWN+RStick` → выбрать `ReverseNX-RT`.

**Требования**: SaltyNX 1.4.3+, Atmosphère последней версии, [nx-ovlloader (Ryazhenka)](https://github.com/Dimasick-git/nx-ovlloader) или совместимый.

## Использование

Открой оверлей **с игрой запущенной на переднем плане** (overlay-инжектор работает в контексте игрового процесса). Видишь:

| Состояние | Что покажет |
|-----------|-------------|
| Игра не запущена | «Игра не запущена! Оверлей отключён.» |
| Игра запущена, плагин не инжектнут | «Игра запущена. ReverseNX-RT не запущен!» (плагин не сработал — игра 32-бит или в исключениях SaltyNX) |
| Плагин запущен, но игра не запрашивала режим | «Игра не запрашивала режим!» (выйди из оверлея и подожди пока игра загрузится) |
| Всё работает | Видишь статус системы / режима / DDR + 5 пунктов меню |

Пункты меню (только при работающем плагине + проверенном режиме):

- **Управление системой** — переключает кто решает режим: система vs оверлей.
- **Сменить режим** (только если управление НЕ системное) — Docked ↔ Handheld псевдо-переключение.
- **Разрешение портативное / док** (только если игра использует Default Display Resolution) — выбор из 480p, 540p, 630p, 720p, 810p, 900p, 1080p.
- **Сохранить настройки** — сохраняет в `/SaltySD/plugins/ReverseNX-RT/<TID>.dat`, применяется при следующем запуске игры.

Если язык консоли = Русский, UI на русском; иначе английский upstream-defaults.

## Список совместимых игр (DDR)

Поддерживается [masagrator'ом в отдельной ветке](https://github.com/masagrator/ReverseNX-RT/blob/compatible_games/README.md) — мы не дублируем, синхронизация это не трогает.

## Локализация

Добавить новый язык — см. [`docs/RU/i18n.md`](docs/RU/i18n.md).

## Сборка

### Через Docker (Windows-friendly)

```powershell
.\scripts\build.ps1 -Dist
```

Скрипт сам подтянет `devkitpro/devkita64`, прокинет директорию и выдаст
`dist/ReverseNX-RT-*.zip` с SHA-256.

### Нативно

```bash
git clone --recursive https://github.com/Dimasick-git/ReverseNX-RT.git
cd ReverseNX-RT
make
```

Подробнее — [`docs/RU/build.md`](docs/RU/build.md).

## Лицензия

GPL-2.0, наследуется от masagrator/ReverseNX-RT.

## Кредиты

- **masagrator** — оригинальный ReverseNX-RT, SaltyNX, постоянная поддержка.
- **ppkantorski** — libultrahand-rebuild, рабочая база для нашего форка.
- **WerWolv** — Tesla overlay framework.
- **devkitPro** — toolchain.
- **Dimasick-git (Ryazhenka)** — libryazhahand swap, RYZH сигнатура, EN/RU локализация, CI/CD пайплайн.

## Полезные ссылки

- [masagrator/SaltyNX](https://github.com/masagrator/SaltyNX) (инжектор плагина)
- [ppkantorski/nx-ovlloader](https://github.com/Dimasick-git/nx-ovlloader) (наш форк loader'а)
- [WerWolv/Tesla-Menu](https://github.com/WerWolv/Tesla-Menu)
