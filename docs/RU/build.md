# Сборка ReverseNX-RT (Ryazhenka)

## Метод 1. Docker на Windows

```powershell
git clone --recursive https://github.com/Dimasick-git/ReverseNX-RT.git
cd ReverseNX-RT
.\scripts\build.ps1 -Dist
```

- `-Dist` — собрать `.ovl` + упаковать в SD-layout zip.
- `-Clean` — `make clean` перед сборкой.

Скрипт сам подтянет `devkitpro/devkita64` и установит нужные пакеты
(`switch-libpng`, `switch-curl`, `switch-zziplib`, `switch-mbedtls`,
`switch-libjpeg-turbo`).

## Метод 2. Нативно

```bash
# Linux/WSL
sudo dkp-pacman -S switch-dev switch-libpng switch-curl switch-zziplib switch-mbedtls switch-libjpeg-turbo
git clone --recursive https://github.com/Dimasick-git/ReverseNX-RT.git
cd ReverseNX-RT
export DEVKITPRO=/opt/devkitpro
make
```

Результат: `Overlay/ReverseNX-RT-ovl.ovl` + `Out/switch/.overlays/ReverseNX-RT-ovl.ovl`
(root Makefile копирует туда).

## Метод 3. CI (GitHub Actions)

| Триггер | Workflow | Что делает |
|---------|----------|------------|
| Push в `main` или PR | `build.yml` | Собирает `.ovl` + zip с SD-layout, заливает артефакт |
| Push тега `v*` | `release.yml` | Собирает + публикует GitHub Release с SHA-256 |
| Изменение `Overlay/Makefile` в `main` | `release.yml` | Авто-тэг из `APP_VERSION` |
| Ежедневно 03:00 UTC | `sync_upstream.yml` | `git fetch upstream master` (где **upstream = masagrator**), открывает PR |
| PR | `verify_build.yml` | Smoke-build + проверка RYZH сигнатуры |

## Структура релизного zip

```
ReverseNX-RT-2.2.1+ryazha-<hash>.zip
├── switch/
│   └── .overlays/
│       └── ReverseNX-RT-ovl.ovl       ← собственно оверлей (с подписью RYZH)
└── config/
    └── ReverseNX-RT/
        └── lang/
            ├── en.json
            └── ru.json
```

Распаковка в корень SD-карты — установка готова. **Не забудь поставить
SaltyNX** отдельно — без него оверлей покажет «SaltyNX не работает!».

## Проверка подписи

```bash
tail -c 4 Overlay/ReverseNX-RT-ovl.ovl | xxd
# 00000000: 5259 5a48                                RYZH
```

Если видишь `5554 4c52` (ULTR) — собралось с upstream Makefile;
проверь что `printf 'RYZH' >> $@` в правиле `$(OUTPUT).ovl`.

## Проблемы

- **`tesla.hpp: No such file or directory`** — submodule не подцеплен:
  `git submodule update --init --recursive`.
- **`undefined reference to png_*`** — нет `-lpng16` в `LIBS` или не
  установлен `switch-libpng`.
- **`undefined reference to ult::*`** — пересобери libryazhahand через
  его `ryazhahand.mk` (Makefile делает автоматически).
