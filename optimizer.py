"""
minoreOptimizer — Windows System Optimization Tool
License: MIT License
Copyright (c) 2024 bpm500 — https://github.com/bpm500
"""

import sys
import os
import subprocess
import ctypes
import platform
import webbrowser
import threading
import time
import json
import winreg

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QTabWidget, QVBoxLayout,
    QHBoxLayout, QPushButton, QLabel, QCheckBox, QScrollArea,
    QGroupBox, QFrame, QTextEdit, QMessageBox, QSpacerItem, QSizePolicy,
    QLineEdit, QColorDialog, QSlider, QSpinBox, QGridLayout
)
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QSize, QTimer, QPropertyAnimation, QEasingCurve
from PyQt6.QtGui import QFont, QIcon, QColor, QPalette, QPixmap, QCursor, QPainter, QBrush

# ══════════════════════════════════════════════════════════════════
#  HELPERS
# ══════════════════════════════════════════════════════════════════

if getattr(sys, "frozen", False):
    BASE_DIR    = os.path.dirname(sys.executable)
    _BUNDLE_DIR = sys._MEIPASS  # type: ignore[attr-defined]
else:
    BASE_DIR    = os.path.dirname(os.path.abspath(__file__))
    _BUNDLE_DIR = BASE_DIR

def asset(name):
    p = os.path.join(BASE_DIR, name)
    if os.path.exists(p):
        return p
    return os.path.join(_BUNDLE_DIR, name)

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def run_as_admin():
    ctypes.windll.shell32.ShellExecuteW(
        None, "runas", sys.executable, " ".join(sys.argv), None, 1
    )

def run_cmd(cmd, timeout=30):
    """Run command with timeout. Returns (output, success)."""
    try:
        r = subprocess.run(
            cmd, shell=True, capture_output=True,
            text=True, encoding="utf-8", errors="ignore",
            timeout=timeout
        )
        out = (r.stdout + r.stderr).strip()
        ok  = r.returncode == 0
        return out, ok
    except subprocess.TimeoutExpired:
        return f"[TIMEOUT] Command exceeded {timeout}s limit", False
    except Exception as e:
        return str(e), False

def run_cmd_s(cmd, timeout=30):
    """run_cmd but returns only the string (legacy compat)."""
    out, _ = run_cmd(cmd, timeout)
    return out

def run_ps(script, timeout=30):
    return run_cmd_s(
        f'powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "{script}"',
        timeout
    )

def safe_run_batch(cmds, log_fn, label="", max_errors=5, timeout=20):
    """
    Smart batch runner with adaptive logic:
    - TIMEOUT is treated as WARNING only, NOT as an error.
      A slow HDD can delay reg/netsh commands — we wait and move on.
    - ERROR counter only increments on real failures (returncode != 0
      AND output contains known error keywords).
    - Aborts only after max_errors REAL errors in a row.
    - Returns True if completed, False if aborted.
    """
    # Keywords that indicate a genuine failure vs. "command not found on this OS"
    REAL_ERROR_KEYWORDS = [
        "access is denied", "not recognized", "invalid parameter",
        "failed", "error:", "cannot", "refused", "0x8"
    ]
    errors = 0
    for cmd in cmds:
        out, ok = run_cmd(cmd, timeout)

        if ok or not out:
            errors = 0  # success — reset error streak
            continue

        out_low = out.lower()

        # Timeout: warn but do NOT count as error — slow HDD/system
        if "[TIMEOUT]" in out:
            log_fn(f"  ⚠ [SLOW] {label} — команда медленная, продолжаем: {cmd[:55]}...")
            continue

        # "Not applicable" style results — command ran but feature not present
        # e.g. schtasks on non-existent task, sc on non-existent service
        if any(k in out_low for k in ["не существует", "not exist", "не найден",
                                       "no tasks", "specified service", "1060",
                                       "element not found", "no instances"]):
            continue  # silently skip — feature just not present on this system

        # Real error
        is_real_err = any(k in out_low for k in REAL_ERROR_KEYWORDS)
        if is_real_err:
            errors += 1
            log_fn(f"  ✗ [ERR {errors}/{max_errors}] {cmd[:60]}")
            if errors >= max_errors:
                log_fn(f"  ⚠  {label}: {max_errors} реальных ошибок подряд — пропускаем блок")
                return False
        # Non-zero but not a recognized error = warning only
    return True


def run_process_streaming(cmd, log_fn, parse_progress=None, timeout=600):
    """
    Run a long command and stream its output line-by-line.
    parse_progress: optional callable(line) -> str|None — returns display text or None
    Filters out garbled non-ASCII (Russian console artifacts etc.).
    Returns True on success.
    """
    import re
    try:
        proc = subprocess.Popen(
            cmd, shell=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, encoding="utf-8", errors="replace"
        )
        deadline = time.time() + timeout
        for raw_line in proc.stdout:
            if time.time() > deadline:
                proc.kill()
                log_fn("  ⚠ [TIMEOUT] Процесс прерван по таймауту")
                return False
            line = raw_line.strip()
            if not line:
                continue
            # Strip obvious encoding garbage (sequences of replacement chars)
            line_clean = re.sub(r'[^\x00-\x7F\u0400-\u04FF\s%.,\[\]():/\\=_\-+#!?]', '', line).strip()
            if not line_clean:
                continue
            if parse_progress:
                display = parse_progress(line_clean)
                if display:
                    log_fn(display)
            else:
                log_fn(f"  {line_clean}")
        proc.wait()
        return proc.returncode == 0
    except Exception as e:
        log_fn(f"  ✗ Ошибка запуска: {e}")
        return False

# ══════════════════════════════════════════════════════════════════
#  STYLESHEET
# ══════════════════════════════════════════════════════════════════

STYLE = """
* { font-family: 'Segoe UI', Arial, sans-serif; }

QMainWindow, QWidget#root { background: #181818; }

QTabWidget::pane { border: none; background: #1e1e1e; }
QTabBar { background: #141414; }
QTabBar::tab {
    background: #141414; color: #888;
    padding: 11px 24px; font-size: 13px;
    border-bottom: 2px solid transparent; margin-right: 1px;
}
QTabBar::tab:selected {
    color: #e8e8e8; border-bottom: 2px solid #4f9ef8; background: #1e1e1e;
}
QTabBar::tab:hover:!selected { color: #bbb; background: #1a1a1a; }

QScrollArea { border: none; background: transparent; }
QScrollBar:vertical { background: #1e1e1e; width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: #444; border-radius: 3px; min-height: 24px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QGroupBox {
    color: #7ab4f5; border: 1px solid #2a2a2a; border-radius: 8px;
    margin-top: 12px; padding: 14px 12px 10px 12px;
    font-size: 12px; font-weight: 600; letter-spacing: 0.5px;
}
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }

QCheckBox { color: #c8c8c8; font-size: 13px; spacing: 10px; padding: 4px 2px; }
QCheckBox::indicator {
    width: 17px; height: 17px; border: 1px solid #444;
    border-radius: 4px; background: #252525;
}
QCheckBox::indicator:checked { background: #4f9ef8; border-color: #4f9ef8; }
QCheckBox::indicator:hover { border-color: #666; }
QCheckBox:hover { color: #e8e8e8; }

QPushButton {
    background: #2a2a2a; color: #c8c8c8; border: 1px solid #383838;
    border-radius: 6px; padding: 8px 16px; font-size: 13px;
}
QPushButton:hover  { background: #333; border-color: #555; color: #e8e8e8; }
QPushButton:pressed { background: #222; }
QPushButton:disabled { color: #555; border-color: #2a2a2a; }

QPushButton#btnOptimize {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1a5fc4, stop:1 #1248a0);
    color: #fff; font-size: 15px; font-weight: 700;
    border: none; border-radius: 8px; padding: 13px 36px; letter-spacing: 0.3px;
}
QPushButton#btnOptimize:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #2068d4, stop:1 #1a55b5);
}
QPushButton#btnRestore {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1a7a3a, stop:1 #145e2c);
    color: #fff; font-size: 13px; border: none; border-radius: 7px; padding: 11px 22px;
}
QPushButton#btnRestore:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #218a44, stop:1 #1a7a3a);
}
QPushButton#btnCopy {
    background: #252530; color: #8ab4f8; border: 1px solid #363660;
    border-radius: 6px; padding: 8px 16px; font-size: 12px;
}
QPushButton#btnCopy:hover { background: #2d2d42; }
QPushButton#btnDeepSeek {
    background: #1a2540; color: #7ab4f5; border: 1px solid #2a3a60;
    border-radius: 6px; padding: 8px 16px; font-size: 12px;
}
QPushButton#btnDeepSeek:hover { background: #202e54; }
QPushButton#btnMSI {
    background: #2a1515; color: #f47878; border: 1px solid #4a2020;
    border-radius: 6px; padding: 8px 16px; font-size: 12px;
}
QPushButton#btnMSI:hover { background: #341a1a; }
QPushButton#btnGithub {
    background: transparent; color: #888; border: 1px solid #333;
    border-radius: 6px; padding: 7px 14px; font-size: 12px;
}
QPushButton#btnGithub:hover { background: #252525; color: #bbb; border-color: #555; }

QLabel#titleBig { font-size: 24px; font-weight: 700; color: #f0f0f0; }
QLabel#titleSub { font-size: 13px; color: #666; }
QLabel#infoCard {
    background: #232323; border: 1px solid #2d2d2d; border-radius: 10px;
    color: #c8c8c8; font-size: 14px; padding: 18px 20px;
}
QLabel#sysCard {
    background: #1c1c1c; border: 1px solid #2a2a2a; border-radius: 10px;
    color: #c8c8c8; font-size: 14px; padding: 20px 22px;
}
QLabel#warnLabel {
    color: #e07a5f; font-size: 12px; font-style: italic;
    padding: 8px 4px 4px 4px; border-top: 1px solid #2d2d2d; margin-top: 6px;
}
QFrame#sep { background: #282828; max-height: 1px; }

QPushButton#profileFull, QPushButton#profileMedium,
QPushButton#profileLight, QPushButton#profileLaptop {
    background: #242424; color: #777; border: 1px solid #333;
    border-radius: 8px; font-size: 12px; font-weight: 500; padding: 6px 4px;
}
QPushButton#profileFull:hover, QPushButton#profileMedium:hover,
QPushButton#profileLight:hover, QPushButton#profileLaptop:hover {
    background: #2c2c2c; border-color: #555; color: #bbb;
}
QPushButton#profileFull[active='true']   { background:#1a2a3a; border:2px solid #4f9ef8; color:#7ab4f5; font-weight:700; }
QPushButton#profileMedium[active='true'] { background:#1e2a1e; border:2px solid #4edd4e; color:#7de87d; font-weight:700; }
QPushButton#profileLight[active='true']  { background:#2a2a1a; border:2px solid #f0c040; color:#f0d070; font-weight:700; }
QPushButton#profileLaptop[active='true'] { background:#2a1a2a; border:2px solid #c07af0; color:#d4a0f8; font-weight:700; }

QLabel#profileDesc {
    background: #1e1e2a; border: 1px solid #2a2a3a; border-radius: 8px;
    color: #9ab4d8; font-size: 12px; padding: 10px 14px; font-style: italic;
}

QWidget#consoleRoot { background: #0d0d0d; }
QTextEdit#console {
    background: #0d0d0d; color: #b8ffb8; border: none;
    font-family: 'Cascadia Code', 'Consolas', monospace; font-size: 12px;
    selection-background-color: #1a3a1a;
}
QPushButton#btnReboot {
    background: #143a14; color: #4edd4e; border: 1px solid #247a24;
    border-radius: 6px; padding: 9px 22px; font-size: 13px;
}
QPushButton#btnReboot:hover { background: #1a4e1a; }
QPushButton#btnReboot:disabled { background: #1e1e1e; color: #444; border-color: #333; }
QPushButton#btnCloseConsole {
    background: #2a1414; color: #e07a5f; border: 1px solid #4a2020;
    border-radius: 6px; padding: 9px 22px; font-size: 13px;
}
QPushButton#btnCloseConsole:hover { background: #361c1c; }

/* ── Fast Utility Tab ── */
QWidget#fuCard {
    background: #1e1e1e;
    border: 1px solid #2a2a2a;
    border-radius: 10px;
}
QPushButton#fuRunBtn {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1a5fc4, stop:1 #1248a0);
    color: #fff; font-size: 14px; font-weight: 700;
    border: none; border-radius: 8px; padding: 12px 32px;
}
QPushButton#fuRunBtn:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #2068d4, stop:1 #1a55b5);
}
QPushButton#fuRunBtn:disabled { background: #252525; color: #555; }
QLineEdit#rgbInput {
    background: #1a1a1a; color: #c8c8c8; border: 1px solid #3a3a3a;
    border-radius: 5px; padding: 5px 10px; font-size: 12px;
    font-family: 'Consolas', monospace;
}
QLineEdit#rgbInput:focus { border-color: #4f9ef8; }
QPushButton#colorPreview {
    border: 2px solid #444; border-radius: 6px; min-width: 36px; max-width: 36px;
    min-height: 36px; max-height: 36px;
}
QPushButton#colorPreview:hover { border-color: #888; }
QSlider::groove:horizontal {
    height: 4px; background: #333; border-radius: 2px;
}
QSlider::handle:horizontal {
    background: #4f9ef8; width: 14px; height: 14px; margin: -5px 0;
    border-radius: 7px;
}
QSlider::sub-page:horizontal { background: #4f9ef8; border-radius: 2px; }
QLabel#colorSwatch {
    border: 2px solid #444; border-radius: 6px;
    min-width: 24px; min-height: 24px;
}
"""

# ══════════════════════════════════════════════════════════════════
#  SYSTEM INFO
# ══════════════════════════════════════════════════════════════════

def _get_gpu_full_info():
    results = []

    # Strategy 1: NVIDIA NVML
    try:
        nvml = ctypes.windll.LoadLibrary("nvml.dll")
        nvml.nvmlInit()
        count = ctypes.c_uint(0)
        nvml.nvmlDeviceGetCount(ctypes.byref(count))

        class nvmlMemory_t(ctypes.Structure):
            _fields_ = [("total", ctypes.c_ulonglong),
                        ("free",  ctypes.c_ulonglong),
                        ("used",  ctypes.c_ulonglong)]

        for i in range(count.value):
            handle = ctypes.c_void_p()
            nvml.nvmlDeviceGetHandleByIndex(i, ctypes.byref(handle))
            mem = nvmlMemory_t()
            nvml.nvmlDeviceGetMemoryInfo(handle, ctypes.byref(mem))
            name_buf = ctypes.create_string_buffer(96)
            nvml.nvmlDeviceGetName(handle, name_buf, 96)
            name = name_buf.value.decode("utf-8", errors="replace").strip()
            if name:
                results.append((name, mem.total / (1024 ** 3)))
        nvml.nvmlShutdown()
        if results:
            return results
    except Exception:
        pass

    # Strategy 2: Intel via registry
    try:
        path = r"SYSTEM\CurrentControlSet\Control\Class\{4d36e968-e325-11ce-bfc1-08002be10318}"
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path) as key:
            for i in range(winreg.QueryInfoKey(key)[0]):
                try:
                    with winreg.OpenKey(key, winreg.EnumKey(key, i)) as sub:
                        try:
                            desc, _ = winreg.QueryValueEx(sub, "DriverDesc")
                            if "Intel" in desc and any(x in desc for x in ("Arc", "UHD", "Iris")):
                                try:
                                    m, _ = winreg.QueryValueEx(sub, "HardwareInformation.AdapterMemorySize")
                                    results.append((desc, abs(int(m)) / (1024 ** 3)))
                                except Exception:
                                    results.append((desc, 0.0))
                        except Exception:
                            pass
                except Exception:
                    pass
    except Exception:
        pass
    if results:
        return results

    # Strategy 3: PowerShell CIM uint64
    try:
        cmd = (
            "$gpus = Get-CimInstance Win32_VideoController | "
            "Where-Object { $_.Name -notmatch 'Parsec|Virtual|Remote|Indirect|Microsoft Basic|RDP|SpaceDesk|Moonlight|Sunshine|AnyDesk|TeamViewer|VNC|Citrix' }; "
            "$list = $gpus | ForEach-Object { @{ Name = $_.Name; VRAM = [uint64]$_.AdapterRAM } }; "
            "$list | ConvertTo-Json -Compress"
        )
        raw = subprocess.check_output(
            ["powershell", "-NoProfile", "-NonInteractive", "-Command", cmd],
            text=True, creationflags=0x08000000, timeout=15
        )
        data = json.loads(raw.strip())
        if isinstance(data, dict):
            data = [data]
        for item in data:
            name = str(item.get("Name", "")).strip()
            vram_bytes = int(item.get("VRAM", 0))
            if name:
                results.append((name, vram_bytes / (1024 ** 3)))
    except Exception:
        pass

    return results if results else [("Не определено", 0.0)]


def get_system_info():
    info = {}
    try:
        out = run_cmd_s("wmic cpu get Name,MaxClockSpeed /format:list", 10)
        name = freq = ""
        for line in out.splitlines():
            if line.startswith("Name="):
                name = line.split("=", 1)[1].strip()
            elif line.startswith("MaxClockSpeed="):
                mhz = line.split("=", 1)[1].strip()
                try:
                    freq = f"{int(mhz)/1000:.2f} GHz"
                except:
                    freq = mhz
        info["cpu"] = f"{name}  —  {freq}" if name else "Не определено"
    except:
        info["cpu"] = "Ошибка"

    try:
        gpu_list = _get_gpu_full_info()
        lines = [f"{n}  —  {v:.1f} GB" if v > 0.1 else n for n, v in gpu_list]
        info["gpu"]      = "\n".join(lines) if lines else "Не определено"
        info["gpu_list"] = gpu_list
    except:
        info["gpu"]      = "Ошибка"
        info["gpu_list"] = []

    try:
        out = run_cmd_s("wmic memorychip get Capacity,Speed /format:list", 10)
        total, speeds = 0, []
        for line in out.splitlines():
            if line.startswith("Capacity="):
                try:
                    total += int(line.split("=", 1)[1].strip())
                except:
                    pass
            elif line.startswith("Speed="):
                spd = line.split("=", 1)[1].strip()
                if spd and spd not in speeds:
                    speeds.append(spd)
        info["ram"] = f"{total//(1024**3)} GB  —  {speeds[0]+' MHz' if speeds else '?'}"
    except:
        info["ram"] = "Ошибка"

    return info

# ══════════════════════════════════════════════════════════════════
#  TOAST NOTIFICATION
# ══════════════════════════════════════════════════════════════════

class ToastNotification(QWidget):
    def __init__(self, parent, message):
        super().__init__(parent,
            Qt.WindowType.FramelessWindowHint |
            Qt.WindowType.Tool |
            Qt.WindowType.WindowStaysOnTopHint)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(Qt.WidgetAttribute.WA_ShowWithoutActivating)

        lay = QHBoxLayout(self)
        lay.setContentsMargins(18, 12, 18, 12)
        icon = QLabel("✅")
        icon.setStyleSheet("font-size:18px; background:transparent;")
        lbl  = QLabel(message)
        lbl.setStyleSheet(
            "color:#e8e8e8; font-size:14px; font-weight:600; "
            "background:transparent; font-family:'Segoe UI';"
        )
        lay.addWidget(icon)
        lay.addSpacing(6)
        lay.addWidget(lbl)
        self.setStyleSheet(
            "ToastNotification { background:#1e2a3a; border:1px solid #2e4a6a; border-radius:10px; }"
        )
        self.adjustSize()
        if parent:
            pr = parent.rect()
            px = parent.mapToGlobal(pr.topLeft())
            self.move(px.x() + (pr.width()-self.width())//2,
                      px.y() + (pr.height()-self.height())//2)
        self.setWindowOpacity(0.0)
        self.show()
        self._a_in = QPropertyAnimation(self, b"windowOpacity")
        self._a_in.setDuration(250)
        self._a_in.setStartValue(0.0)
        self._a_in.setEndValue(0.96)
        self._a_in.setEasingCurve(QEasingCurve.Type.OutCubic)
        self._a_in.start()
        QTimer.singleShot(3200, self._fade_out)

    def _fade_out(self):
        self._a_out = QPropertyAnimation(self, b"windowOpacity")
        self._a_out.setDuration(350)
        self._a_out.setStartValue(0.96)
        self._a_out.setEndValue(0.0)
        self._a_out.setEasingCurve(QEasingCurve.Type.InCubic)
        self._a_out.finished.connect(self.close)
        self._a_out.start()

# ══════════════════════════════════════════════════════════════════
#  OPTIMIZATION WORKER
# ══════════════════════════════════════════════════════════════════

class OptimizeWorker(QThread):
    log      = pyqtSignal(str)
    finished = pyqtSignal()

    def __init__(self, settings):
        super().__init__()
        self.s = settings

    def L(self, text):
        self.log.emit(text)

    def _batch(self, cmds, label="", timeout=20):
        """Safe batch runner — aborts block after 5 errors, respects timeouts."""
        return safe_run_batch(cmds, self.L, label, max_errors=5, timeout=timeout)

    def _ps(self, script, timeout=25):
        out = run_ps(script, timeout)
        if "[TIMEOUT]" in out:
            self.L(f"  ✗ [TIMEOUT] PowerShell script exceeded {timeout}s")
        return out

    def run(self):
        s = self.s
        self.L("=" * 64)
        self.L("  minoreOptimizer — Запуск оптимизации системы")
        self.L("=" * 64)

        # ── 1. Power plan ────────────────────────────────────────
        if s.get("perf_preset"):
            self.L("\n[►] Установка плана электропитания...")
            run_cmd_s("powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 2>NUL", 10)
            run_cmd_s("powercfg /setactive e9a42b02-d5df-448d-aa00-03f14749eb61 2>NUL", 10)
            run_cmd_s("powercfg /change monitor-timeout-ac 0", 5)
            run_cmd_s("powercfg /change standby-timeout-ac 0", 5)
            run_cmd_s("powercfg /change hibernate-timeout-ac 0", 5)
            self.L("    ✓ Готово")

        # ── 2. Telemetry ─────────────────────────────────────────
        if s.get("disable_telemetry"):
            self.L("\n[►] Отключение телеметрии и кейлоггера...")
            self._batch([
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection" /v AllowTelemetry /t REG_DWORD /d 0 /f',
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection" /v AllowTelemetry /t REG_DWORD /d 0 /f',
                'sc stop DiagTrack & sc config DiagTrack start= disabled',
                'sc stop dmwappushservice & sc config dmwappushservice start= disabled',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\Input\\TIPC" /v Enabled /t REG_DWORD /d 0 /f',
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\InputPersonalization" /v AllowInputPersonalization /t REG_DWORD /d 0 /f',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\InputPersonalization" /v RestrictImplicitTextCollection /t REG_DWORD /d 1 /f',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\InputPersonalization" /v RestrictImplicitInkCollection /t REG_DWORD /d 1 /f',
            ], "Телеметрия")
            self.L("    ✓ Готово")

        # ── 3. Privacy ───────────────────────────────────────────
        if s.get("disable_privacy"):
            self.L("\n[►] Отключение приватных настроек...")
            self._batch([
                'reg add "HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AdvertisingInfo" /v Enabled /t REG_DWORD /d 0 /f',
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\AdvertisingInfo" /v DisabledByGroupPolicy /t REG_DWORD /d 1 /f',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Privacy" /v TailoredExperiencesWithDiagnosticDataEnabled /t REG_DWORD /d 0 /f',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\Personalization\\Settings" /v AcceptedPrivacyPolicy /t REG_DWORD /d 0 /f',
            ], "Приватность")
            self.L("    ✓ Готово")

        # ── 4. Telemetry tasks ───────────────────────────────────
        if s.get("disable_telemetry_tasks"):
            self.L("\n[►] Отключение задач телеметрии, Copilot, AI, Recall...")
            tasks = [
                "\\Microsoft\\Windows\\Application Experience\\Microsoft Compatibility Appraiser",
                "\\Microsoft\\Windows\\Application Experience\\ProgramDataUpdater",
                "\\Microsoft\\Windows\\Customer Experience Improvement Program\\Consolidator",
                "\\Microsoft\\Windows\\Customer Experience Improvement Program\\UsbCeip",
                "\\Microsoft\\Windows\\DiskDiagnostic\\Microsoft-Windows-DiskDiagnosticDataCollector",
                "\\Microsoft\\Windows\\Feedback\\Siuf\\DmClient",
                "\\Microsoft\\Windows\\Feedback\\Siuf\\DmClientOnScenarioDownload",
                "\\Microsoft\\Windows\\Windows Error Reporting\\QueueReporting",
                "\\Microsoft\\Windows\\CloudExperienceHost\\CreateObjectTask",
                "\\Microsoft\\Windows\\Autochk\\Proxy",
                "\\Microsoft\\Windows\\NetTrace\\GatherNetworkInfo",
                "\\Microsoft\\Windows\\Diagnosis\\Scheduled",
                "\\Microsoft\\Windows\\WindowsAI\\CreateSuggestionIndex",
                "\\Microsoft\\Windows\\Recall\\AutoIndexRecall",
            ]
            self._batch(
                [f'schtasks /Change /Disable /TN "{t}" 2>NUL' for t in tasks],
                "Задачи телеметрии", timeout=8
            )
            run_cmd_s('reg add "HKCU\\Software\\Policies\\Microsoft\\Windows\\WindowsCopilot" /v TurnOffWindowsCopilot /t REG_DWORD /d 1 /f', 5)
            self.L("    ✓ Готово")

        # ── 5. Spy tasks ─────────────────────────────────────────
        if s.get("disable_spy_tasks"):
            self.L("\n[►] Отключение шпионских задач планировщика...")
            self._batch(
                [f'schtasks /Change /Disable /TN "{t}" 2>NUL' for t in [
                    "\\Microsoft\\Windows\\Maps\\MapsToastTask",
                    "\\Microsoft\\Windows\\Maps\\MapsUpdateTask",
                    "\\Microsoft\\Windows\\Shell\\FamilySafetyMonitor",
                    "\\Microsoft\\Windows\\Shell\\FamilySafetyRefreshTask",
                    "\\Microsoft\\Windows\\Location\\Notifications",
                    "\\Microsoft\\Windows\\Location\\WindowsActionDialog",
                    "\\Microsoft\\Windows\\BrokerInfrastructure\\BrokerTask",
                    "\\Microsoft\\Windows\\Management\\Provisioning\\Cellular",
                ]], "Шпионские задачи", timeout=8
            )
            self.L("    ✓ Готово")

        # ── 6. Defender ──────────────────────────────────────────
        if s.get("disable_defender"):
            self.L("\n[►] Отключение Windows Defender...")
            self._batch([
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows Defender" /v DisableAntiSpyware /t REG_DWORD /d 1 /f',
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows Defender\\Real-Time Protection" /v DisableRealtimeMonitoring /t REG_DWORD /d 1 /f',
            ], "Defender")
            run_ps("Set-MpPreference -DisableRealtimeMonitoring $true", 15)
            self.L("    ✓ Готово (требуется перезагрузка)")

        # ── 7. OneDrive ──────────────────────────────────────────
        if s.get("remove_onedrive"):
            self.L("\n[►] Удаление OneDrive...")
            run_cmd_s("taskkill /f /im OneDrive.exe 2>NUL", 5)
            time.sleep(1)
            # Method 1: winget (Win10 21H1+ / Win11)
            out, ok = run_cmd("winget uninstall --id Microsoft.OneDrive --silent --accept-source-agreements 2>NUL", 45)
            if ok:
                self.L("  ✓ Удалено через winget")
            else:
                # Method 2: classic installer (legacy builds)
                self.L("  > winget недоступен, используем классический деинсталлятор...")
                run_cmd_s('%SystemRoot%\\SysWOW64\\OneDriveSetup.exe /uninstall 2>NUL', 30)
                run_cmd_s('%SystemRoot%\\System32\\OneDriveSetup.exe /uninstall 2>NUL', 30)
                # Method 3: PowerShell package removal
                run_ps(
                    "Get-AppxPackage *OneDrive* | Remove-AppxPackage -ErrorAction SilentlyContinue; "
                    "Get-AppxProvisionedPackage -Online | Where-Object DisplayName -like *OneDrive* | "
                    "Remove-AppxProvisionedPackage -Online -ErrorAction SilentlyContinue",
                    timeout=40
                )
            # Always: disable via policy + remove autostart
            run_cmd_s('reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\OneDrive" /v DisableFileSyncNGSC /t REG_DWORD /d 1 /f', 5)
            run_cmd_s('reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\OneDrive" /v DisableLibrariesDefaultSaveToOneDrive /t REG_DWORD /d 1 /f', 5)
            run_cmd_s('reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" /v OneDrive /f 2>NUL', 5)
            # Remove OneDrive from Explorer sidebar
            run_cmd_s('reg add "HKCU\\Software\\Classes\\CLSID\\{018D5C66-4533-4307-9B53-224DE2ED1FE6}" /v System.IsPinnedToNameSpaceTree /t REG_DWORD /d 0 /f', 5)
            self.L("    ✓ Готово")

        # ── 8. Photo viewer ──────────────────────────────────────
        if s.get("set_photo_viewer"):
            self.L("\n[►] Установка классического просмотрщика фото...")
            self._batch(
                [f'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations" /v "{ext}" /t REG_SZ /d "PhotoViewer.FileAssoc.Tiff" /f'
                 for ext in [".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tif", ".tiff"]],
                "PhotoViewer", timeout=5
            )
            self.L("    ✓ Готово")

        # ── 9. Xbox Game Bar ─────────────────────────────────────
        if s.get("disable_xbox_gamebar"):
            self.L("\n[►] Отключение Xbox Game Bar и DVR...")
            self._batch([
                'reg add "HKCU\\System\\GameConfigStore" /v GameDVR_Enabled /t REG_DWORD /d 0 /f',
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\GameDVR" /v AllowGameDVR /t REG_DWORD /d 0 /f',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameDVR" /v AppCaptureEnabled /t REG_DWORD /d 0 /f',
                'reg add "HKCU\\SOFTWARE\\Microsoft\\GameBar" /v UseNexusForGameBarEnabled /t REG_DWORD /d 0 /f',
                'sc stop XblGameSave & sc config XblGameSave start= disabled',
                'sc stop XboxNetApiSvc & sc config XboxNetApiSvc start= disabled',
            ], "Xbox GameBar")
            self.L("    ✓ Готово")

        # ── 10. GPU HW Scheduling ────────────────────────────────
        if s.get("gpu_hw_scheduling"):
            self.L("\n[►] Включение GPU Hardware Accelerated Scheduling...")
            run_cmd_s('reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers" /v HwSchMode /t REG_DWORD /d 2 /f', 5)
            self.L("    ✓ Готово (требуется перезагрузка)")

        # ── 11. Network ──────────────────────────────────────────
        if s.get("optimize_network"):
            self.L("\n[►] Оптимизация сети (TCP/IP, DNS, Nagle)...")
            self._batch([
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters" /v TCPNoDelay /t REG_DWORD /d 1 /f',
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters" /v TcpAckFrequency /t REG_DWORD /d 1 /f',
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters" /v CacheHashTableBucketSize /t REG_DWORD /d 1 /f',
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters" /v CacheHashTableSize /t REG_DWORD /d 384 /f',
                'netsh int tcp set global autotuninglevel=normal',
                'netsh int tcp set global dca=enabled',
                'netsh int tcp set global netdma=enabled',
                'netsh int tcp set global ecncapability=disabled',
                'netsh int tcp set global rss=enabled',
                'netsh winsock reset',
                'ipconfig /flushdns',
            ], "Сеть", timeout=15)
            self.L("    ✓ Готово")

        # ── 12. Registry cleanup ─────────────────────────────────
        if s.get("clean_registry"):
            self.L("\n[►] Очистка устаревших ключей реестра...")
            self._batch([
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs" /f 2>NUL',
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU" /f 2>NUL',
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TypedPaths" /f 2>NUL',
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist" /f 2>NUL',
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedPidlMRU" /f 2>NUL',
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSavePidlMRU" /f 2>NUL',
            ], "Реестр", timeout=5)
            self.L("    ✓ Готово")

        # ── 13. Timer / HPET ─────────────────────────────────────
        if s.get("timer_resolution"):
            self.L("\n[►] Настройка таймера прерываний (HPET/DynamicTick)...")
            self._batch([
                "bcdedit /set useplatformtick yes",
                "bcdedit /deletevalue useplatformclock 2>NUL",
                "bcdedit /set disabledynamictick yes",
            ], "Timer", timeout=8)
            self.L("    ✓ Готово (требуется перезагрузка)")

        # ── 14. Pagefile ─────────────────────────────────────────
        if s.get("optimize_pagefile"):
            self.L("\n[►] Оптимизация виртуальной памяти (pagefile)...")
            self._ps(
                "$cs = Get-WmiObject Win32_ComputerSystem; "
                "$cs.AutomaticManagedPagefile = $false; $cs.Put() | Out-Null; "
                "$pf = Get-WmiObject Win32_PageFileSetting; "
                "if ($pf) { $pf.InitialSize = 4096; $pf.MaximumSize = 8192; $pf.Put() | Out-Null }",
                timeout=20
            )
            run_cmd_s('reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management" /v ClearPageFileAtShutdown /t REG_DWORD /d 1 /f', 5)
            self.L("    ✓ Готово (требуется перезагрузка)")

        # ── 15. DNS / Winsock ────────────────────────────────────
        if s.get("dns_winsock"):
            self.L("\n[►] Сброс DNS-кэша и Winsock...")
            self._batch([
                "ipconfig /flushdns",
                "ipconfig /registerdns",
                "netsh winsock reset catalog",
                "netsh int ip reset",
            ], "DNS/Winsock", timeout=15)
            self.L("    ✓ Готово")

        # ── 16. Search indexing ──────────────────────────────────
        if s.get("disable_search_index"):
            self.L("\n[►] Отключение индексирования поиска...")
            run_cmd_s("sc stop WSearch & sc config WSearch start= disabled", 10)
            self.L("    ✓ Готово")

        # ── 17. Services ─────────────────────────────────────────
        if s.get("disable_services"):
            self.L("\n[►] Отключение лишних служб...")
            self._optimize_services()
            self.L("    ✓ Готово")

        # ── 18. Memory cleaner — safe, Windows-aware ─────────────
        if s.get("memory_clean"):
            self.L("\n[►] Очистка оперативной памяти...")
            # 1. Flush working sets of non-critical processes only
            #    We use NtSetSystemInformation with SystemMemoryListCommand
            #    via PowerShell — this is what RAMMap does internally.
            #    It flushes Modified/Standby lists without touching
            #    running processes or our own app.
            clean_script = (
                "Add-Type -TypeDefinition @'\n"
                "using System;\n"
                "using System.Runtime.InteropServices;\n"
                "public class MemClean {\n"
                "  [DllImport(\"ntdll.dll\")] public static extern uint\n"
                "  NtSetSystemInformation(int cls, ref int info, int len);\n"
                "  public static void FlushStandby() {\n"
                "    int cmd = 4; // MemoryPurgeStandbyList\n"
                "    NtSetSystemInformation(0x50, ref cmd, 4);\n"
                "    cmd = 3; // MemoryFlushModifiedList\n"
                "    NtSetSystemInformation(0x50, ref cmd, 4);\n"
                "  }\n"
                "}\n"
                "'@ -Language CSharp\n"
                "[MemClean]::FlushStandby()"
            )
            # Write to temp file to avoid inline escaping issues
            tmp = os.path.join(os.environ.get("TEMP", "C:\\Temp"), "mc_flush.ps1")
            try:
                with open(tmp, "w", encoding="utf-8") as f:
                    f.write(clean_script)
                out, ok = run_cmd(
                    f'powershell -NoProfile -ExecutionPolicy Bypass -File "{tmp}"',
                    timeout=12
                )
                if ok:
                    self.L("  ✓ Standby/Modified списки RAM очищены")
                else:
                    self.L("  ⚠ NtSetSystemInformation не поддержан — используем fallback")
                    # Fallback: ProcessIdleTasks (safe, only flushes idle queues)
                    run_cmd("rundll32.exe advapi32.dll,ProcessIdleTasks", timeout=6)
            except Exception as e:
                self.L(f"  ⚠ Fallback: {e}")
                run_cmd("rundll32.exe advapi32.dll,ProcessIdleTasks", timeout=6)
            finally:
                try:
                    os.remove(tmp)
                except Exception:
                    pass
            # 2. Force GC on our own process memory (safe, only affects this app)
            import gc; gc.collect()
            self.L("    ✓ Готово")

        # ── 19. CHKDSK ───────────────────────────────────────────
        if s.get("fix_errors"):
            self.L("\n[►] Планирование CHKDSK при перезагрузке...")
            run_cmd_s("echo Y | chkdsk C: /f", 10)
            self.L("    ✓ Запланировано при следующей загрузке")

        # ── 20. Invalid data ─────────────────────────────────────
        if s.get("clean_invalid"):
            self.L("\n[►] Очистка недопустимых данных устройств...")
            self._batch([
                'reg delete "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist" /f 2>NUL',
                "pnputil /scan-devices 2>NUL",
            ], "Данные устройств", timeout=15)
            self.L("    ✓ Готово")

        # ── 21. Drivers ──────────────────────────────────────────
        if s.get("fix_drivers"):
            self.L("\n[►] Проверка несовместимых драйверов...")
            out = run_ps(
                "Get-PnpDevice | Where-Object {$_.Status -ne 'OK'} | "
                "Select-Object Name,Status | Format-Table -AutoSize | Out-String",
                timeout=20
            )
            if out.strip() and "[TIMEOUT]" not in out:
                self.L(out)
            else:
                self.L("    Все устройства работают корректно")
            self.L("    ✓ Готово")

        # ── 22. Junk cleaner ─────────────────────────────────────
        if s.get("clean_junk"):
            self.L("\n[►] Удаление мусора...")
            # Phase 1: quick file deletes (5s each)
            self._batch([
                'del /s /f /q "%TEMP%\\*.*" 2>NUL',
                'del /s /f /q "C:\\Windows\\Temp\\*.*" 2>NUL',
                'del /s /f /q "C:\\Windows\\Prefetch\\*.*" 2>NUL',
                'reg delete "HKCU\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\Bags" /f 2>NUL',
                'reg delete "HKCU\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\BagMRU" /f 2>NUL',
                'ie4uinit.exe -ClearIconCache',
            ], "Мусор (быстрое)", timeout=5)
            # Phase 2: slower ops with generous timeouts
            self.L("  > Очистка кэша обновлений Windows...")
            run_cmd_s("net stop wuauserv 2>NUL", 10)
            run_cmd_s("net stop bits 2>NUL", 10)
            run_cmd_s('rmdir /s /q "%SystemRoot%\\SoftwareDistribution\\Download" 2>NUL', 15)
            run_cmd_s("net start wuauserv 2>NUL", 10)
            run_cmd_s("net start bits 2>NUL", 10)
            self.L("  > Очистка кэша Edge...")
            run_cmd_s('rmdir /s /q "%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\\Default\\Cache" 2>NUL', 10)
            self.L("  > WinSxS cleanup (может занять 1-2 мин)...")
            run_cmd_s("dism /online /cleanup-image /startcomponentcleanup", timeout=120)
            self.L("    ✓ Готово")

        # ── 23. Metro apps ───────────────────────────────────────
        metro = {
            "remove_3dbuilder":  ["Microsoft.3DBuilder"],
            "remove_camera":     ["Microsoft.WindowsCamera"],
            "remove_mail":       ["Microsoft.windowscommunicationsapps", "Microsoft.WindowsMaps"],
            "remove_money":      ["Microsoft.BingFinance", "Microsoft.BingSports",
                                  "Microsoft.BingNews", "Microsoft.BingWeather"],
            "remove_groove":     ["Microsoft.ZuneMusic", "Microsoft.ZuneVideo"],
            "remove_people":     ["Microsoft.People", "Microsoft.Office.OneNote"],
            "remove_phone":      ["Microsoft.WindowsPhone", "Microsoft.Windows.Photos"],
            "remove_solitaire":  ["Microsoft.MicrosoftSolitaireCollection"],
            "remove_voice":      ["Microsoft.WindowsSoundRecorder"],
            "remove_xbox":       ["Microsoft.XboxApp", "Microsoft.XboxGameOverlay",
                                  "Microsoft.XboxGamingOverlay", "Microsoft.XboxIdentityProvider",
                                  "Microsoft.Xbox.TCUI", "Microsoft.XboxSpeechToTextOverlay"],
        }
        for key, pkgs in metro.items():
            if s.get(key):
                for pkg in pkgs:
                    self.L(f"\n[►] Удаление {pkg}...")
                    out = run_ps(f"Get-AppxPackage *{pkg}* | Remove-AppxPackage", timeout=30)
                    if "[TIMEOUT]" in out:
                        self.L(f"  ✗ Timeout при удалении {pkg} — пропущено")
                    else:
                        self.L("    ✓ Готово")

        # ── 24–32. Advanced tweaks ───────────────────────────────
        if s.get("disable_dynamic_tick"):
            self.L("\n[►] Отключение Dynamic Tick...")
            run_cmd_s("bcdedit /set disabledynamictick yes", 8)
            self.L("    ✓ Готово (требуется перезагрузка)")

        if s.get("force_hpet"):
            self.L("\n[►] Принудительный HPET + TSC Sync Enhanced...")
            run_cmd_s("bcdedit /set useplatformclock true", 8)
            run_cmd_s("bcdedit /set tscsyncpolicy Enhanced", 8)
            self.L("    ✓ Готово (требуется перезагрузка)")

        if s.get("disable_nagle"):
            self.L("\n[►] Отключение алгоритма Nagle на всех интерфейсах...")
            self._ps(
                "$path = 'HKLM:\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces'; "
                "Get-ChildItem $path | ForEach-Object { "
                "  Set-ItemProperty -Path $_.PSPath -Name 'TcpAckFrequency' -Value 1 -Type DWord -Force -EA SilentlyContinue; "
                "  Set-ItemProperty -Path $_.PSPath -Name 'TCPNoDelay' -Value 1 -Type DWord -Force -EA SilentlyContinue }",
                timeout=15
            )
            self.L("    ✓ Готово")

        if s.get("disable_mouse_accel"):
            self.L("\n[►] Отключение ускорения мыши...")
            self._batch([
                'reg add "HKCU\\Control Panel\\Mouse" /v MouseSpeed /t REG_SZ /d 0 /f',
                'reg add "HKCU\\Control Panel\\Mouse" /v MouseThreshold1 /t REG_SZ /d 0 /f',
                'reg add "HKCU\\Control Panel\\Mouse" /v MouseThreshold2 /t REG_SZ /d 0 /f',
                'reg add "HKCU\\Control Panel\\Mouse" /v MouseSensitivity /t REG_SZ /d 10 /f',
            ], "Mouse", timeout=5)
            self.L("    ✓ Готово")

        if s.get("processor_idle_disable"):
            self.L("\n[►] Processor Idle Disable — агрессивный режим CPU...")
            run_cmd_s("powercfg -attributes SUB_PROCESSOR IDLEDISABLE -ATTRIB_HIDE", 8)
            run_cmd_s("powercfg -setacvalueindex scheme_current SUB_PROCESSOR IDLEDISABLE 1", 8)
            run_cmd_s("powercfg -setactive scheme_current", 8)
            self.L("    ✓ Готово")

        if s.get("system_responsiveness"):
            self.L("\n[►] System Responsiveness = 0 (приоритет игровых задач)...")
            self._batch([
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile" /v SystemResponsiveness /t REG_DWORD /d 0 /f',
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\Games" /v "GPU Priority" /t REG_DWORD /d 8 /f',
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\Games" /v "Priority" /t REG_DWORD /d 6 /f',
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\Games" /v "Scheduling Category" /t REG_SZ /d "High" /f',
            ], "Responsiveness", timeout=5)
            self.L("    ✓ Готово")

        if s.get("network_throttling"):
            self.L("\n[►] Убрать Network Throttling Index...")
            run_cmd_s(
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile" '
                '/v NetworkThrottlingIndex /t REG_DWORD /d 4294967295 /f', 5
            )
            self.L("    ✓ Готово")

        if s.get("disable_core_parking"):
            self.L("\n[►] Отключение Core Parking (все ядра активны)...")
            run_cmd_s("powercfg -attributes SUB_PROCESSOR CPMINCORES -ATTRIB_HIDE", 8)
            run_cmd_s("powercfg -setacvalueindex scheme_current SUB_PROCESSOR CPMINCORES 100", 8)
            run_cmd_s("powercfg -setactive scheme_current", 8)
            self.L("    ✓ Готово")

        if s.get("network_stack_boost"):
            self.L("\n[►] Network Stack Boost...")
            self._batch([
                "netsh int tcp set global autotuninglevel=normal",
                "netsh int tcp set global rss=enabled",
                "netsh int tcp set global chimney=enabled",
                "netsh int tcp set global dca=enabled",
                "netsh int tcp set global netdma=enabled",
                "netsh int tcp set global ecncapability=disabled",
                "netsh int tcp set heuristics disabled",
            ], "NetStack", timeout=10)
            self.L("    ✓ Готово")

        # ── 33. CPU Scheduling Optimization (Win11) ──────────────
        if s.get("cpu_scheduling"):
            self.L("\n[►] CPU Scheduling Optimization (Windows 11)...")
            run_cmd_s(
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile" '
                '/v SchedulingCategory /t REG_DWORD /d 0 /f', 5
            )
            # Also set Win32PrioritySeparation for best foreground responsiveness
            run_cmd_s(
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\PriorityControl" '
                '/v Win32PrioritySeparation /t REG_DWORD /d 38 /f', 5
            )
            self.L("    ✓ Готово (требуется перезагрузка)")

        # ── 34. Remove Power Throttling ───────────────────────────
        if s.get("disable_power_throttling"):
            self.L("\n[►] Отключение Power Throttling для всех процессов...")
            run_cmd_s("powercfg /SETACVALUEINDEX scheme_current SUB_PROCESSOR PROCTHROTTLEMAX 0", 8)
            run_cmd_s("powercfg /SETACTIVE scheme_current", 8)
            # Disable via registry (persistent across reboots)
            run_cmd_s(
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerThrottling" '
                '/v PowerThrottlingOff /t REG_DWORD /d 1 /f', 5
            )
            self.L("    ✓ Готово")

        # ── 35. Real-Time priority for foreground app ─────────────
        if s.get("realtime_boost"):
            self.L("\n[►] Real-Time Thread Boost для активного приложения...")
            # Set foreground boost via registry (safe — Windows caps it internally)
            run_cmd_s(
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile" '
                '/v SystemResponsiveness /t REG_DWORD /d 0 /f', 5
            )
            # Foreground app gets maximum quantum boost
            run_cmd_s(
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\PriorityControl" '
                '/v Win32PrioritySeparation /t REG_DWORD /d 0x26 /f', 5
            )
            # ProcessIdleTasks to yield background queue
            run_cmd("rundll32.exe advapi32.dll,ProcessIdleTasks", timeout=5)
            self.L("    ✓ Готово")

        # ── 36. Lock power scheme (anti-throttle) ────────────────
        if s.get("lock_power_scheme"):
            self.L("\n[►] Lock System-Wide Power Scheme (защита от сброса питания)...")
            # Lock Ultimate Performance / High Performance plan
            run_cmd_s("powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c 2>NUL", 8)
            run_cmd_s("powercfg -attributes SUB_PROCESSOR IDLEDISABLE -ATTRIB_HIDE", 8)
            run_cmd_s("powercfg -setacvalueindex scheme_current SUB_PROCESSOR IDLEDISABLE 1", 8)
            run_cmd_s("powercfg -setactive scheme_current", 8)
            # Prevent power plan reset by disabling power policy manager changes
            run_cmd_s(
                'reg add "HKLM\\SOFTWARE\\Policies\\Microsoft\\Power\\PowerSettings" '
                '/v ActivePowerScheme /t REG_SZ /d "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c" /f', 5
            )
            self.L("    ✓ Готово")

        # ── 37. Network Packet Optimization (CTCP) ────────────────
        if s.get("network_packet_opt"):
            self.L("\n[►] Network Packet Optimization (CTCP, без Nagle)...")
            self._batch([
                "netsh interface tcp set global congestionprovider=ctcp",
                "netsh interface tcp set global ecncapability=disabled",
                "netsh interface tcp set global rss=enabled",
                "netsh interface tcp set global autotuninglevel=normal",
            ], "CTCP", timeout=10)
            self.L("    ✓ Готово")

        # ── 38. Registry Reactivity Boost ────────────────────────
        if s.get("registry_reactivity"):
            self.L("\n[►] Registry Reactivity Boost (IoPageLockLimit)...")
            self._batch([
                # IoPageLockLimit = max → allows Windows to lock more I/O pages
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management" '
                '/v IoPageLockLimit /t REG_DWORD /d 0xFFFFFFFF /f',
                # LargePageMinimum — enables large page support if available
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management" '
                '/v LargeSystemCache /t REG_DWORD /d 0 /f',
                # Disable paging of kernel and drivers
                'reg add "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management" '
                '/v DisablePagingExecutive /t REG_DWORD /d 1 /f',
            ], "Reactivity", timeout=5)
            self.L("    ✓ Готово (требуется перезагрузка)")

        # ── FINAL ────────────────────────────────────────────────
        self.L("\n" + "═" * 64)
        self.L("  Финальная проверка целостности системы")
        self.L("═" * 64)

        # SFC /scannow — stream with % progress
        self.L("\n[►] SFC /scannow ...")

        import re as _re
        _last_sfc_pct = [0]

        def _parse_sfc(line):
            # SFC outputs: "Windows Resource Protection is scanning..." etc.
            # Progress line looks like: "... 100% complete."  or percent numbers
            m = _re.search(r'(\d{1,3})\s*%', line)
            if m:
                pct = int(m.group(1))
                if pct != _last_sfc_pct[0]:
                    _last_sfc_pct[0] = pct
                    bar = "█" * (pct // 5) + "░" * (20 - pct // 5)
                    return f"  SFC [{bar}] {pct}%"
            # Show key status lines, skip garbage
            low = line.lower()
            if any(k in low for k in ["scanning", "complete", "found", "repaired",
                                       "no integrity", "protection", "corrupt"]):
                # Strip non-ASCII artifacts that come from Windows console
                clean = _re.sub(r'[^\x20-\x7E]', '', line).strip()
                if clean:
                    return f"  {clean}"
            return None

        ok_sfc = run_process_streaming("sfc /scannow", self.L, _parse_sfc, timeout=600)
        self.L(f"  {'✓ SFC завершён' if ok_sfc else '⚠ SFC завершился с ошибками'}")

        # DISM /RestoreHealth — stream with % progress
        self.L("\n[►] DISM /Online /Cleanup-Image /RestoreHealth ...")

        _last_dism_pct = [0]

        def _parse_dism(line):
            m = _re.search(r'(\d{1,3})[,.]?\d*\s*%', line)
            if m:
                pct = int(m.group(1))
                if pct != _last_dism_pct[0]:
                    _last_dism_pct[0] = pct
                    bar = "█" * (pct // 5) + "░" * (20 - pct // 5)
                    return f"  DISM [{bar}] {pct}%"
            low = line.lower()
            if any(k in low for k in ["restoring", "scanning", "complete", "error",
                                       "operation", "component", "download", "progress"]):
                clean = _re.sub(r'[^\x20-\x7E]', '', line).strip()
                if clean:
                    return f"  {clean}"
            return None

        ok_dism = run_process_streaming(
            "DISM /Online /Cleanup-Image /RestoreHealth",
            self.L, _parse_dism, timeout=1200
        )
        self.L(f"  {'✓ DISM завершён' if ok_dism else '⚠ DISM завершился с предупреждениями'}")

        self.L("\n" + "═" * 64)
        self.L("  ✅  Оптимизация завершена успешно!")
        self.L("  Рекомендуется перезагрузить компьютер.")
        self.L("═" * 64)
        self.finished.emit()

    def _optimize_services(self):
        WHITELIST   = ["nvidia","amd","intel","cheat","vpn","faceit",
                       "realtek","steelseries","logitech","corsair","razer","asus","msi"]
        MSFT_GUARD  = ["microsoft","windows","wlan","lan","audio",
                       "rpc","dcom","plug","pnp","power","security"]
        force_off   = ["DiagTrack","dmwappushservice","WMPNetworkSvc","RemoteRegistry",
                       "Fax","TabletInputService","WerSvc","PcaSvc","wercplsupport",
                       "MapsBroker","lfsvc","RetailDemo","TrkWks","WalletService",
                       "PhoneSvc","SmsRouter","SharedAccess"]
        for svc in force_off:
            run_cmd_s(f"sc stop {svc} 2>NUL", 8)
            run_cmd_s(f"sc config {svc} start= disabled 2>NUL", 5)
            self.L(f"  [OFF] {svc}")

        out = run_ps(
            "Get-Service | Where-Object {$_.StartType -eq 'Automatic' -and $_.Status -ne 'Running'} "
            "| Select-Object -ExpandProperty Name", timeout=20
        )
        if "[TIMEOUT]" in out:
            self.L("  ✗ Не удалось получить список служб (timeout)")
            return
        errors = 0
        for svc in out.splitlines():
            svc = svc.strip()
            if not svc:
                continue
            low = svc.lower()
            if any(w in low for w in WHITELIST) or any(w in low for w in MSFT_GUARD):
                continue
            _, ok = run_cmd(f"sc config {svc} start= demand 2>NUL", 5)
            if ok:
                self.L(f"  [DEMAND] {svc}")
                errors = 0
            else:
                errors += 1
                if errors >= 10:
                    self.L("  ⚠  Слишком много ошибок служб, прерываем")
                    break

# ══════════════════════════════════════════════════════════════════
#  CONSOLE WINDOW
# ══════════════════════════════════════════════════════════════════

class CmdWindow(QWidget):
    def __init__(self, settings):
        super().__init__()
        self.setObjectName("consoleRoot")
        self.setWindowTitle("minoreOptimizer — Консоль")
        self.resize(860, 580)
        self.setStyleSheet(STYLE)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        hdr = QWidget()
        hdr.setStyleSheet("background:#111; border-bottom:1px solid #1e1e1e;")
        hl  = QHBoxLayout(hdr)
        hl.setContentsMargins(16, 10, 16, 10)
        dot = QLabel("●"); dot.setStyleSheet("color:#4edd4e; font-size:14px;")
        lbl = QLabel("  minoreOptimizer — процесс оптимизации")
        lbl.setStyleSheet("color:#4edd4e; font-size:13px; font-weight:600;")
        hl.addWidget(dot); hl.addWidget(lbl); hl.addStretch()
        layout.addWidget(hdr)

        self.console = QTextEdit()
        self.console.setObjectName("console")
        self.console.setReadOnly(True)
        self.console.setLineWrapMode(QTextEdit.LineWrapMode.NoWrap)
        layout.addWidget(self.console)

        btn_bar = QWidget()
        btn_bar.setStyleSheet("background:#111; border-top:1px solid #1e1e1e;")
        bl = QHBoxLayout(btn_bar)
        bl.setContentsMargins(16, 12, 16, 12); bl.setSpacing(10)
        self.btn_reboot = QPushButton("🔄  Перезагрузить компьютер")
        self.btn_reboot.setObjectName("btnReboot")
        self.btn_reboot.setEnabled(False)
        self.btn_reboot.clicked.connect(self._reboot)
        btn_close = QPushButton("✕  Закрыть")
        btn_close.setObjectName("btnCloseConsole")
        btn_close.clicked.connect(self.close)
        bl.addStretch(); bl.addWidget(self.btn_reboot); bl.addWidget(btn_close)
        layout.addWidget(btn_bar)

        self.worker = OptimizeWorker(settings)
        self.worker.log.connect(self._append)
        self.worker.finished.connect(self._done)
        self.worker.start()

    def _append(self, text):
        self.console.append(text)
        sb = self.console.verticalScrollBar()
        sb.setValue(sb.maximum())

    def _done(self):
        self.btn_reboot.setEnabled(True)
        QMessageBox.information(self, "Оптимизация завершена",
            "✅  Все операции выполнены!\n\nРекомендуется перезагрузить компьютер.")

    def _reboot(self):
        run_cmd_s('shutdown /r /t 10 /c "minoreOptimizer: перезагрузка после оптимизации"', 5)
        self.close()

# ══════════════════════════════════════════════════════════════════
#  HELPERS — PNG ICONS
# ══════════════════════════════════════════════════════════════════

def _sep():
    f = QFrame(); f.setObjectName("sep"); f.setFrameShape(QFrame.Shape.HLine)
    return f

def _set_png_icon(btn, filename, fallback="", size=18):
    path = asset(filename)
    if os.path.exists(path):
        px = QPixmap(path).scaled(size, size,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation)
        btn.setIcon(QIcon(px))
        btn.setIconSize(QSize(size, size))
        text = btn.text().strip()
        for em in ["🛡","🤖","🔥","📋","⬡","⚡"]:
            if text.startswith(em):
                btn.setText("   " + text[len(em):].strip())
                break
    else:
        text = btn.text().strip()
        if fallback and not text.startswith(fallback):
            btn.setText(f"{fallback}   {text}")

def _set_icon_btn(btn, filename, fallback=""):
    _set_png_icon(btn, filename, fallback=fallback, size=18)

# ══════════════════════════════════════════════════════════════════
#  TAB 1 — MAIN
# ══════════════════════════════════════════════════════════════════

class Tab1_Main(QWidget):
    def __init__(self, get_settings_fn):
        super().__init__()
        self.get_settings = get_settings_fn
        self._build()

    def _build(self):
        lay = QVBoxLayout(self)
        lay.setContentsMargins(32, 32, 32, 32)
        lay.setSpacing(22)

        hdr = QHBoxLayout(); hdr.setSpacing(16)
        icon_lbl = QLabel(); icon_lbl.setFixedSize(52, 52)
        icon_path = asset("icon.png")
        if os.path.exists(icon_path):
            px = QPixmap(icon_path).scaled(52, 52,
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation)
            icon_lbl.setPixmap(px)
        else:
            icon_lbl.setText("⚡"); icon_lbl.setStyleSheet("font-size:40px;")
        icon_lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)

        tv = QVBoxLayout(); tv.setSpacing(4)
        t1 = QLabel("minoreOptimizer"); t1.setObjectName("titleBig")
        t2 = QLabel("Профессиональный инструмент оптимизации Windows")
        t2.setObjectName("titleSub")
        tv.addWidget(t1); tv.addWidget(t2)

        badge = QLabel("v2.0")
        badge.setStyleSheet(
            "background:#1a3a6a; color:#7ab4f5; border:1px solid #2a5aaa; "
            "border-radius:10px; padding:3px 10px; font-size:11px; font-weight:600;")

        hdr.addWidget(icon_lbl); hdr.addLayout(tv); hdr.addStretch(); hdr.addWidget(badge)
        lay.addLayout(hdr)
        lay.addWidget(_sep())

        info = QLabel(
            "Перед запуском оптимизации рекомендуется:\n"
            "   •  Создать точку восстановления системы\n"
            "   •  Закрыть все запущенные приложения\n"
            "   •  Подключить зарядное устройство (если ноутбук)\n\n"
            "Настройте нужные параметры во вкладке  ⚙ Настройки, "
            "затем нажмите «Запуск оптимизации»."
        )
        info.setObjectName("infoCard"); info.setWordWrap(True)
        lay.addWidget(info)
        lay.addStretch()

        sr = QHBoxLayout()
        admin_ok = is_admin()
        sl = QLabel(f"{'✅' if admin_ok else '⚠'}  {'Запущено от администратора' if admin_ok else 'Требуются права администратора'}")
        sl.setStyleSheet(f"color:{'#4edd4e' if admin_ok else '#e07a5f'}; font-size:12px;")
        sr.addWidget(sl); sr.addStretch()
        lay.addLayout(sr)
        lay.addWidget(_sep())

        br = QHBoxLayout(); br.setSpacing(14)
        self.btn_restore = QPushButton("   Создать точку восстановления")
        self.btn_restore.setObjectName("btnRestore")
        self.btn_restore.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        self.btn_restore.setFixedHeight(46)
        _set_png_icon(self.btn_restore, "restore.png", fallback="🛡", size=20)
        self.btn_restore.clicked.connect(self._create_restore)

        self.btn_opt = QPushButton("⚡   Запуск оптимизации")
        self.btn_opt.setObjectName("btnOptimize")
        self.btn_opt.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        self.btn_opt.setFixedHeight(46)
        self.btn_opt.clicked.connect(self._run_opt)

        br.addWidget(self.btn_restore); br.addStretch(); br.addWidget(self.btn_opt)
        lay.addLayout(br)

    def _create_restore(self):
        self.btn_restore.setEnabled(False)
        self.btn_restore.setText("⏳  Создание точки...")
        QApplication.processEvents()
        run_ps(
            "Enable-ComputerRestore -Drive 'C:\\'; "
            "Checkpoint-Computer -Description 'minoreOptimizer Restore Point' -RestorePointType MODIFY_SETTINGS",
            timeout=60
        )
        self.btn_restore.setEnabled(True)
        _set_png_icon(self.btn_restore, "restore.png", fallback="🛡", size=20)
        self.btn_restore.setText(
            "   Создать точку восстановления" if os.path.exists(asset("restore.png"))
            else "🛡   Создать точку восстановления"
        )
        QMessageBox.information(self, "Готово", "✅  Точка восстановления успешно создана!")

    def _run_opt(self):
        settings = self.get_settings()
        active = [k for k, v in settings.items() if v]
        if not active:
            QMessageBox.warning(self, "Нет настроек", "Включите хотя бы одну опцию на вкладке «Настройки».")
            return
        reply = QMessageBox.question(
            self, "Подтверждение запуска",
            f"Будет выполнено {len(active)} операций.\n\nПродолжить оптимизацию?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )
        if reply == QMessageBox.StandardButton.Yes:
            self._cmd_win = CmdWindow(settings)
            self._cmd_win.show()

# ══════════════════════════════════════════════════════════════════
#  OPTIMIZATION PROFILES
# ══════════════════════════════════════════════════════════════════

PROFILES = {
    "full": {
        "label": "☀️  Full", "subtitle": "Максимальная оптимизация",
        "obj": "profileFull", "color": "#4f9ef8",
        "desc": (
            "☀️ Full Optimization — всё включено. Максимальная производительность для игрового ПК. "
            "Отключает телеметрию, службы, Xbox, Defender, очищает мусор, применяет все CPU/GPU/сетевые твики. "
            "Рекомендуется создать точку восстановления перед запуском."
        ),
        "keys": [
            "perf_preset","gpu_hw_scheduling","timer_resolution","optimize_pagefile",
            "disable_xbox_gamebar","disable_telemetry","disable_privacy",
            "disable_telemetry_tasks","disable_spy_tasks",
            "disable_defender","remove_onedrive","set_photo_viewer",
            "optimize_network","dns_winsock","network_stack_boost","network_throttling",
            "network_packet_opt","disable_services","disable_search_index",
            "memory_clean","clean_registry","clean_junk",
            "disable_dynamic_tick","force_hpet","disable_nagle",
            "disable_mouse_accel","processor_idle_disable",
            "system_responsiveness","disable_core_parking",
            "cpu_scheduling","disable_power_throttling","realtime_boost",
            "lock_power_scheme","registry_reactivity",
        ],
    },
    "medium": {
        "label": "⚡  Medium", "subtitle": "Баланс производительности",
        "obj": "profileMedium", "color": "#4edd4e",
        "desc": (
            "⚡ Medium Optimization — баланс между скоростью и стабильностью. "
            "Отключает телеметрию, ненужные службы и задачи, применяет сетевые и CPU твики. "
            "Defender и OneDrive не трогает. Подходит для большинства пользователей."
        ),
        "keys": [
            "perf_preset","gpu_hw_scheduling","optimize_pagefile",
            "disable_xbox_gamebar","disable_telemetry","disable_privacy",
            "disable_telemetry_tasks","disable_spy_tasks",
            "set_photo_viewer","optimize_network","dns_winsock",
            "network_stack_boost","network_throttling","network_packet_opt",
            "disable_services","disable_search_index",
            "memory_clean","clean_registry","clean_junk",
            "disable_nagle","disable_mouse_accel",
            "system_responsiveness","disable_core_parking",
            "cpu_scheduling","disable_power_throttling",
        ],
    },
    "light": {
        "label": "✨  Light", "subtitle": "Лёгкая очистка",
        "obj": "profileLight", "color": "#f0c040",
        "desc": (
            "✨ Light Optimization — безопасная лёгкая очистка без радикальных изменений. "
            "Очищает мусор, сбрасывает DNS, отключает телеметрию и рекламу. "
            "Не меняет системные службы. Идеально для первого раза."
        ),
        "keys": [
            "disable_telemetry","disable_privacy","disable_telemetry_tasks",
            "dns_winsock","memory_clean","clean_junk","clean_registry",
            "set_photo_viewer","disable_mouse_accel",
        ],
    },
    "laptop": {
        "label": "💻  Laptop", "subtitle": "Оптимизация ноутбука",
        "obj": "profileLaptop", "color": "#c07af0",
        "desc": (
            "💻 Laptop Optimization — специально для ноутбуков. "
            "Отключает телеметрию и фоновые задачи, оптимизирует сеть. "
            "НЕ отключает управление питанием и Core Parking — важны для батареи. "
            "НЕ включает Processor Idle Disable — убивает аккумулятор."
        ),
        "keys": [
            "disable_telemetry","disable_privacy","disable_telemetry_tasks",
            "disable_spy_tasks","set_photo_viewer",
            "optimize_network","dns_winsock","network_throttling",
            "disable_search_index","memory_clean",
            "clean_registry","clean_junk","disable_mouse_accel","system_responsiveness",
        ],
    },
}

# ══════════════════════════════════════════════════════════════════
#  TAB 2 — SETTINGS
# ══════════════════════════════════════════════════════════════════

class Tab2_Settings(QWidget):
    def __init__(self):
        super().__init__()
        self.checks: dict[str, QCheckBox] = {}
        self._profile_btns: dict[str, QPushButton] = {}
        self._active_profile: str | None = None
        self._build()

    def _cb(self, key, label, layout):
        cb = QCheckBox(label); cb.setChecked(False)
        layout.addWidget(cb); self.checks[key] = cb

    def _group(self, title):
        g = QGroupBox(title); gl = QVBoxLayout(g); gl.setSpacing(4)
        return g, gl

    def _build(self):
        outer = QVBoxLayout(self)
        outer.setContentsMargins(0, 0, 0, 0)
        outer.setSpacing(0)

        # ── Profile panel ────────────────────────────────────────
        pw = QWidget()
        pw.setStyleSheet("background:#161616; border-bottom:1px solid #2a2a2a;")
        pv = QVBoxLayout(pw)
        pv.setContentsMargins(18, 14, 18, 14); pv.setSpacing(10)

        pt = QLabel("📚  Профили оптимизации")
        pt.setStyleSheet("color:#9ab4d8; font-size:12px; font-weight:600; "
                         "letter-spacing:0.5px; background:transparent;")
        pv.addWidget(pt)

        pbr = QHBoxLayout(); pbr.setSpacing(8)
        for pid, pd in PROFILES.items():
            btn = QPushButton(f"{pd['label']}\n{pd['subtitle']}")
            btn.setObjectName(pd["obj"])
            btn.setProperty("active", "false")
            btn.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
            btn.setFixedHeight(56)
            btn.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
            btn.setStyleSheet(
                f"QPushButton#{pd['obj']} {{ background:#242424; color:#777; border:1px solid #333; "
                f"border-radius:8px; font-size:12px; font-weight:500; padding:6px 4px; }}"
                f"QPushButton#{pd['obj']}:hover {{ background:#2c2c2c; border-color:#555; color:#bbb; }}"
                f"QPushButton#{pd['obj']}[active='true'] {{ background:#1a1a2e; "
                f"border:2px solid {pd['color']}; color:{pd['color']}; font-weight:700; }}"
            )
            btn.clicked.connect(lambda checked, p=pid: self._apply_profile(p))
            self._profile_btns[pid] = btn
            pbr.addWidget(btn)
        pv.addLayout(pbr)

        self._desc = QLabel("Выберите профиль для автоматической настройки галочек")
        self._desc.setObjectName("profileDesc")
        self._desc.setWordWrap(True)
        self._desc.setMinimumHeight(44)
        pv.addWidget(self._desc)
        outer.addWidget(pw)

        # ── Scrollable checkboxes ────────────────────────────────
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        outer.addWidget(scroll)

        w = QWidget(); scroll.setWidget(w)
        lay = QVBoxLayout(w)
        lay.setContentsMargins(20, 18, 20, 24); lay.setSpacing(10)

        g, gl = self._group("⚡  Производительность")
        self._cb("perf_preset",             "Максимальный план электропитания (Ultimate Performance)", gl)
        self._cb("gpu_hw_scheduling",       "GPU Hardware Scheduling — приоритет GPU для игр", gl)
        self._cb("timer_resolution",        "HPET / Timer Resolution — точность таймера 0.5 ms", gl)
        self._cb("optimize_pagefile",       "Оптимизация pagefile (4–8 GB, очистка при выключении)", gl)
        self._cb("disable_xbox_gamebar",    "Отключить Xbox Game Bar и DVR (снижает input lag)", gl)
        self._cb("processor_idle_disable",  "Processor Idle Disable — агрессивный режим CPU", gl)
        self._cb("disable_core_parking",    "Отключить Core Parking — все ядра CPU всегда активны", gl)
        self._cb("system_responsiveness",   "System Responsiveness = 0 (приоритет игровых задач + GPU)", gl)
        self._cb("disable_dynamic_tick",    "Отключить Dynamic Tick (bcdedit — стабильность таймера)", gl)
        self._cb("force_hpet",              "Принудительный HPET + TSC Sync Enhanced (bcdedit)", gl)
        self._cb("disable_mouse_accel",     "Полное отключение ускорения мыши (pointer precision)", gl)
        self._cb("cpu_scheduling",          "CPU Scheduling Optimization — Win32PrioritySeparation (Win 11)", gl)
        self._cb("disable_power_throttling","Отключить Power Throttling — процессоры не замедляются", gl)
        self._cb("realtime_boost",          "Real-Time Thread Boost — максимальный квант активного окна", gl)
        self._cb("lock_power_scheme",       "Lock Power Scheme — защита плана питания от сброса", gl)
        self._cb("registry_reactivity",     "Registry Reactivity Boost — IoPageLockLimit + DisablePagingExecutive", gl)
        lay.addWidget(g)

        g, gl = self._group("🔒  Конфиденциальность и телеметрия")
        self._cb("disable_telemetry",       "Отключить кейлоггер и телеметрию", gl)
        self._cb("disable_privacy",         "Отключить приватные настройки (реклама, профилирование)", gl)
        self._cb("disable_telemetry_tasks", "Отключить задачи: телеметрия, Copilot, Windows AI, Recall", gl)
        self._cb("disable_spy_tasks",       "Отключить шпионские задачи планировщика", gl)
        lay.addWidget(g)

        g, gl = self._group("🛠  Система")
        self._cb("disable_defender", "Отключить Windows Defender", gl)
        self._cb("remove_onedrive",  "Удалить OneDrive", gl)
        self._cb("set_photo_viewer", "Установить классический просмотрщик фото", gl)
        lay.addWidget(g)

        g, gl = self._group("🌐  Сеть")
        self._cb("optimize_network",    "Оптимизация TCP/IP, DNS-буферов, Nagle, автонастройка", gl)
        self._cb("disable_nagle",       "Отключить алгоритм Nagle на каждом интерфейсе (per-adapter)", gl)
        self._cb("network_stack_boost", "Network Stack Boost (RSS, DCA, Chimney, NetDMA)", gl)
        self._cb("network_throttling",  "Убрать Network Throttling Index (макс. пропускная способность)", gl)
        self._cb("network_packet_opt",  "Network Packet Optimization — CTCP + ECN disabled", gl)
        self._cb("dns_winsock",         "Сброс DNS-кэша и Winsock", gl)
        lay.addWidget(g)

        g, gl = self._group("⚙  Службы и автозагрузка")
        self._cb("disable_services",     "Отключить лишние службы (защита: Nvidia/AMD/Intel/Cheat/AC/VPN/Microsoft/FACEIT)", gl)
        self._cb("disable_search_index", "Отключить индексирование поиска (ускоряет диск)", gl)
        lay.addWidget(g)

        g, gl = self._group("🧹  Обслуживание системы")
        self._cb("memory_clean",   "Memory Cleaner — разово очистить ОЗУ", gl)
        self._cb("fix_errors",     "Исправление ошибок диска (CHKDSK /f)", gl)
        self._cb("clean_invalid",  "Очистка недопустимых данных устройств", gl)
        self._cb("fix_drivers",    "Проверить несовместимые драйверы", gl)
        self._cb("clean_registry", "Очистка реестра (MRU, UserAssist, ShellBags)", gl)
        self._cb("clean_junk",     "Удаление мусора (кэш обновлений, Edge, Store, WinSxS, Temp, Prefetch)", gl)
        lay.addWidget(g)

        g, gl = self._group("📦  Удаление Metro-приложений (Win 10 / 11)")
        self._cb("remove_3dbuilder", "Удалить 3D Builder", gl)
        self._cb("remove_camera",    "Удалить Camera", gl)
        self._cb("remove_mail",      "Удалить Mail, Calendar, Maps", gl)
        self._cb("remove_money",     "Удалить Money, Sports, News, Weather", gl)
        self._cb("remove_groove",    "Удалить Groove Music, Film & TV", gl)
        self._cb("remove_people",    "Удалить People, OneNote", gl)
        self._cb("remove_phone",     "Удалить Phone Companion, Photos", gl)
        self._cb("remove_solitaire", "Удалить Solitaire Collection", gl)
        self._cb("remove_voice",     "Удалить Voice Recorder", gl)
        self._cb("remove_xbox",      "Удалить XBOX (все компоненты)", gl)
        warn = QLabel("⚠   Некоторые METRO приложения удаляются навсегда и не могут быть восстановлены.")
        warn.setObjectName("warnLabel"); warn.setWordWrap(True)
        gl.addWidget(warn)
        lay.addWidget(g)
        lay.addStretch()

    def _apply_profile(self, pid):
        pd = PROFILES[pid]
        if self._active_profile == pid:
            self._active_profile = None
            for cb in self.checks.values(): cb.setChecked(False)
            for btn in self._profile_btns.values():
                btn.setProperty("active", "false")
                btn.style().unpolish(btn); btn.style().polish(btn)
            self._desc.setText("Выберите профиль для автоматической настройки галочек")
            return
        self._active_profile = pid
        for cb in self.checks.values(): cb.setChecked(False)
        for key in pd["keys"]:
            if key in self.checks: self.checks[key].setChecked(True)
        for p2, btn in self._profile_btns.items():
            btn.setProperty("active", "true" if p2 == pid else "false")
            btn.style().unpolish(btn); btn.style().polish(btn)
        self._desc.setText(pd["desc"])

    def get_settings(self):
        return {k: cb.isChecked() for k, cb in self.checks.items()}

# ══════════════════════════════════════════════════════════════════
#  TAB 3 — SYSTEM INFO
# ══════════════════════════════════════════════════════════════════

class Tab3_SysInfo(QWidget):
    def __init__(self):
        super().__init__()
        self.sys_info = {}
        self._build()
        threading.Thread(target=self._load, daemon=True).start()

    def _build(self):
        lay = QVBoxLayout(self)
        lay.setContentsMargins(32, 32, 32, 32); lay.setSpacing(20)

        t = QLabel("💻  Информация о системе"); t.setObjectName("titleBig")
        lay.addWidget(t); lay.addWidget(_sep())

        self.card = QLabel("⏳  Загрузка информации о системе...")
        self.card.setObjectName("sysCard"); self.card.setWordWrap(True)
        self.card.setTextFormat(Qt.TextFormat.RichText)
        self.card.setAlignment(Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        self.card.setMinimumHeight(140)
        lay.addWidget(self.card)
        lay.addStretch(); lay.addWidget(_sep())

        br = QHBoxLayout(); br.setSpacing(10)

        btn_copy = QPushButton("  Копировать")
        btn_copy.setObjectName("btnCopy")
        btn_copy.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        btn_copy.setFixedHeight(38)
        _set_icon_btn(btn_copy, "copy.png", fallback="📋")
        btn_copy.clicked.connect(self._copy)

        btn_ds = QPushButton("  DeepSeek")
        btn_ds.setObjectName("btnDeepSeek")
        btn_ds.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        btn_ds.setFixedHeight(38)
        _set_icon_btn(btn_ds, "deepsek.png", fallback="🤖")
        btn_ds.clicked.connect(lambda: webbrowser.open("https://chat.deepseek.com/"))

        btn_msi = QPushButton("  MSI Afterburner")
        btn_msi.setObjectName("btnMSI")
        btn_msi.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        btn_msi.setFixedHeight(38)
        _set_icon_btn(btn_msi, "msi.png", fallback="🔥")
        btn_msi.clicked.connect(self._launch_msi)

        br.addWidget(btn_copy); br.addWidget(btn_ds); br.addWidget(btn_msi); br.addStretch()
        lay.addLayout(br); lay.addWidget(_sep())

        gh_r = QHBoxLayout(); gh_r.addStretch()
        btn_gh = QPushButton(); btn_gh.setObjectName("btnGithub")
        btn_gh.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        btn_gh.setFixedHeight(34)
        gh_icon = asset("github.png")
        if os.path.exists(gh_icon):
            px = QPixmap(gh_icon).scaled(16, 16,
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation)
            btn_gh.setIcon(QIcon(px)); btn_gh.setIconSize(QSize(16, 16))
            btn_gh.setText("  github.com/bpm500")
        else:
            btn_gh.setText("⬡  github.com/bpm500")
        btn_gh.clicked.connect(lambda: webbrowser.open("https://github.com/bpm500"))
        gh_r.addWidget(btn_gh); lay.addLayout(gh_r)

    def _load(self):
        info = get_system_info()
        self.sys_info = info
        cpu     = info.get("cpu", "?")
        gpu_raw = info.get("gpu", "?")
        ram     = info.get("ram", "?")
        gpu_lines = [g.strip() for g in gpu_raw.split("\n") if g.strip()]

        def row(label, value):
            return (
                f"<tr>"
                f"<td style='color:#7ab4f5;font-weight:600;white-space:nowrap;"
                f"padding-right:20px;vertical-align:top;'>{label}</td>"
                f"<td style='color:#c8c8c8;vertical-align:top;'>{value}</td>"
                f"</tr><tr><td colspan='2' style='height:10px;'></td></tr>"
            )

        gpu_rows = "".join(row("GPU", g) for g in gpu_lines) if gpu_lines else row("GPU", "Не определено")
        html = (
            f"<table cellspacing='0' cellpadding='0' style='font-size:14px;'>"
            f"{row('CPU', cpu)}{gpu_rows}{row('Memory', ram)}</table>"
        )
        self.card.setText(html)

    def _copy(self):
        gpu_list = self.sys_info.get("gpu_list", [])
        gpu_str  = (" | ".join(f"{n} {v:.1f} GB" if v > 0.1 else n for n, v in gpu_list)
                    if gpu_list else self.sys_info.get("gpu", "?").replace("\n", " | "))
        specs = (
            f"CPU: {self.sys_info.get('cpu','?')}\n"
            f"GPU: {gpu_str}\n"
            f"Memory: {self.sys_info.get('ram','?')}"
        )
        text = (
            "Я хочу сделать средний разгон своей видеокарты который не навредит ей "
            "но прибавит производительности в играх через MSI Afterburner. "
            f"Мои характеристики:\n{specs}"
        )
        QApplication.clipboard().setText(text)
        self._toast = ToastNotification(self.window(), "Ваши характеристики скопированы")

    def _launch_msi(self):
        exe = os.path.join(BASE_DIR, "MSI", "MSIAfterburner.exe")
        if os.path.exists(exe):
            subprocess.Popen([exe])
        else:
            QMessageBox.warning(self, "MSI Afterburner",
                f"Файл не найден:\n{exe}\n\n"
                "Создайте папку MSI рядом с программой\n"
                "и поместите туда MSIAfterburner.exe")

# ══════════════════════════════════════════════════════════════════
#  FAST UTILITY WORKER
# ══════════════════════════════════════════════════════════════════

class FastUtilityWorker(QThread):
    log      = pyqtSignal(str)
    finished = pyqtSignal()

    def __init__(self, settings: dict, tweak_colors: dict):
        super().__init__()
        self.s  = settings
        self.tc = tweak_colors   # {"mouse_sel": QColor, "text_sel": QColor}

    def L(self, text):
        self.log.emit(text)

    def run(self):
        s = self.s
        self.L("=" * 60)
        self.L("  minoreOptimizer — Fast Utility")
        self.L("=" * 60)

        # ── RAM cleaner (standby flush — instant, no reboot) ─────
        if s.get("fu_ram_clean"):
            self.L("\n[►] Очистка RAM (Standby/Modified lists)...")
            import gc
            clean_script = (
                "Add-Type -TypeDefinition @'\n"
                "using System; using System.Runtime.InteropServices;\n"
                "public class MemClean {\n"
                "  [DllImport(\"ntdll.dll\")] public static extern uint\n"
                "  NtSetSystemInformation(int cls, ref int info, int len);\n"
                "  public static void Flush() {\n"
                "    int c=4; NtSetSystemInformation(0x50,ref c,4);\n"
                "    c=3;     NtSetSystemInformation(0x50,ref c,4); }\n"
                "}\n'@ -Language CSharp\n[MemClean]::Flush()"
            )
            tmp = os.path.join(os.environ.get("TEMP","C:\\Temp"), "_fu_ram.ps1")
            try:
                with open(tmp, "w", encoding="utf-8") as f:
                    f.write(clean_script)
                _, ok = run_cmd(f'powershell -NoProfile -ExecutionPolicy Bypass -File "{tmp}"', 12)
                self.L("  ✓ Standby/Modified RAM очищены" if ok else "  ⚠ API недоступен, используем fallback")
                if not ok:
                    run_cmd("rundll32.exe advapi32.dll,ProcessIdleTasks", 6)
            finally:
                try: os.remove(tmp)
                except: pass
            gc.collect()
            self.L("    ✓ Готово")

        # ── Flush DNS (instant) ───────────────────────────────────
        if s.get("fu_flush_dns"):
            self.L("\n[►] Сброс DNS-кэша...")
            run_cmd_s("ipconfig /flushdns", 10)
            run_cmd_s("ipconfig /registerdns", 10)
            self.L("    ✓ Готово")

        # ── Kill bloat processes (instant) ────────────────────────
        if s.get("fu_kill_bloat"):
            self.L("\n[►] Завершение фоновых bloatware процессов...")
            bloat = [
                "OneDrive.exe", "SkypeApp.exe", "YourPhone.exe",
                "SearchApp.exe", "SearchIndexer.exe", "SearchProtocolHost.exe",
                "MicrosoftEdgeUpdate.exe", "WaasMedic.exe", "WerFault.exe",
                "wsappx.exe", "GameBarFTServer.exe", "GameBarPresenceWriter.exe",
                "XboxPcApp.exe", "Microsoft.Photos.exe",
            ]
            killed = 0
            for proc in bloat:
                _, ok = run_cmd(f"taskkill /f /im {proc} 2>NUL", 4)
                if ok:
                    self.L(f"  [KILL] {proc}")
                    killed += 1
            self.L(f"    ✓ Завершено {killed} процессов")

        # ── Restart Explorer (instant visual refresh) ─────────────
        if s.get("fu_restart_explorer"):
            self.L("\n[►] Перезапуск Explorer...")
            run_cmd_s("taskkill /f /im explorer.exe 2>NUL", 5)
            time.sleep(1)
            run_cmd_s("start explorer.exe", 5)
            self.L("    ✓ Готово")

        # ── Flush icon cache ──────────────────────────────────────
        if s.get("fu_icon_cache"):
            self.L("\n[►] Сброс кэша иконок...")
            run_cmd_s("taskkill /f /im explorer.exe 2>NUL", 5)
            time.sleep(0.5)
            run_cmd_s('del /f /q "%LOCALAPPDATA%\\Microsoft\\Windows\\Explorer\\iconcache*.db" 2>NUL', 5)
            run_cmd_s('del /f /q "%LOCALAPPDATA%\\IconCache.db" 2>NUL', 5)
            run_cmd_s("ie4uinit.exe -ClearIconCache", 5)
            time.sleep(0.5)
            run_cmd_s("start explorer.exe", 4)
            self.L("    ✓ Готово")

        # ── Flush thumbnail cache ─────────────────────────────────
        if s.get("fu_thumb_cache"):
            self.L("\n[►] Очистка кэша эскизов (thumbnails)...")
            run_cmd_s('taskkill /f /im explorer.exe 2>NUL', 5)
            time.sleep(0.5)
            run_cmd_s('del /f /q "%LOCALAPPDATA%\\Microsoft\\Windows\\Explorer\\thumbcache_*.db" 2>NUL', 8)
            time.sleep(0.5)
            run_cmd_s("start explorer.exe", 4)
            self.L("    ✓ Готово")

        # ── Temp files cleanup (instant) ──────────────────────────
        if s.get("fu_temp_clean"):
            self.L("\n[►] Быстрая очистка временных файлов...")
            safe_run_batch([
                'del /s /f /q "%TEMP%\\*.*" 2>NUL',
                'del /s /f /q "C:\\Windows\\Temp\\*.*" 2>NUL',
                'del /s /f /q "C:\\Windows\\Prefetch\\*.*" 2>NUL',
            ], self.L, "TempClean", timeout=10)
            self.L("    ✓ Готово")

        # ── Flush system clipboard ────────────────────────────────
        if s.get("fu_clear_clipboard"):
            self.L("\n[►] Очистка буфера обмена...")
            run_cmd_s("cmd /c echo off | clip", 4)
            self.L("    ✓ Готово")

        # ── Network reset (instant, no reboot needed for basic) ───
        if s.get("fu_net_reset"):
            self.L("\n[►] Быстрый сброс сети (TCP, DNS, ARP)...")
            safe_run_batch([
                "ipconfig /flushdns",
                "arp -d *",
                "nbtstat -R",
                "nbtstat -RR",
                "netsh int ip reset 2>NUL",
                "netsh winsock reset catalog",
            ], self.L, "NetReset", timeout=12)
            self.L("    ✓ Готово (Winsock сброшен — рекомендуется перезагрузка)")

        # ── Set High Performance power plan (instant) ─────────────
        if s.get("fu_power_high"):
            self.L("\n[►] Активация плана питания — Высокая производительность...")
            run_cmd_s("powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c 2>NUL", 8)
            # Try Ultimate Performance
            run_cmd_s("powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 2>NUL", 8)
            run_cmd_s("powercfg -setactive e9a42b02-d5df-448d-aa00-03f14749eb61 2>NUL", 8)
            self.L("    ✓ Готово")

        # ── Disable startup delay (instant) ───────────────────────
        if s.get("fu_startup_delay"):
            self.L("\n[►] Убрать задержку автозапуска приложений...")
            safe_run_batch([
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Serialize" /v StartupDelayInMSec /t REG_DWORD /d 0 /f',
                'reg add "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon" /v SyncForegroundPolicy /t REG_DWORD /d 0 /f',
            ], self.L, "StartupDelay", timeout=5)
            self.L("    ✓ Готово")

        # ── TWEAKS: Mouse selection color ─────────────────────────
        if s.get("tweak_mouse_color"):
            c = self.tc.get("mouse_sel", QColor(0, 120, 215))
            r, g_val, b = c.red(), c.green(), c.blue()
            hex_color = f"{r} {g_val} {b}"
            self.L(f"\n[►] Смена цвета выделения мышкой → RGB({r},{g_val},{b})...")
            # Windows stores this as "R G B" string in HilightText / Hilight
            run_cmd_s(
                f'reg add "HKCU\\Control Panel\\Colors" /v Hilight /t REG_SZ /d "{hex_color}" /f', 5
            )
            run_cmd_s(
                f'reg add "HKCU\\Control Panel\\Colors" /v HilightText /t REG_SZ /d "255 255 255" /f', 5
            )
            # Apply immediately via SystemParametersInfo would need a DLL call;
            # restart Explorer to apply visually
            run_cmd_s("taskkill /f /im explorer.exe 2>NUL", 4)
            time.sleep(0.8)
            run_cmd_s("start explorer.exe", 3)
            self.L("    ✓ Готово (Explorer перезапущен)")

        # ── TWEAKS: Text selection color ──────────────────────────
        if s.get("tweak_text_color"):
            c = self.tc.get("text_sel", QColor(0, 78, 152))
            r, g_val, b = c.red(), c.green(), c.blue()
            hex_bgr = f"00{b:02X}{g_val:02X}{r:02X}"   # Windows accent uses BGR in registry
            self.L(f"\n[►] Смена цвета выделения текста → RGB({r},{g_val},{b})...")
            # Text highlight color via AccentColor (Win10/11 theme color)
            run_cmd_s(
                f'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent" '
                f'/v AccentColor /t REG_DWORD /d 0x{hex_bgr} /f', 5
            )
            run_cmd_s(
                f'reg add "HKCU\\Control Panel\\Colors" /v HilightText /t REG_SZ /d "255 255 255" /f', 5
            )
            self.L("    ✓ Готово (перелогин для полного применения)")

        # ── TWEAKS: Remove shortcut arrows ────────────────────────
        if s.get("tweak_no_arrows"):
            self.L("\n[►] Убрать стрелочки с ярлыков...")
            # Replace arrow overlay with blank icon
            run_ps(
                "Set-ItemProperty -Path 'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons' "
                "-Name '29' -Value '%SystemRoot%\\System32\\imageres.dll,197' -Force -EA SilentlyContinue",
                timeout=8
            )
            run_cmd_s("taskkill /f /im explorer.exe 2>NUL", 4)
            time.sleep(0.8)
            run_cmd_s("ie4uinit.exe -ClearIconCache", 4)
            run_cmd_s("start explorer.exe", 3)
            self.L("    ✓ Готово")

        # ── TWEAKS: Show file extensions ──────────────────────────
        if s.get("tweak_show_ext"):
            self.L("\n[►] Показывать расширения файлов...")
            run_cmd_s(
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced" '
                '/v HideFileExt /t REG_DWORD /d 0 /f', 5
            )
            run_cmd_s("taskkill /f /im explorer.exe 2>NUL & start explorer.exe", 5)
            self.L("    ✓ Готово")

        # ── TWEAKS: Show hidden files ─────────────────────────────
        if s.get("tweak_show_hidden"):
            self.L("\n[►] Показывать скрытые файлы и папки...")
            run_cmd_s(
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced" '
                '/v Hidden /t REG_DWORD /d 1 /f', 5
            )
            run_cmd_s("taskkill /f /im explorer.exe 2>NUL & start explorer.exe", 5)
            self.L("    ✓ Готово")

        # ── TWEAKS: Disable Aero Shake ────────────────────────────
        if s.get("tweak_no_aero_shake"):
            self.L("\n[►] Отключение Aero Shake (встряхивание окна)...")
            run_cmd_s(
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced" '
                '/v DisallowShaking /t REG_DWORD /d 1 /f', 5
            )
            self.L("    ✓ Готово")

        # ── TWEAKS: Faster Alt+Tab ────────────────────────────────
        if s.get("tweak_fast_alttab"):
            self.L("\n[►] Ускорение Alt+Tab (отключить анимацию переключения)...")
            safe_run_batch([
                'reg add "HKCU\\Control Panel\\Desktop" /v MenuShowDelay /t REG_SZ /d "0" /f',
                'reg add "HKCU\\Control Panel\\Desktop\\WindowMetrics" /v MinAnimate /t REG_SZ /d "0" /f',
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced" /v ExtendedUIHoverTime /t REG_DWORD /d 1 /f',
            ], self.L, "AltTab", timeout=5)
            self.L("    ✓ Готово")

        # ── TWEAKS: Dark mode ─────────────────────────────────────
        if s.get("tweak_dark_mode"):
            self.L("\n[►] Включение тёмного режима Windows...")
            safe_run_batch([
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize" /v AppsUseLightTheme /t REG_DWORD /d 0 /f',
                'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize" /v SystemUsesLightTheme /t REG_DWORD /d 0 /f',
            ], self.L, "DarkMode", timeout=5)
            self.L("    ✓ Готово")

        # ── TWEAKS: Disable Sticky Keys ───────────────────────────
        if s.get("tweak_no_sticky"):
            self.L("\n[►] Отключение залипания клавиш (Sticky Keys)...")
            safe_run_batch([
                'reg add "HKCU\\Control Panel\\Accessibility\\StickyKeys" /v Flags /t REG_SZ /d "506" /f',
                'reg add "HKCU\\Control Panel\\Accessibility\\ToggleKeys" /v Flags /t REG_SZ /d "58" /f',
                'reg add "HKCU\\Control Panel\\Accessibility\\Keyboard Response" /v Flags /t REG_SZ /d "122" /f',
            ], self.L, "StickyKeys", timeout=5)
            self.L("    ✓ Готово")

        self.L("\n" + "═" * 60)
        self.L("  ✅  Fast Utility завершён!")
        self.L("═" * 60)
        self.finished.emit()


# ══════════════════════════════════════════════════════════════════
#  COLOR PICKER WIDGET
# ══════════════════════════════════════════════════════════════════

class ColorPickerWidget(QWidget):
    """
    Self-contained RGB color picker.
    Contains: colored swatch button (opens QColorDialog) +
              three R/G/B sliders with spinboxes + HEX input field.
    Uses setVisible(True/False) to show/hide instead of setEnabled
    to avoid Qt grey-out rendering issues on dark stylesheets.
    """
    colorChanged = pyqtSignal(QColor)

    def __init__(self, default_color: QColor = None, parent=None):
        super().__init__(parent)
        self._color   = default_color if (default_color and default_color.isValid()) else QColor(0, 120, 215)
        self._locked  = False   # re-entrancy guard
        self._build()

    # ── Build ────────────────────────────────────────────────────
    def _build(self):
        root = QVBoxLayout(self)
        root.setContentsMargins(0, 4, 0, 4)
        root.setSpacing(6)
        self.setStyleSheet("")          # don't inherit parent disabled style

        # ── Swatch + "Open palette" button ───────────────────────
        top_row = QHBoxLayout(); top_row.setSpacing(8)

        self._swatch = QPushButton("  Выбрать цвет  ▼")
        self._swatch.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        self._swatch.setFixedHeight(34)
        self._swatch.clicked.connect(self._open_dialog)
        # Inline style so it isn't affected by global disabled state
        self._swatch.setStyleSheet(
            "QPushButton { border-radius:6px; font-size:12px; font-weight:600; "
            "padding:0 14px; border:2px solid #555; }"
            "QPushButton:hover { border-color:#aaa; }"
        )
        top_row.addWidget(self._swatch)
        top_row.addStretch()
        root.addLayout(top_row)

        # ── Slider box ───────────────────────────────────────────
        box = QWidget()
        box.setStyleSheet(
            "QWidget { background:#1a1a2a; border:1px solid #2e2e44; border-radius:8px; }"
        )
        grid = QGridLayout(box)
        grid.setContentsMargins(10, 8, 10, 8)
        grid.setSpacing(5)
        grid.setColumnStretch(1, 1)

        self._sliders: dict[str, QSlider]  = {}
        self._spins:   dict[str, QSpinBox] = {}

        channel_cfg = [
            ("R", "#cc4444", self._color.red()),
            ("G", "#44aa44", self._color.green()),
            ("B", "#4477cc", self._color.blue()),
        ]
        for row_idx, (ch, color_hex, init_val) in enumerate(channel_cfg):
            # Label
            lbl = QLabel(ch)
            lbl.setFixedWidth(14)
            lbl.setStyleSheet(
                f"QLabel {{ color:{color_hex}; font-weight:700; font-size:13px; "
                f"background:transparent; border:none; }}"
            )
            # Slider — explicit full style to avoid inheritance issues
            sl = QSlider(Qt.Orientation.Horizontal)
            sl.setRange(0, 255)
            sl.setValue(init_val)
            sl.setFixedHeight(18)
            sl.setStyleSheet(
                "QSlider::groove:horizontal {"
                "  height:4px; background:#333; border-radius:2px; margin:0; }"
                f"QSlider::sub-page:horizontal {{ background:{color_hex}; border-radius:2px; }}"
                "QSlider::handle:horizontal {"
                "  background:#ddd; width:14px; height:14px; margin:-5px 0;"
                "  border-radius:7px; border:1px solid #888; }"
                "QSlider::handle:horizontal:hover { background:#fff; }"
            )
            # SpinBox
            sp = QSpinBox()
            sp.setRange(0, 255)
            sp.setValue(init_val)
            sp.setFixedWidth(54)
            sp.setFixedHeight(26)
            sp.setStyleSheet(
                "QSpinBox { background:#111122; color:#d0d0d0; border:1px solid #3a3a55; "
                "border-radius:4px; padding:2px 4px; font-size:12px; }"
                "QSpinBox::up-button   { width:16px; border:none; background:#222233; }"
                "QSpinBox::down-button { width:16px; border:none; background:#222233; }"
                "QSpinBox::up-arrow    { width:7px; height:7px; }"
                "QSpinBox::down-arrow  { width:7px; height:7px; }"
            )

            # Connect — use default-arg capture to freeze loop variable
            sl.valueChanged.connect(
                lambda val, c=ch: self._on_slider(c, val))
            sp.valueChanged.connect(
                lambda val, c=ch: self._on_spin(c, val))

            self._sliders[ch] = sl
            self._spins[ch]   = sp

            grid.addWidget(lbl, row_idx, 0)
            grid.addWidget(sl,  row_idx, 1)
            grid.addWidget(sp,  row_idx, 2)

        root.addWidget(box)

        # ── HEX input ────────────────────────────────────────────
        hex_row = QHBoxLayout(); hex_row.setSpacing(8)
        hex_lbl = QLabel("HEX")
        hex_lbl.setFixedWidth(30)
        hex_lbl.setStyleSheet("QLabel { color:#888; font-size:11px; font-weight:600; "
                               "background:transparent; border:none; }")

        self._hex_edit = QLineEdit()
        self._hex_edit.setFixedHeight(28)
        self._hex_edit.setPlaceholderText("#RRGGBB  или  R, G, B")
        self._hex_edit.setStyleSheet(
            "QLineEdit { background:#111122; color:#c8c8c8; border:1px solid #3a3a55; "
            "border-radius:5px; padding:4px 8px; font-family:Consolas,monospace; font-size:12px; }"
            "QLineEdit:focus { border-color:#4f9ef8; }"
        )
        self._hex_edit.editingFinished.connect(self._on_hex_edited)

        hex_row.addWidget(hex_lbl)
        hex_row.addWidget(self._hex_edit)
        root.addLayout(hex_row)

        # Initialise display
        self._sync_display()

    # ── Sync all display elements from self._color ───────────────
    def _sync_display(self):
        if self._locked:
            return
        self._locked = True
        c = self._color
        r, g, b = c.red(), c.green(), c.blue()

        # Update sliders + spinboxes
        for ch, val in (("R", r), ("G", g), ("B", b)):
            self._sliders[ch].setValue(val)
            self._spins[ch].setValue(val)

        # Update HEX field
        self._hex_edit.setText(c.name().upper())

        # Update swatch button colour
        # Compute contrasting text colour for readability
        brightness = 0.299*r + 0.587*g + 0.114*b
        txt_col = "#000000" if brightness > 140 else "#ffffff"
        self._swatch.setStyleSheet(
            f"QPushButton {{ background:{c.name()}; color:{txt_col}; "
            f"border-radius:6px; font-size:12px; font-weight:600; "
            f"padding:0 14px; border:2px solid #777; }}"
            f"QPushButton:hover {{ border-color:#fff; }}"
        )

        self._locked = False

    # ── Slot: slider moved ───────────────────────────────────────
    def _on_slider(self, ch: str, val: int):
        if self._locked:
            return
        self._locked = True
        self._spins[ch].setValue(val)
        self._locked = False
        self._rebuild_color()

    # ── Slot: spinbox changed ────────────────────────────────────
    def _on_spin(self, ch: str, val: int):
        if self._locked:
            return
        self._locked = True
        self._sliders[ch].setValue(val)
        self._locked = False
        self._rebuild_color()

    # ── Rebuild QColor from current slider values ────────────────
    def _rebuild_color(self):
        r = self._sliders["R"].value()
        g = self._sliders["G"].value()
        b = self._sliders["B"].value()
        self._color = QColor(r, g, b)
        # Sync only HEX + swatch (sliders already correct)
        if self._locked:
            return
        self._locked = True
        self._hex_edit.setText(self._color.name().upper())
        brightness = 0.299*r + 0.587*g + 0.114*b
        txt_col = "#000000" if brightness > 140 else "#ffffff"
        self._swatch.setStyleSheet(
            f"QPushButton {{ background:{self._color.name()}; color:{txt_col}; "
            f"border-radius:6px; font-size:12px; font-weight:600; "
            f"padding:0 14px; border:2px solid #777; }}"
            f"QPushButton:hover {{ border-color:#fff; }}"
        )
        self._locked = False
        self.colorChanged.emit(self._color)

    # ── Slot: HEX field committed ────────────────────────────────
    def _on_hex_edited(self):
        import re as _re
        text = self._hex_edit.text().strip()
        # "R, G, B" or "R G B"
        m = _re.match(r'^(\d{1,3})\s*[,\s]\s*(\d{1,3})\s*[,\s]\s*(\d{1,3})$', text)
        if m:
            nc = QColor(
                max(0, min(255, int(m.group(1)))),
                max(0, min(255, int(m.group(2)))),
                max(0, min(255, int(m.group(3)))),
            )
        else:
            hex_s = text if text.startswith("#") else f"#{text}"
            nc = QColor(hex_s)
        if nc.isValid():
            self._color = nc
            self._sync_display()
            self.colorChanged.emit(self._color)
        else:
            # Restore current value on bad input
            self._hex_edit.setText(self._color.name().upper())

    # ── Slot: swatch button → open QColorDialog ──────────────────
    def _open_dialog(self):
        nc = QColorDialog.getColor(self._color, self, "Выбор цвета",
                                   QColorDialog.ColorDialogOption.ShowAlphaChannel)
        if nc.isValid():
            nc.setAlpha(255)    # ignore alpha — Windows doesn't use it here
            self._color = nc
            self._sync_display()
            self.colorChanged.emit(self._color)

    # ── Public getter ────────────────────────────────────────────
    def color(self) -> QColor:
        return self._color


# ══════════════════════════════════════════════════════════════════
#  TAB 5 — FAST UTILITY
# ══════════════════════════════════════════════════════════════════

class Tab5_FastUtility(QWidget):
    def __init__(self):
        super().__init__()
        self.checks: dict[str, QCheckBox] = {}
        self._mouse_color = QColor(0, 120, 215)
        self._text_color  = QColor(0, 78, 152)
        self._cmd_win = None
        self._build()

    def _cb(self, key, label, layout):
        cb = QCheckBox(label)
        cb.setChecked(False)
        layout.addWidget(cb)
        self.checks[key] = cb

    def _group(self, title):
        g  = QGroupBox(title)
        gl = QVBoxLayout(g); gl.setSpacing(5)
        return g, gl

    def _build(self):
        outer = QVBoxLayout(self)
        outer.setContentsMargins(0, 0, 0, 0)
        outer.setSpacing(0)

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        outer.addWidget(scroll)

        w = QWidget(); scroll.setWidget(w)
        root = QVBoxLayout(w)
        root.setContentsMargins(24, 20, 24, 24)
        root.setSpacing(16)

        # ── Info banner ──────────────────────────────────────────
        info = QLabel(
            "⚡  Fast Utility — мгновенные улучшения без перезагрузки\n"
            "Все операции применяются сразу. Рекомендуется создать точку восстановления перед запуском."
        )
        info.setObjectName("infoCard")
        info.setWordWrap(True)
        root.addWidget(info)

        # ── Two-column layout ────────────────────────────────────
        cols = QHBoxLayout(); cols.setSpacing(16)

        # ── LEFT: Fast Utility checkboxes ────────────────────────
        left = QVBoxLayout(); left.setSpacing(10)

        g, gl = self._group("🧹  Очистка")
        self._cb("fu_ram_clean",       "Memory Cleaner — очистка Standby/Modified RAM", gl)
        self._cb("fu_temp_clean",      "Удалить временные файлы (TEMP, Prefetch)", gl)
        self._cb("fu_icon_cache",      "Сбросить кэш иконок", gl)
        self._cb("fu_thumb_cache",     "Очистить кэш эскизов (thumbnails)", gl)
        self._cb("fu_clear_clipboard", "Очистить буфер обмена", gl)
        left.addWidget(g)

        g, gl = self._group("🌐  Сеть")
        self._cb("fu_flush_dns", "Flush DNS + регистрация DNS", gl)
        self._cb("fu_net_reset", "Быстрый сброс TCP/ARP/Winsock", gl)
        left.addWidget(g)

        g, gl = self._group("⚡  Производительность")
        self._cb("fu_kill_bloat",       "Завершить фоновые bloatware процессы", gl)
        self._cb("fu_restart_explorer", "Перезапустить Explorer", gl)
        self._cb("fu_power_high",       "Активировать план питания Ultimate Performance", gl)
        self._cb("fu_startup_delay",    "Убрать задержку автозапуска приложений", gl)
        left.addWidget(g)

        left.addStretch()
        cols.addLayout(left, 1)

        # ── RIGHT: Visual tweaks ─────────────────────────────────
        right = QVBoxLayout(); right.setSpacing(10)

        g, gl = self._group("🖱  Цвет выделения")

        # Mouse selection color
        mouse_check = QCheckBox("Изменить цвет выделения мышкой")
        mouse_check.setChecked(False)
        gl.addWidget(mouse_check)
        self.checks["tweak_mouse_color"] = mouse_check

        self._mouse_picker = ColorPickerWidget(QColor(0, 120, 215))
        self._mouse_picker.setVisible(False)
        self._mouse_picker.colorChanged.connect(lambda c: setattr(self, "_mouse_color", c))
        gl.addWidget(self._mouse_picker)
        gl.addSpacing(2)
        mouse_check.toggled.connect(self._mouse_picker.setVisible)

        # Separator line
        sep = QFrame(); sep.setObjectName("sep"); sep.setFrameShape(QFrame.Shape.HLine)
        gl.addWidget(sep)
        gl.addSpacing(2)

        # Text selection color
        text_check = QCheckBox("Изменить цвет выделения текста")
        text_check.setChecked(False)
        gl.addWidget(text_check)
        self.checks["tweak_text_color"] = text_check

        self._text_picker = ColorPickerWidget(QColor(0, 78, 152))
        self._text_picker.setVisible(False)
        self._text_picker.colorChanged.connect(lambda c: setattr(self, "_text_color", c))
        gl.addWidget(self._text_picker)
        text_check.toggled.connect(self._text_picker.setVisible)
        right.addWidget(g)

        g, gl = self._group("🎨  Интерфейс")
        self._cb("tweak_no_arrows",    "Убрать стрелочки с ярлыков", gl)
        self._cb("tweak_show_ext",     "Показывать расширения файлов", gl)
        self._cb("tweak_show_hidden",  "Показывать скрытые файлы и папки", gl)
        self._cb("tweak_dark_mode",    "Включить тёмный режим Windows", gl)
        self._cb("tweak_no_aero_shake","Отключить Aero Shake (встряхивание окна)", gl)
        self._cb("tweak_fast_alttab",  "Ускорить Alt+Tab (убрать анимацию)", gl)
        self._cb("tweak_no_sticky",    "Отключить залипание клавиш (Sticky Keys)", gl)
        right.addWidget(g)

        right.addStretch()
        cols.addLayout(right, 1)

        root.addLayout(cols)

        # ── Run button ───────────────────────────────────────────
        root.addWidget(_sep())
        btn_row = QHBoxLayout()
        btn_row.addStretch()
        self._btn_run = QPushButton("⚡   Применить выбранное")
        self._btn_run.setObjectName("fuRunBtn")
        self._btn_run.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
        self._btn_run.setFixedHeight(46)
        self._btn_run.clicked.connect(self._run)
        btn_row.addWidget(self._btn_run)
        root.addLayout(btn_row)

    def _run(self):
        settings = {k: cb.isChecked() for k, cb in self.checks.items()}
        active = [k for k, v in settings.items() if v]
        if not active:
            QMessageBox.warning(self, "Ничего не выбрано",
                "Выберите хотя бы одну операцию.")
            return
        colors = {
            "mouse_sel": self._mouse_color,
            "text_sel":  self._text_color,
        }
        self._btn_run.setEnabled(False)
        self._cmd_win = _FastCmdWindow(settings, colors, on_finish=lambda: self._btn_run.setEnabled(True))
        self._cmd_win.show()


class _FastCmdWindow(QWidget):
    """Console window for Fast Utility."""
    def __init__(self, settings, colors, on_finish=None):
        super().__init__()
        self._on_finish = on_finish
        self.setObjectName("consoleRoot")
        self.setWindowTitle("minoreOptimizer — Fast Utility")
        self.resize(800, 480)
        self.setStyleSheet(STYLE)

        lay = QVBoxLayout(self)
        lay.setContentsMargins(0, 0, 0, 0)
        lay.setSpacing(0)

        hdr = QWidget()
        hdr.setStyleSheet("background:#111; border-bottom:1px solid #1e1e1e;")
        hl  = QHBoxLayout(hdr); hl.setContentsMargins(16, 10, 16, 10)
        dot = QLabel("●"); dot.setStyleSheet("color:#f0c040; font-size:14px;")
        lbl = QLabel("  minoreOptimizer — Fast Utility")
        lbl.setStyleSheet("color:#f0c040; font-size:13px; font-weight:600;")
        hl.addWidget(dot); hl.addWidget(lbl); hl.addStretch()
        lay.addWidget(hdr)

        self.console = QTextEdit()
        self.console.setObjectName("console")
        self.console.setReadOnly(True)
        self.console.setLineWrapMode(QTextEdit.LineWrapMode.NoWrap)
        lay.addWidget(self.console)

        bb = QWidget(); bb.setStyleSheet("background:#111; border-top:1px solid #1e1e1e;")
        bl = QHBoxLayout(bb); bl.setContentsMargins(16, 12, 16, 12)
        btn_close = QPushButton("✕  Закрыть")
        btn_close.setObjectName("btnCloseConsole")
        btn_close.clicked.connect(self.close)
        bl.addStretch(); bl.addWidget(btn_close)
        lay.addWidget(bb)

        self.worker = FastUtilityWorker(settings, colors)
        self.worker.log.connect(self._append)
        self.worker.finished.connect(self._done)
        self.worker.start()

    def _append(self, text):
        self.console.append(text)
        sb = self.console.verticalScrollBar()
        sb.setValue(sb.maximum())

    def _done(self):
        QMessageBox.information(self, "Готово", "✅  Fast Utility завершён!")
        if self._on_finish:
            self._on_finish()

    def closeEvent(self, e):
        if self._on_finish:
            self._on_finish()
        super().closeEvent(e)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("minoreOptimizer v2.0")
        self.resize(920, 740); self.setMinimumSize(760, 560)
        self.setStyleSheet(STYLE)
        icon_path = asset("icon.png")
        if os.path.exists(icon_path):
            self.setWindowIcon(QIcon(icon_path))

        central = QWidget(); central.setObjectName("root")
        self.setCentralWidget(central)
        ml = QVBoxLayout(central); ml.setContentsMargins(0,0,0,0); ml.setSpacing(0)

        self.tabs = QTabWidget(); self.tabs.setDocumentMode(True)
        ml.addWidget(self.tabs)

        self.tab_settings = Tab2_Settings()
        self.tab_main     = Tab1_Main(self.tab_settings.get_settings)
        self.tab_info     = Tab3_SysInfo()
        self.tab_fast     = Tab5_FastUtility()

        self.tabs.addTab(self.tab_main,     "   🏠  Главная   ")
        self.tabs.addTab(self.tab_settings, "   ⚙  Настройки   ")
        self.tabs.addTab(self.tab_info,     "   💻  Система   ")
        self.tabs.addTab(self.tab_fast,     "   ⚡  Fast Utility   ")

        self.statusBar().setStyleSheet("background:#111; color:#555; font-size:11px; border-top:1px solid #222;")
        self.statusBar().showMessage(
            f"   minoreOptimizer v2.0  ·  Windows {platform.version()}  ·  "
            f"{'✅ Администратор' if is_admin() else '⚠  Нет прав администратора'}"
        )

# ══════════════════════════════════════════════════════════════════
#  ENTRY POINT
# ══════════════════════════════════════════════════════════════════

def main():
    if not is_admin():
        _app = QApplication(sys.argv)
        msg = QMessageBox()
        msg.setWindowTitle("minoreOptimizer — Права администратора")
        msg.setText("minoreOptimizer требует прав администратора.\n\nНажмите OK для перезапуска.")
        msg.setIcon(QMessageBox.Icon.Warning)
        msg.setStandardButtons(QMessageBox.StandardButton.Ok | QMessageBox.StandardButton.Cancel)
        if msg.exec() == QMessageBox.StandardButton.Ok:
            run_as_admin()
        sys.exit(0)

    app = QApplication(sys.argv)
    app.setApplicationName("minoreOptimizer")
    app.setApplicationVersion("2.0")
    app.setOrganizationName("bpm500")

    p = QPalette()
    p.setColor(QPalette.ColorRole.Window,          QColor(24,24,24))
    p.setColor(QPalette.ColorRole.WindowText,      QColor(200,200,200))
    p.setColor(QPalette.ColorRole.Base,            QColor(30,30,30))
    p.setColor(QPalette.ColorRole.AlternateBase,   QColor(38,38,38))
    p.setColor(QPalette.ColorRole.Text,            QColor(200,200,200))
    p.setColor(QPalette.ColorRole.Button,          QColor(42,42,42))
    p.setColor(QPalette.ColorRole.ButtonText,      QColor(200,200,200))
    p.setColor(QPalette.ColorRole.Highlight,       QColor(79,158,248))
    p.setColor(QPalette.ColorRole.HighlightedText, QColor(255,255,255))
    app.setPalette(p)

    win = MainWindow(); win.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
