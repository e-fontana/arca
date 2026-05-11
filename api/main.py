from flask import Flask, jsonify

from modules.rtc.router import bp as rtc_bp
from modules.rtc.service import sync_time, get_time, fmt_time


def create_app() -> Flask:
    app = Flask(__name__)
    app.register_blueprint(rtc_bp)

    @app.get("/health")
    def health():
        return jsonify({"status": "ok"})

    return app


if __name__ == "__main__":
    print("Sincronizando RTC com horário do sistema...")
    try:
        ok = sync_time()
        print("Sync", "OK" if ok else "FALHOU")
        data = get_time()
        print("RTC atual:", fmt_time(data))
    except Exception as e:
        print(f"Erro na inicialização: {e}")

    app = create_app()
    print("Servidor RTC em http://localhost:5000")
    app.run(host="0.0.0.0", port=5000)
