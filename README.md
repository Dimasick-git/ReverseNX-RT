# ReverseNX-RT

Оверлей для переключения режима игры между портативным и доковым и изменения разрешения. Версия **2.2.3**: новая libryazhahand, компактные элементы и перенос строк состояния.

Скопируйте содержимое `ReverseNX-RT.zip` в корень SD-карты. Нужны Ryazhahand-Overlay и SaltyNX; для HOS 23 требуется SaltyNX 2.0.0 или новее.

Переводы находятся в `/config/ReverseNX-RT/lang/`, настройки игр — в `/SaltySD/plugins/ReverseNX-RT/<TID>.dat`. Формат настроек и протокол связи с SaltyNX сохранены.

Сборка: devkitA64, актуальный libnx и portlibs. Выполните `git submodule update --init --recursive`, затем `make`. GitHub Actions собирает закреплённый libnx с API HOS 23 и проверяет формат и подпись оверлея. Релизы публикуются только для тегов `v*`.

Оригинал: [masagrator/ReverseNX-RT](https://github.com/masagrator/ReverseNX-RT). Форк основан на версии ppkantorski с библиотекой [libryazhahand](https://github.com/Dimasick-git/libryazhahand); поддержка — Dimasick-git. Лицензия находится в `LICENSE`.
