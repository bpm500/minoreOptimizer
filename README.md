<div align="center">

# minoreOptimizer

**Профессиональный инструмент оптимизации Windows 10 / 11**

[![Windows](https://img.shields.io/badge/Windows-10%20%2F%2011-0078D4?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/bpm500/minoreOptimizer)
[![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?style=for-the-badge&logo=python&logoColor=white)](https://python.org)
[![PyQt6](https://img.shields.io/badge/PyQt6-GUI-41CD52?style=for-the-badge&logo=qt&logoColor=white)](https://pypi.org/project/PyQt6/)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)
[![Version](https://img.shields.io/badge/Version-2.0-blue?style=for-the-badge)](https://github.com/bpm500/minoreOptimizer/releases)

<br/>

> Один инструмент — полная оптимизация системы.  
> Больше FPS, меньше задержек, чище система, никаких лишних телодвижений.

<br/>

![Screenshot](https://cdn.discordapp.com/attachments/1132699523978899477/1479205862626295818/icon.png?ex=69ab314f&is=69a9dfcf&hm=760c97042dd242eef7275ef1bd3e7782a854c0f2dc4be51fefdc3f10745567a6&)

</div>

---

## ⚡ Что такое minoreOptimizer?

**minoreOptimizer** — это десктопное приложение для Windows с тёмным интерфейсом, которое автоматизирует десятки оптимизаций системы одним нажатием кнопки. Инструмент ориентирован на геймеров, стримеров и всех, кто хочет выжать максимум из своего железа без ручного копания в реестре и командной строке.

**Что делает программа:**
- Устраняет задержки ввода (input lag) и снижает DPC latency
- Отключает телеметрию, слежку и фоновые процессы Microsoft
- Применяет сетевые и CPU твики для онлайн-игр
- Очищает мусор, кэш и неиспользуемые службы
- Предоставляет мгновенные утилиты без перезагрузки
- Позволяет визуально кастомизировать интерфейс Windows

---

## 🖥️ Системные требования

| Параметр | Требование |
|---|---|
| ОС | Windows 10 (1903+) / Windows 11 |
| Права | **Администратор** (обязательно) |
| Python | 3.10+ (для запуска из исходников) |
| Зависимости | PyQt6 |
| Архитектура | x64 |

---

## 🚀 Установка и запуск

### Вариант 1 — готовый EXE (рекомендуется)

1. Скачайте `minoreOptimizer.exe` из раздела [Releases](https://github.com/bpm500/minoreOptimizer/releases)
2. Запустите **от имени администратора** (ПКМ → «Запуск от имени администратора»)

### Вариант 2 — из исходного кода

```bash
# Клонируйте репозиторий
git clone https://github.com/bpm500/minoreOptimizer.git
cd minoreOptimizer

# Установите зависимости
pip install PyQt6

# Запустите (от администратора)
python optimizer.py
```

### Вариант 3 — сборка собственного EXE

```bash
# Убедитесь что в папке есть все PNG-иконки и icon.ico
build.bat
# Готовый EXE появится в папке dist/
```

> ⚠️ **Важно:** перед первым использованием рекомендуется создать точку восстановления системы. Кнопка есть прямо в программе на вкладке «Главная».

---

## 📖 Руководство пользователя

### Вкладка 🏠 Главная

Стартовый экран. Содержит:
- Кнопку **«Создать точку восстановления»** — создаёт снапшот системы перед оптимизацией
- Кнопку **«Запуск оптимизации»** — запускает все выбранные на вкладке «Настройки» операции
- Статус прав администратора

### Вкладка ⚙ Настройки

Главная вкладка с выбором оптимизаций. Разделена на группы.  
Сверху — **профили** для быстрого выбора набора галочек.

#### Профили оптимизации

| Профиль | Для кого | Что включает |
|---|---|---|
| ☀️ **Full** | Игровой ПК | Всё. Максимальная производительность, отключает Defender, OneDrive, Xbox |
| ⚡ **Medium** | Большинство пользователей | Всё кроме Defender и OneDrive. Баланс скорости и стабильности |
| ✨ **Light** | Первое знакомство | Только безопасная очистка: телеметрия, DNS, мусор |
| 💻 **Laptop** | Ноутбуки | Без агрессивных CPU-твиков. Щадит батарею |

> Повторный клик на активный профиль — сбрасывает все галочки.

---

### 🔧 Описание всех параметров

#### ⚡ Производительность

<details>
<summary><b>Максимальный план электропитания (Ultimate Performance)</b></summary>

Активирует скрытый план Windows «Ultimate Performance». Полностью отключает энергосбережение процессора, убирает таймеры простоя и гарантирует максимальную частоту CPU в любой момент. Разница особенно заметна при частых коротких нагрузках (например, в онлайн-играх с резкими перепадами нагрузки).

</details>

<details>
<summary><b>GPU Hardware Scheduling</b></summary>

Включает аппаратное планирование задач GPU (HAGS). Видеокарта сама управляет своей очередью команд вместо CPU. Снижает задержки рендеринга и немного повышает FPS. Требует: NVIDIA 451.48+ или AMD 20.11.2+, Windows 10 2004+.

</details>

<details>
<summary><b>HPET / Timer Resolution — точность таймера 0.5 ms</b></summary>

Устанавливает максимальную точность системного таймера (0.5 мс вместо стандартных 15.6 мс). Уменьшает DPC latency, стабилизирует фреймтайм. Также отключает Dynamic Tick через bcdedit.

</details>

<details>
<summary><b>Оптимизация pagefile (4–8 GB)</b></summary>

Отключает автоматическое управление файлом подкачки и задаёт фиксированный размер 4096–8192 МБ. Фиксированный размер устраняет фрагментацию и задержки при расширении. `ClearPageFileAtShutdown` очищает pagefile при каждом выключении.

</details>

<details>
<summary><b>Отключить Xbox Game Bar и DVR</b></summary>

Полностью отключает Xbox Game Bar, DVR-запись и сервисы `XblGameSave`/`XboxNetApiSvc`. Убирает фоновый оверлей, который захватывает системные ресурсы и увеличивает input lag.

</details>

<details>
<summary><b>Processor Idle Disable — агрессивный режим CPU</b></summary>

Запрещает процессору переходить в состояния простоя C1/C2/C3. CPU всегда работает на полной частоте. Значительно снижает DPC latency. **Не рекомендуется для ноутбуков** — увеличивает нагрев и потребление.

</details>

<details>
<summary><b>Отключить Core Parking</b></summary>

Отключает механизм «парковки ядер», при котором Windows отключает неиспользуемые ядра. Все ядра всегда активны и готовы к работе без задержки пробуждения.

</details>

<details>
<summary><b>System Responsiveness = 0</b></summary>

Устанавливает `SystemResponsiveness=0` и `GPU Priority=8` для профиля задач «Games». Windows отдаёт 100% процессорного времени мультимедийным и игровым задачам вместо фоновых служб.

</details>

<details>
<summary><b>Отключить Dynamic Tick</b></summary>

`bcdedit /set disabledynamictick yes` — запрещает Windows динамически изменять частоту системного прерывания. Таймер тикает стабильно, снижается джиттер и улучшается равномерность фреймтайма.

</details>

<details>
<summary><b>Принудительный HPET + TSC Sync Enhanced</b></summary>

`useplatformclock true` включает аппаратный счётчик HPET. `tscsyncpolicy Enhanced` синхронизирует TSC-счётчики между ядрами. Повышает точность измерений времени в играх.

</details>

<details>
<summary><b>Полное отключение ускорения мыши</b></summary>

Убирает кривую ускорения мыши (`MouseSpeed=0`, `MouseThreshold1=0`, `MouseThreshold2=0`). Движение курсора становится линейным — каждый физический миллиметр даёт одинаковое экранное перемещение независимо от скорости.

</details>

<details>
<summary><b>CPU Scheduling Optimization (Win 11)</b></summary>

`SchedulingCategory=0` и `Win32PrioritySeparation=38`. Windows увеличивает квант времени для активного приложения на переднем плане. Снижает задержки отклика в играх.

</details>

<details>
<summary><b>Отключить Power Throttling</b></summary>

Отключает механизм, который искусственно ограничивает частоту CPU для фоновых задач. `PROCTHROTTLEMAX=0` и `PowerThrottlingOff=1` запрещают любое программное замедление процессора.

</details>

<details>
<summary><b>Real-Time Thread Boost</b></summary>

Настраивает планировщик Windows так, чтобы активное приложение получало максимальный приоритет. `Win32PrioritySeparation=0x26` даёт переднему процессу более длинный квант без вытеснения фоновыми задачами.

</details>

<details>
<summary><b>Lock Power Scheme</b></summary>

Блокирует план электропитания через реестр, предотвращая его сброс драйверами или обновлениями Windows.

</details>

<details>
<summary><b>Registry Reactivity Boost</b></summary>

`IoPageLockLimit=0xFFFFFFFF` позволяет Windows блокировать максимальное количество страниц I/O в физической памяти. `DisablePagingExecutive=1` запрещает выгрузку кода ядра и драйверов на диск. Снижает задержки дискового I/O.

</details>

---

#### 🔒 Конфиденциальность и телеметрия

<details>
<summary><b>Отключить кейлоггер и телеметрию</b></summary>

`AllowTelemetry=0`, отключение служб `DiagTrack` и `dmwappushservice`. Отключает сбор текста набора клавиш через TIPC и InputPersonalization.

</details>

<details>
<summary><b>Отключить приватные настройки</b></summary>

Отключает рекламный идентификатор, персонализацию на основе диагностики и профилирование поведения пользователя.

</details>

<details>
<summary><b>Отключить задачи телеметрии, Copilot, AI, Recall</b></summary>

Отключает 14 запланированных задач: Compatibility Appraiser, CEIP, DiskDiagnostic, Feedback, Error Reporting, Windows AI, Recall. Деактивирует Copilot через групповые политики.

</details>

<details>
<summary><b>Отключить шпионские задачи планировщика</b></summary>

Отключает задачи Maps, FamilySafety, Location (геолокация) и BrokerTask (фоновый брокер приложений).

</details>

---

#### 🛠 Система

<details>
<summary><b>Отключить Windows Defender</b></summary>

Отключает антивирус через групповые политики. **Рекомендуется только при наличии стороннего антивируса.** Требует перезагрузку.

</details>

<details>
<summary><b>Удалить OneDrive</b></summary>

Три метода удаления: winget (Win10 21H1+/Win11) → классический деинсталлятор → PowerShell Remove-AppxPackage. Дополнительно убирает OneDrive из боковой панели Explorer и блокирует повторную установку.

</details>

<details>
<summary><b>Установить классический просмотрщик фото</b></summary>

Возвращает быстрый Windows Photo Viewer для JPG/PNG/BMP/GIF/TIFF вместо тяжёлого приложения «Фото».

</details>

---

#### 🌐 Сеть

<details>
<summary><b>Оптимизация TCP/IP, DNS-буферов, Nagle</b></summary>

Комплекс твиков: `TCPNoDelay=1` (алгоритм Nagle), увеличение DNS-кэша, `autotuninglevel=normal`, включение DCA и NetDMA, RSS, сброс Winsock и DNS-кэша.

</details>

<details>
<summary><b>Отключить алгоритм Nagle (per-adapter)</b></summary>

Алгоритм Nagle объединяет маленькие TCP-пакеты, добавляя до 200 мс задержки в онлайн-играх. Отключается индивидуально для каждого сетевого адаптера через `TCPIP\Parameters\Interfaces`.

</details>

<details>
<summary><b>Network Stack Boost (RSS, DCA, Chimney, NetDMA)</b></summary>

Receive Side Scaling распределяет обработку пакетов по всем ядрам. DCA помещает данные пакетов прямо в кэш процессора. TCP Chimney переносит обработку TCP на сетевой адаптер.

</details>

<details>
<summary><b>Убрать Network Throttling Index</b></summary>

`NetworkThrottlingIndex=0xFFFFFFFF` — отключает программное ограничение пропускной способности сети, которое Windows вводит «для защиты» мультимедийных потоков.

</details>

<details>
<summary><b>Network Packet Optimization (CTCP)</b></summary>

Compound TCP — улучшенный алгоритм управления перегрузкой, агрессивнее использует полосу. ECN отключается для совместимости с большинством роутеров.

</details>

<details>
<summary><b>Сброс DNS-кэша и Winsock</b></summary>

`ipconfig /flushdns` + `netsh winsock reset catalog` — полное пересоздание сетевого стека. Устраняет артефакты после установки VPN или антивирусов.

</details>

---

#### ⚙ Службы и автозагрузка

<details>
<summary><b>Отключить лишние службы</b></summary>

Анализирует все автоматические службы и переводит нагрузочные в ручной режим. **Защита:** Nvidia, AMD, Intel, FACEIT, EAC, VPN, аудио, сеть, ключевые компоненты Microsoft — не трогаются.

</details>

<details>
<summary><b>Отключить индексирование поиска</b></summary>

Останавливает `WSearch`. Индексатор постоянно сканирует диск в фоне — после отключения снижается нагрузка на HDD/SSD. Особенно заметно на медленных дисках.

</details>

---

#### 🧹 Обслуживание системы

<details>
<summary><b>Memory Cleaner</b></summary>

Использует `NtSetSystemInformation` с командами `MemoryPurgeStandbyList` и `MemoryFlushModifiedList`. Это тот же механизм, что использует RAMMap. **Не трогает рабочие наборы процессов** — только освобождает кэшированные страницы памяти.

</details>

<details>
<summary><b>Исправление ошибок диска (CHKDSK /f)</b></summary>

Планирует запуск CHKDSK при следующей перезагрузке. Проверяет файловую систему и исправляет повреждённые секторы.

</details>

<details>
<summary><b>Очистка реестра</b></summary>

Удаляет накопленный мусор: RecentDocs, RunMRU, TypedPaths, UserAssist, ComDlg32 MRU, ShellBags.

</details>

<details>
<summary><b>Удаление мусора</b></summary>

Многоэтапная очистка: кэш Windows Update, браузер Edge, Microsoft Store (`wsreset`), TEMP, Prefetch, сжатие WinSxS через DISM.

</details>

---

### Вкладка 💻 Система

Отображает информацию о вашем железе: CPU, GPU (с объёмом VRAM), RAM.

- **Копировать** — копирует характеристики с готовым промптом для разгона через MSI Afterburner в буфер обмена
- **DeepSeek** — открывает чат для консультации по разгону
- **MSI Afterburner** — запускает программу из папки `MSI/` рядом с exe

---

### Вкладка ⚡ Fast Utility

Мгновенные операции **без перезагрузки**. Два столбца:

**Левый — утилиты:**

| Операция | Описание |
|---|---|
| Memory Cleaner | Очистка Standby/Modified RAM через NtSetSystemInformation |
| Удалить временные файлы | TEMP, Prefetch — быстрая очистка |
| Сбросить кэш иконок | Удаляет `iconcache*.db`, перезапускает Explorer |
| Очистить thumbnails | Удаляет `thumbcache_*.db` |
| Очистить буфер обмена | Мгновенно |
| Flush DNS | `ipconfig /flushdns` + регистрация DNS |
| Быстрый сброс сети | TCP/ARP/Winsock |
| Kill bloatware | Завершает фоновые процессы Microsoft (OneDrive, Edge Update, SearchIndexer и др.) |
| Перезапустить Explorer | Устраняет зависания без перезагрузки |
| Ultimate Performance | Активирует план питания |
| Убрать задержку автозапуска | `StartupDelayInMSec=0` |

**Правый — визуальные твики:**

| Твик | Описание |
|---|---|
| 🖱 Цвет выделения мышкой | RGB-палитра + HEX ввод. Меняет цвет рамки выделения файлов |
| 🖊 Цвет выделения текста | Отдельная палитра для цвета выделения текста в интерфейсе |
| Убрать стрелочки с ярлыков | Чистые иконки без стрелки |
| Показывать расширения файлов | `.exe`, `.mp4` и т.д. всегда видны |
| Показывать скрытые файлы | Папки и файлы с атрибутом Hidden |
| Тёмный режим Windows | Системный тёмный режим для приложений и интерфейса |
| Отключить Aero Shake | Отключает «встряхивание» для сворачивания окон |
| Ускорить Alt+Tab | Убирает анимацию переключения |
| Отключить Sticky Keys | Убирает диалог залипания при 5 нажатиях Shift |

---

## ⚙️ Структура проекта

```
minoreOptimizer/
├── optimizer.py          ← основной файл программы
├── build.bat             ← сборка standalone EXE
├── version_info.txt      ← метаданные для EXE
├── requirements.txt      ← зависимости Python
├── icon.png              ← иконка приложения
├── icon.ico              ← иконка для EXE
├── github.png            ← иконка кнопки GitHub
├── msi.png               ← иконка кнопки MSI Afterburner
├── deepsek.png           ← иконка кнопки DeepSeek
├── restore.png           ← иконка кнопки восстановления
├── parameters_description.txt ← описание всех параметров
└── MSI/
    └── MSIAfterburner.exe ← (опционально, положите сами)
```

---

## 🛡️ Безопасность и откат

- **Перед оптимизацией** нажмите «Создать точку восстановления» — это занимает 30 секунд и позволяет полностью откатить все изменения
- Откат: `Win+R` → `rstrui.exe` → выберите точку «minoreOptimizer Restore Point»
- Все операции логируются в консоли в реальном времени
- SFC и DISM запускаются автоматически в конце для проверки целостности системы

---

## ❓ Частые вопросы

<details>
<summary><b>Программа требует права администратора — это нормально?</b></summary>

Да. Все операции (изменение реестра, управление службами, bcdedit, powercfg) требуют прав администратора. Программа запрашивает их через UAC при запуске.

</details>

<details>
<summary><b>Нужна ли перезагрузка?</b></summary>

Часть изменений применяется мгновенно, часть (bcdedit, драйверы, pagefile) — после перезагрузки. Программа явно указывает «требуется перезагрузка» рядом с такими пунктами. Кнопка перезагрузки появляется в консоли по завершении.

</details>

<details>
<summary><b>Можно ли отменить изменения?</b></summary>

Да — через точку восстановления системы, которую программа предлагает создать перед запуском.

</details>

<details>
<summary><b>Почему отключение Defender выделено отдельно?</b></summary>

Это необратимое и потенциально опасное действие. Оно не включено в профили Medium и Light — только в Full. Рекомендуется только при наличии альтернативного антивируса.

</details>

<details>
<summary><b>Программа работает на Windows 10?</b></summary>

Да, Windows 10 версии 1903 и выше. Часть функций (CPU Scheduling, некоторые параметры Win11) могут не применяться на Win10 — программа корректно их пропускает.

</details>

---

## 📦 Сборка EXE

```bash
# Требования: Python 3.10+, все PNG-иконки в папке
pip install PyQt6 pyinstaller

# Запустите build.bat от имени администратора
build.bat

# Результат: dist/minoreOptimizer.exe (~80 MB, не требует Python)
```

Готовый EXE полностью автономен — не требует установки Python или каких-либо зависимостей.

---

## 🤝 Контрибьюция

Pull requests приветствуются. Для крупных изменений — сначала откройте Issue для обсуждения.

1. Fork репозитория
2. Создайте ветку (`git checkout -b feature/amazing-tweak`)
3. Commit изменений (`git commit -m 'Add amazing tweak'`)
4. Push в ветку (`git push origin feature/amazing-tweak`)
5. Откройте Pull Request

---

## 📄 Лицензия

MIT License — см. файл [LICENSE](LICENSE)

---

<div align="center">

Сделано с ❤️ by [bpm500](https://github.com/bpm500)

**[⭐ Поставьте звезду если программа помогла!](https://github.com/bpm500/minoreOptimizer)**

</div>
