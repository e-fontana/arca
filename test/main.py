import serial
import json
import threading
import datetime
import time
from flask import Flask, jsonify
from flask_cors import CORS

PORT    = "/dev/ttyUSB0"
BAUD    = 115200
TIMEOUT = 2

app = Flask(__name__)
CORS(app)

_ser  = None
_lock = threading.Lock()


def _get_serial() -> serial.Serial:
    global _ser
    if _ser is None or not _ser.is_open:
        _ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
    return _ser


def get_time() -> dict:
    with _lock:
        ser = _get_serial()
        ser.reset_input_buffer()
        ser.write(b'G')
        # firmware envia "Command received: G\n" antes do JSON — pula até '{'
        line = ser.readline().decode("utf-8").strip()
        while line and not line.startswith('{'):
            line = ser.readline().decode("utf-8").strip()
        return json.loads(line)


def sync_time() -> bool:
    now = datetime.datetime.now()
    with _lock:
        ser = _get_serial()
        ser.reset_input_buffer()

        # Fase 1: envia 'S' e aguarda firmware confirmar que está pronto
        ser.write(b'S')
        deadline = time.time() + 2.0
        ready = False
        while time.time() < deadline:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if "Command received: S" in line:
                ready = True
                break

        if not ready:
            return False

        # Fase 2: firmware está aguardando os 6 bytes do payload
        ser.write(bytes([
            now.hour, now.minute, now.second,
            now.day, now.month, now.year - 2000,
        ]))

        # Aguarda ACK 0x06
        deadline = time.time() + 1.0
        while time.time() < deadline:
            b = ser.read(1)
            if b == b'\x06':
                return True
            if b == b'\x15':
                return False
        return False


def _fmt(data: dict) -> str:
    return (
        f"{data['year']}-{data['month']:02d}-{data['day']:02d} "
        f"{data['hours']:02d}:{data['minutes']:02d}:{data['seconds']:02d} "
        f"({data['weekday']})  valid={data['is_valid']}"
    )


@app.route("/api/rtc", methods=["GET"])
def route_get_rtc():
    try:
        data = get_time()
        print("RTC:", _fmt(data))
        return jsonify(data)
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/api/rtc/sync", methods=["POST"])
def route_sync_rtc():
    try:
        ok = sync_time()
        now = datetime.datetime.now().isoformat(timespec="seconds")
        print(f"Sync {'OK' if ok else 'FALHOU'}  ({now})")
        return jsonify({"synced": ok})
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/health", methods=["GET"])
def health():
    return jsonify({"status": "ok"})


if __name__ == "__main__":
    print("Sincronizando RTC com horário do sistema...")
    try:
        ok = sync_time()
        print("Sync", "OK" if ok else "FALHOU (NAK)")
        data = get_time()
        print("RTC atual:", _fmt(data))
    except Exception as e:
        print(f"Erro na inicialização: {e}")

    print("Servidor RTC em http://localhost:5000")
    app.run(host="0.0.0.0", port=5000)
