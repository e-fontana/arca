import threading
import serial
from config import SERIAL_PORT, SERIAL_BAUD, SERIAL_TIMEOUT

_ser:  serial.Serial | None = None
_lock: threading.Lock        = threading.Lock()


def get_connection() -> serial.Serial:
    global _ser
    if _ser is None or not _ser.is_open:
        _ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=SERIAL_TIMEOUT)
    return _ser


def get_lock() -> threading.Lock:
    return _lock
