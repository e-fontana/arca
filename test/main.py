#!/usr/bin/env python3
"""ARCA Controller — Terminal UI for NFC access control FSM (115200 baud)."""

import curses
import serial
import threading
import queue
import time
import sys
from dataclasses import dataclass, field
from typing import Optional

# ── Constants ────────────────────────────────────────────────────────────────

DEFAULT_PORT   = "/dev/ttyUSB0"
DEFAULT_BAUD   = 115200
MAX_USER_LEVEL = 5
CMD_CANCELAR   = b"\x1b"           # ESC byte
TIMEOUT_NFC_S  = 5                 # mirrors TIMEOUT_AGUARDA_NFC on firmware

# Maps device messages → inferred FSM state label
DEVICE_STATE_MAP = {
    "ARCA:MENU":              "MENU_PC",
    "ARCA:DESCONECTADO":      "IDLE",
    "CADASTRO:AGUARDA_DADOS": "CADASTRO_RECEBE_DADOS",
    "CADASTRO:AGUARDA_NFC":   "CADASTRO_AGUARDA_NFC",
    "CADASTRO:NFC_OK":        "CADASTRO_NFC_OK",
    "CADASTRO:NFC_TIMEOUT":   "CADASTRO_RECEBE_DADOS (timeout)",
    "CADASTRO:CANCELADO":     "MENU_PC",
    "CADASTRO:ERR_DADOS":     "CADASTRO_RECEBE_DADOS (erro dados)",
    "ACK:OK":                 "MENU_PC",
    "CMD:INVALIDO":           "MENU_PC (cmd inválido)",
}

MENU_ITEMS = [
    ("CMD:1", "Cadastrar Usuário"),
    ("CMD:2", "Deletar Usuário"),
    ("CMD:3", "Exportar Logs"),
    ("CMD:0", "Desconectar"),
]

# ── Screens ───────────────────────────────────────────────────────────────────

SCREEN_CONNECT  = "connect"
SCREEN_MENU     = "menu"
SCREEN_CADASTRO = "cadastro"
SCREEN_WAITING  = "waiting"   # aguardando NFC / resposta

# ── Color pairs ───────────────────────────────────────────────────────────────

C_NORMAL   = 1
C_SELECTED = 2
C_TITLE    = 3
C_OK       = 4
C_WARN     = 5
C_ERR      = 6
C_TX       = 7
C_RX       = 8
C_DIM      = 9


# ── Serial manager ────────────────────────────────────────────────────────────

class SerialManager:
    def __init__(self, rx_queue: queue.Queue):
        self._q    = rx_queue
        self._ser: Optional[serial.Serial] = None
        self._lock = threading.Lock()
        self._running = False

    @property
    def connected(self) -> bool:
        return self._ser is not None and self._ser.is_open

    def connect(self, port: str, baud: int) -> str:
        """Returns '' on success, error message on failure."""
        try:
            ser = serial.Serial(port, baud, timeout=0.05)
            with self._lock:
                self._ser = ser
            self._running = True
            threading.Thread(target=self._rx_loop, daemon=True).start()
            return ""
        except Exception as exc:
            return str(exc)

    def disconnect(self):
        self._running = False
        with self._lock:
            if self._ser:
                try:
                    self._ser.close()
                except Exception:
                    pass
                self._ser = None

    def send(self, text: str):
        data = (text + "\n").encode("utf-8")
        with self._lock:
            if self._ser and self._ser.is_open:
                self._ser.write(data)

    def send_cancel(self):
        with self._lock:
            if self._ser and self._ser.is_open:
                self._ser.write(CMD_CANCELAR)

    def _rx_loop(self):
        buf = b""
        while self._running:
            try:
                with self._lock:
                    ser = self._ser
                if ser is None or not ser.is_open:
                    break
                chunk = ser.read(64)
                if chunk:
                    buf += chunk
                    while b"\n" in buf:
                        line, buf = buf.split(b"\n", 1)
                        line = line.strip(b"\r")
                        if line:
                            self._q.put(("rx", line.decode("utf-8", errors="replace")))
            except Exception:
                break


# ── App ───────────────────────────────────────────────────────────────────────

@dataclass
class LogEntry:
    ts: str
    direction: str   # "tx" | "rx" | "info" | "warn" | "err"
    text: str


class App:
    def __init__(self, stdscr: "curses.window"):
        self.scr  = stdscr
        self.rxq: queue.Queue = queue.Queue()
        self.mgr  = SerialManager(self.rxq)

        # UI state
        self.screen      = SCREEN_CONNECT
        self.menu_cursor = 0
        self.log: list[LogEntry] = []
        self.log_scroll  = 0

        # Connection form
        self.port    = DEFAULT_PORT
        self.baud    = str(DEFAULT_BAUD)
        self.cf_idx  = 0          # focused field index on connect screen

        # Cadastro form
        self.cad_nome     = ""
        self.cad_nivel    = "1"
        self.cad_field    = 0     # 0=nome, 1=nivel
        self.cad_editing  = False

        # Device state label
        self.device_state = "IDLE"

        # Status bar message
        self.status_msg   = "Use ↑↓ para navegar, ENTER para confirmar, ESC para voltar."
        self.status_color = C_DIM

        # Waiting screen message
        self.wait_msg  = ""
        self.wait_since: float = 0.0

        self._init_colors()

    # ── Colours ──────────────────────────────────────────────────────────────

    def _init_colors(self):
        curses.start_color()
        curses.use_default_colors()
        curses.init_pair(C_NORMAL,   curses.COLOR_WHITE,   -1)
        curses.init_pair(C_SELECTED, curses.COLOR_BLACK,   curses.COLOR_CYAN)
        curses.init_pair(C_TITLE,    curses.COLOR_CYAN,    -1)
        curses.init_pair(C_OK,       curses.COLOR_GREEN,   -1)
        curses.init_pair(C_WARN,     curses.COLOR_YELLOW,  -1)
        curses.init_pair(C_ERR,      curses.COLOR_RED,     -1)
        curses.init_pair(C_TX,       curses.COLOR_BLUE,    -1)
        curses.init_pair(C_RX,       curses.COLOR_GREEN,   -1)
        curses.init_pair(C_DIM,      8,                    -1)   # dark gray

    # ── Helpers ───────────────────────────────────────────────────────────────

    def _ts(self) -> str:
        return time.strftime("%H:%M:%S")

    def _log(self, direction: str, text: str):
        self.log.append(LogEntry(self._ts(), direction, text))
        self.log_scroll = max(0, len(self.log) - 1)

    def _set_status(self, msg: str, color: int = C_DIM):
        self.status_msg   = msg
        self.status_color = color

    def _send(self, text: str):
        self.mgr.send(text)
        self._log("tx", text)

    # ── Main loop ─────────────────────────────────────────────────────────────

    def run(self):
        curses.curs_off = curses.cbreak
        self.scr.keypad(True)
        self.scr.nodelay(True)
        curses.curs_set(0)

        while True:
            self._drain_rx()
            self._draw()
            key = self.scr.getch()
            if key == curses.ERR:
                time.sleep(0.02)
                continue
            if self._handle_key(key) == "quit":
                break

        self.mgr.disconnect()

    # ── RX processing ────────────────────────────────────────────────────────

    def _drain_rx(self):
        while not self.rxq.empty():
            kind, text = self.rxq.get_nowait()
            self._log("rx", text)
            self._handle_device_msg(text)

    def _handle_device_msg(self, msg: str):
        if msg in DEVICE_STATE_MAP:
            self.device_state = DEVICE_STATE_MAP[msg]

        if msg == "ARCA:MENU":
            self.screen = SCREEN_MENU
            self._set_status("Selecione uma opção com ↑↓ e ENTER.", C_OK)

        elif msg == "CADASTRO:AGUARDA_DADOS":
            self.screen      = SCREEN_CADASTRO
            self.cad_nome    = ""
            self.cad_nivel   = "1"
            self.cad_field   = 0
            self.cad_editing = False
            self._set_status("Preencha os dados e pressione ENTER para enviar.", C_NORMAL)

        elif msg == "CADASTRO:ERR_DADOS":
            self.screen = SCREEN_CADASTRO
            self._set_status("Dados inválidos — corrija e tente novamente.", C_ERR)

        elif msg in ("CADASTRO:AGUARDA_NFC",):
            self.screen     = SCREEN_WAITING
            self.wait_msg   = "Aproxime o cartão NFC do leitor…"
            self.wait_since = time.time()
            self._set_status("Pressione ESC para cancelar.", C_WARN)

        elif msg in ("CADASTRO:CANCELADO",):
            self.screen = SCREEN_MENU
            self._set_status("Operação cancelada.", C_WARN)

        elif msg in ("CADASTRO:NFC_TIMEOUT",):
            self.screen      = SCREEN_CADASTRO
            self.cad_editing = False
            self._set_status("Timeout NFC — tente novamente.", C_WARN)

        elif msg == "CADASTRO:NFC_OK":
            self._set_status("NFC lido com sucesso! Gravando…", C_OK)

        elif msg == "ACK:OK":
            self.screen = SCREEN_MENU
            self._set_status("Cadastro concluído com sucesso!", C_OK)

        elif msg == "ARCA:DESCONECTADO":
            self.screen = SCREEN_CONNECT
            self._set_status("Dispositivo desconectado.", C_WARN)

        elif msg == "CMD:INVALIDO":
            self._set_status("Comando inválido.", C_ERR)

    # ── Key handling ─────────────────────────────────────────────────────────

    def _handle_key(self, key: int) -> str:
        if self.screen == SCREEN_CONNECT:
            return self._key_connect(key)
        elif self.screen == SCREEN_MENU:
            return self._key_menu(key)
        elif self.screen == SCREEN_CADASTRO:
            return self._key_cadastro(key)
        elif self.screen == SCREEN_WAITING:
            return self._key_waiting(key)
        return ""

    # ·· Connect screen ·······················································

    def _key_connect(self, key: int) -> str:
        if key == curses.KEY_UP:
            self.cf_idx = (self.cf_idx - 1) % 3   # 0=port, 1=baud, 2=connect
        elif key == curses.KEY_DOWN:
            self.cf_idx = (self.cf_idx + 1) % 3
        elif key == ord("\t"):
            self.cf_idx = (self.cf_idx + 1) % 3
        elif key in (10, 13, curses.KEY_ENTER):
            if self.cf_idx == 2:
                self._do_connect()
            elif self.cf_idx == 0:
                self.port = self._inline_edit(self.port)
            elif self.cf_idx == 1:
                self.baud = self._inline_edit(self.baud)
        elif key == ord("q"):
            return "quit"
        return ""

    def _do_connect(self):
        if self.mgr.connected:
            self.mgr.disconnect()
            self._log("info", "Desconectado.")
            self.device_state = "IDLE"
            self._set_status("Desconectado.", C_WARN)
            return
        err = self.mgr.connect(self.port, int(self.baud))
        if err:
            self._log("err", f"Erro: {err}")
            self._set_status(f"Erro: {err}", C_ERR)
        else:
            self._log("info", f"Conectado em {self.port} @ {self.baud}")
            self._set_status("Conectado! Enviando wake…", C_OK)
            # Wake the device (any line triggers ARCA:MENU)
            self._send("WAKE")

    # ·· Menu screen ··························································

    def _key_menu(self, key: int) -> str:
        if key == curses.KEY_UP:
            self.menu_cursor = (self.menu_cursor - 1) % len(MENU_ITEMS)
        elif key == curses.KEY_DOWN:
            self.menu_cursor = (self.menu_cursor + 1) % len(MENU_ITEMS)
        elif key in (10, 13, curses.KEY_ENTER):
            cmd, _ = MENU_ITEMS[self.menu_cursor]
            self._send(cmd)
        elif key == 27:   # ESC — go back to connect screen
            self.screen = SCREEN_CONNECT
        return ""

    # ·· Cadastro form ························································

    def _key_cadastro(self, key: int) -> str:
        if key == 27:   # ESC = cancel
            self.mgr.send_cancel()
            self._log("tx", "[ESC] CANCELAR")
            self.screen = SCREEN_MENU
            return ""

        if key in (curses.KEY_UP, curses.KEY_DOWN, ord("\t")):
            # Move between fields
            if key == curses.KEY_UP:
                self.cad_field = (self.cad_field - 1) % 2
            else:
                self.cad_field = (self.cad_field + 1) % 2
            self.cad_editing = False
            return ""

        if key in (10, 13, curses.KEY_ENTER):
            if not self.cad_editing:
                self.cad_editing = True
            else:
                # Advance field or submit
                if self.cad_field == 0:
                    self.cad_field   = 1
                    self.cad_editing = True
                else:
                    self._submit_cadastro()
            return ""

        if not self.cad_editing:
            return ""

        # Text editing
        if self.cad_field == 0:
            self.cad_nome = _edit_str(self.cad_nome, key, max_len=31)
        elif self.cad_field == 1:
            if key == curses.KEY_UP:
                n = int(self.cad_nivel) if self.cad_nivel.isdigit() else 1
                self.cad_nivel = str(min(n + 1, MAX_USER_LEVEL))
            elif key == curses.KEY_DOWN:
                n = int(self.cad_nivel) if self.cad_nivel.isdigit() else 1
                self.cad_nivel = str(max(n - 1, 1))
            else:
                self.cad_nivel = _edit_str(self.cad_nivel, key, max_len=1, digits_only=True)

        return ""

    def _submit_cadastro(self):
        nome  = self.cad_nome.strip()
        nivel = self.cad_nivel.strip()
        if not nome:
            self._set_status("Nome não pode ser vazio.", C_ERR)
            return
        if not nivel.isdigit() or not (1 <= int(nivel) <= MAX_USER_LEVEL):
            self._set_status(f"Nível deve ser 1–{MAX_USER_LEVEL}.", C_ERR)
            return
        msg = f"NOME:{nome};NIVEL:{nivel}"
        self._send(msg)
        self.cad_editing = False

    # ·· Waiting screen ·······················································

    def _key_waiting(self, key: int) -> str:
        if key == 27:
            self.mgr.send_cancel()
            self._log("tx", "[ESC] CANCELAR")
        return ""

    # ── Inline string editor (blocking) ─────────────────────────────────────

    def _inline_edit(self, current: str) -> str:
        """Simple blocking edit box for the connect form fields."""
        curses.curs_set(1)
        buf = list(current)
        self._draw()
        while True:
            key = self.scr.getch()
            if key in (10, 13, curses.KEY_ENTER):
                break
            elif key == 27:
                buf = list(current)
                break
            else:
                buf_str = _edit_str("".join(buf), key)
                buf = list(buf_str)
            self._draw()
        curses.curs_set(0)
        return "".join(buf)

    # ── Drawing ───────────────────────────────────────────────────────────────

    def _draw(self):
        self.scr.erase()
        h, w = self.scr.getmaxyx()

        self._draw_header(w)
        self._draw_status(h, w)

        body_top = 2
        body_bot = h - 2   # status bar takes last row
        log_h    = 8
        main_h   = body_bot - body_top - log_h - 1

        self._draw_main(body_top, 0, main_h, w)
        self._draw_log(body_top + main_h, 0, log_h, w)

        self.scr.refresh()

    def _draw_header(self, w: int):
        conn_indicator = (
            f"● {self.port}" if self.mgr.connected else "○ Desconectado"
        )
        color = C_OK if self.mgr.connected else C_ERR
        title = " ARCA Controller "
        state = f" [{self.device_state}] "

        self.scr.attron(curses.color_pair(C_TITLE) | curses.A_BOLD)
        self.scr.addstr(0, 0, title.ljust(w - len(state) - len(conn_indicator) - 4))
        self.scr.attroff(curses.color_pair(C_TITLE) | curses.A_BOLD)

        self.scr.attron(curses.color_pair(C_DIM))
        x = len(title)
        self.scr.addstr(0, x, state[:w - x - len(conn_indicator) - 1])
        self.scr.attroff(curses.color_pair(C_DIM))

        self.scr.attron(curses.color_pair(color) | curses.A_BOLD)
        ci_x = max(0, w - len(conn_indicator) - 1)
        self.scr.addstr(0, ci_x, conn_indicator[:w - ci_x - 1])
        self.scr.attroff(curses.color_pair(color) | curses.A_BOLD)

        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(1, 0, "─" * (w - 1))
        self.scr.attroff(curses.color_pair(C_DIM))

    def _draw_status(self, h: int, w: int):
        self.scr.attron(curses.color_pair(self.status_color))
        self.scr.addstr(h - 1, 0, self.status_msg[:w - 1].ljust(w - 1))
        self.scr.attroff(curses.color_pair(self.status_color))

    def _draw_main(self, top: int, left: int, height: int, width: int):
        if self.screen == SCREEN_CONNECT:
            self._draw_connect(top, left, height, width)
        elif self.screen == SCREEN_MENU:
            self._draw_menu(top, left, height, width)
        elif self.screen == SCREEN_CADASTRO:
            self._draw_cadastro(top, left, height, width)
        elif self.screen == SCREEN_WAITING:
            self._draw_waiting(top, left, height, width)

    # ·· Connect screen drawing ···············································

    def _draw_connect(self, top, left, h, w):
        def row(i):
            return top + 2 + i * 2

        self.scr.attron(curses.color_pair(C_TITLE) | curses.A_BOLD)
        self.scr.addstr(top, left + 2, "CONEXÃO SERIAL")
        self.scr.attroff(curses.color_pair(C_TITLE) | curses.A_BOLD)

        fields = [
            ("Porta ", self.port),
            ("Baud  ", self.baud),
        ]
        for i, (label, val) in enumerate(fields):
            is_focused = (self.cf_idx == i)
            attr = curses.color_pair(C_SELECTED) if is_focused else curses.color_pair(C_NORMAL)
            self.scr.attron(curses.color_pair(C_DIM))
            self.scr.addstr(row(i), left + 2, f"{label}: ")
            self.scr.attroff(curses.color_pair(C_DIM))
            self.scr.attron(attr | curses.A_BOLD)
            self.scr.addstr(f" {val:<24} ")
            self.scr.attroff(attr | curses.A_BOLD)
            if is_focused:
                self.scr.attron(curses.color_pair(C_DIM))
                self.scr.addstr("  ← ENTER para editar")
                self.scr.attroff(curses.color_pair(C_DIM))

        # Connect button
        btn_label = " Desconectar " if self.mgr.connected else "  Conectar  "
        btn_color = curses.color_pair(C_ERR) if self.mgr.connected else curses.color_pair(C_OK)
        is_btn    = (self.cf_idx == 2)
        attr = (btn_color | curses.A_REVERSE | curses.A_BOLD) if is_btn else (btn_color | curses.A_BOLD)
        self.scr.attron(attr)
        self.scr.addstr(row(2), left + 4, f"[ {btn_label} ]")
        self.scr.attroff(attr)

        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(row(3) + 1, left + 2, "↑↓ navegar   ENTER selecionar/editar   q sair")
        self.scr.attroff(curses.color_pair(C_DIM))

    # ·· Menu screen drawing ··················································

    def _draw_menu(self, top, left, h, w):
        self.scr.attron(curses.color_pair(C_TITLE) | curses.A_BOLD)
        self.scr.addstr(top, left + 2, "MENU PRINCIPAL")
        self.scr.attroff(curses.color_pair(C_TITLE) | curses.A_BOLD)

        for i, (cmd, label) in enumerate(MENU_ITEMS):
            is_sel = (i == self.menu_cursor)
            attr   = curses.color_pair(C_SELECTED) | curses.A_BOLD if is_sel else curses.color_pair(C_NORMAL)
            prefix = " ▶ " if is_sel else "   "
            row    = top + 2 + i * 2
            self.scr.attron(attr)
            self.scr.addstr(row, left + 2, f"{prefix}[{cmd}]  {label:<30}")
            self.scr.attroff(attr)

        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(top + 2 + len(MENU_ITEMS) * 2 + 1, left + 2,
                        "↑↓ navegar   ENTER enviar comando   ESC voltar")
        self.scr.attroff(curses.color_pair(C_DIM))

    # ·· Cadastro form drawing ················································

    def _draw_cadastro(self, top, left, h, w):
        self.scr.attron(curses.color_pair(C_TITLE) | curses.A_BOLD)
        self.scr.addstr(top, left + 2, "CADASTRO DE USUÁRIO")
        self.scr.attroff(curses.color_pair(C_TITLE) | curses.A_BOLD)

        fields = [
            ("Nome  ", self.cad_nome,  31),
            (f"Nível (1–{MAX_USER_LEVEL})", self.cad_nivel, 1),
        ]
        for i, (label, val, _) in enumerate(fields):
            is_focused = (self.cad_field == i)
            is_editing = is_focused and self.cad_editing
            row = top + 2 + i * 3

            self.scr.attron(curses.color_pair(C_DIM))
            self.scr.addstr(row, left + 2, f"{label}: ")
            self.scr.attroff(curses.color_pair(C_DIM))

            if is_editing:
                attr = curses.color_pair(C_SELECTED) | curses.A_BOLD
            elif is_focused:
                attr = curses.color_pair(C_WARN) | curses.A_BOLD
            else:
                attr = curses.color_pair(C_NORMAL)

            display = val if val else "─"
            self.scr.attron(attr)
            self.scr.addstr(f" {display:<32} ")
            self.scr.attroff(attr)

            if is_editing:
                hint = " (editando — ENTER para confirmar)"
            elif is_focused:
                hint = " (ENTER para editar)"
                if i == 1:
                    hint = " (↑↓ ou ENTER para editar)"
            else:
                hint = ""
            self.scr.attron(curses.color_pair(C_DIM))
            self.scr.addstr(hint[:w - left - 2])
            self.scr.attroff(curses.color_pair(C_DIM))

        hint_row = top + 2 + len(fields) * 3 + 1
        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(hint_row, left + 2, "TAB/↓ próximo campo   ENTER confirmar/enviar   ESC cancelar")
        self.scr.attroff(curses.color_pair(C_DIM))

        # Preview of command to be sent
        preview = f"NOME:{self.cad_nome};NIVEL:{self.cad_nivel}"
        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(hint_row + 2, left + 2, f"Payload: {preview}")
        self.scr.attroff(curses.color_pair(C_DIM))

    # ·· Waiting screen drawing ···············································

    def _draw_waiting(self, top, left, h, w):
        elapsed = int(time.time() - self.wait_since)
        remaining = max(0, TIMEOUT_NFC_S - elapsed)
        bar_w  = min(30, w - 6)
        filled = int(bar_w * remaining / TIMEOUT_NFC_S)
        bar    = "█" * filled + "░" * (bar_w - filled)

        self.scr.attron(curses.color_pair(C_TITLE) | curses.A_BOLD)
        self.scr.addstr(top, left + 2, "AGUARDANDO NFC")
        self.scr.attroff(curses.color_pair(C_TITLE) | curses.A_BOLD)

        self.scr.attron(curses.color_pair(C_WARN) | curses.A_BOLD)
        self.scr.addstr(top + 2, left + 4, self.wait_msg)
        self.scr.attroff(curses.color_pair(C_WARN) | curses.A_BOLD)

        color = C_OK if remaining > 10 else C_ERR
        self.scr.attron(curses.color_pair(color))
        self.scr.addstr(top + 4, left + 4, f"[{bar}] {remaining:>2}s")
        self.scr.attroff(curses.color_pair(color))

        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(top + 6, left + 4, "ESC para cancelar")
        self.scr.attroff(curses.color_pair(C_DIM))

    # ·· Log panel ····························································

    def _draw_log(self, top, left, h, w):
        self.scr.attron(curses.color_pair(C_DIM))
        self.scr.addstr(top, left, "─" * (w - 1))
        self.scr.addstr(top, left + 2, " Log ")
        self.scr.attroff(curses.color_pair(C_DIM))

        visible = h - 1
        start   = max(0, len(self.log) - visible)
        color_map = {
            "tx":   C_TX,
            "rx":   C_RX,
            "info": C_DIM,
            "warn": C_WARN,
            "err":  C_ERR,
        }
        for i, entry in enumerate(self.log[start:start + visible]):
            row = top + 1 + i
            if row >= top + h:
                break
            prefix = {"tx": ">>", "rx": "<<", "info": "--", "warn": "!!", "err": "EE"}.get(entry.direction, "  ")
            color  = color_map.get(entry.direction, C_NORMAL)
            line   = f" {entry.ts} {prefix} {entry.text}"
            self.scr.attron(curses.color_pair(color))
            try:
                self.scr.addstr(row, left, line[:w - 1])
            except curses.error:
                pass
            self.scr.attroff(curses.color_pair(color))


# ── String editing helper ─────────────────────────────────────────────────────

def _edit_str(s: str, key: int, max_len: int = 64, digits_only: bool = False) -> str:
    if key in (curses.KEY_BACKSPACE, 127, 8):
        return s[:-1]
    if 32 <= key <= 126:
        ch = chr(key)
        if digits_only and not ch.isdigit():
            return s
        if len(s) < max_len:
            return s + ch
    return s


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    def _run(stdscr):
        app = App(stdscr)
        app.run()

    try:
        curses.wrapper(_run)
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
