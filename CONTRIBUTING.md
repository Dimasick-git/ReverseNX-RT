# Contributing — ReverseNX-RT (Ryazhenka)

**EN:** Downstream fork that tracks [masagrator/ReverseNX-RT](https://github.com/masagrator/ReverseNX-RT) daily via CI. Bugs in upstream plugin/overlay logic → file with masagrator. Bugs in our Ryazhenka-specific patches (libryazhahand swap, i18n, RYZH signature, CI workflows, RU docs) → file here.

---

## Что присылать сюда

- Опечатки/неточности в `lang/ru.json` (или другом языковом файле).
- Запросы на новые языки.
- Баги в `Overlay/source/rnxs_lang.{hpp,cpp}` (например, не подгружается JSON).
- Проблемы со сборкой `scripts/build.ps1` или CI workflows.
- Совместимость с другими репо Ряженки.
- Опечатки в README/CHANGELOG/docs/RU.

## Что присылать в upstream (masagrator)

- Баги в логике переключения mode/DDR.
- Баги в IPC контракте с SaltyNX плагином.
- Новые поддерживаемые игры в DDR-листе (отдельная ветка `compatible_games`).
- Всё что касается базовой логики оверлея.

Их issue tracker: <https://github.com/masagrator/ReverseNX-RT/issues>.

## Как добавить новый язык

1. Скопировать `lang/en.json` → `lang/<2-letter-code>.json`.
2. Перевести **значения**. Ключи и форматные плейсхолдеры не трогать
   (если есть `"DOCKED_DDR_PREFIX": "Docked DDR: "` — оставь
   trailing space, к нему printf приклеит `%dx%d`).
3. Открыть `Overlay/source/main.cpp::initServices`, добавить case в switch:
   ```cpp
   case SetLanguage_Spanish: rnxs::loadLanguage("es"); break;
   ```
4. Открыть PR. `verify_build.yml` подтвердит сборку.

## Как открыть PR

1. Форкнуть [Dimasick-git/ReverseNX-RT](https://github.com/Dimasick-git/ReverseNX-RT).
2. Создать ветку от `main` (не `master` — `master` зеркалит masagrator).
3. Закоммитить (стиль свободный, RU/EN на выбор).
4. PR в `main`. На PR прогонится `verify_build.yml`.
5. Если меняешь файлы из [`.github/sync-protected-paths.txt`](.github/sync-protected-paths.txt) — упомяни в описании PR.

## Стиль кода

C++: tabs (как у upstream — стиль masagrator'а), namespace `rnxs::`
для всех наших добавок (не загрязняем `tsl::`/`ult::`). Файлы с LF
(см. `.gitattributes`).

Документация: README/CHANGELOG/CONTRIBUTING — формат "1 абзац EN → `---` →
полно RU", как у всех репо Ряженки.
