import datetime
import time
from functools import reduce
from operator import xor

from transport.serial_conn import get_connection, get_lock

_CMD_GET_TIME = 0x47  # 'G'
_CMD_SET_TIME = 0x53  # 'S'

_PROTO_START    = 0xAA
_PROTO_STOP     = 0x55
_PROTO_MAX_DATA = 57

_WEEKDAY_NAMES = ["", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]


def _build_frame(data: bytes) -> bytes:
    parity = reduce(xor, data, 0)
    return bytes([_PROTO_START, len(data)]) + data + bytes([parity, _PROTO_STOP])


def _read_frame(ser) -> bytes | None:
    deadline = time.time() + ser.timeout
    while time.time() < deadline:
        b = ser.read(1)
        if b and b[0] == _PROTO_START:
            break
    else:
        return None

    b = ser.read(1)
    if not b:
        return None
    n = b[0]
    if n == 0 or n > _PROTO_MAX_DATA:
        return None

    rest = ser.read(n + 2)
    if len(rest) != n + 2:
        return None

    data   = rest[:n]
    parity = rest[n]
    stop   = rest[n + 1]

    if stop != _PROTO_STOP or reduce(xor, data, 0) != parity:
        return None

    return bytes(data)


def get_time() -> dict:
    with get_lock():
        ser = get_connection()
        ser.reset_input_buffer()
        ser.write(_build_frame(bytes([_CMD_GET_TIME])))
        data = _read_frame(ser)
        if data is None or len(data) < 8:
            raise RuntimeError("Sem resposta válida do dispositivo")
        return {
            "hours":    data[0],
            "minutes":  data[1],
            "seconds":  data[2],
            "day":      data[3],
            "month":    data[4],
            "year":     2000 + data[5],
            "weekday":  _WEEKDAY_NAMES[data[6]] if 1 <= data[6] <= 7 else "?",
            "is_valid": data[7],
        }


def sync_time() -> bool:
    now = datetime.datetime.now()
    payload = bytes([
        _CMD_SET_TIME,
        now.hour, now.minute, now.second,
        now.day, now.month, now.year - 2000,
        now.isoweekday(),
    ])
    with get_lock():
        ser = get_connection()
        ser.reset_input_buffer()
        ser.write(_build_frame(payload))
        data = _read_frame(ser)
        return data is not None and len(data) >= 1 and data[0] == 0x06


def fmt_time(data: dict) -> str:
    return (
        f"{data['year']}-{data['month']:02d}-{data['day']:02d} "
        f"{data['hours']:02d}:{data['minutes']:02d}:{data['seconds']:02d} "
        f"({data['weekday']})  valid={data['is_valid']}"
    )
