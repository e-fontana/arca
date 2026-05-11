from flask import Blueprint, jsonify
from flask_cors import CORS

from . import service

bp = Blueprint("rtc", __name__, url_prefix="/api/rtc")
CORS(bp)


@bp.get("")
def get_rtc():
    try:
        data = service.get_time()
        return jsonify(data)
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@bp.post("/sync")
def sync_rtc():
    try:
        ok = service.sync_time()
        return jsonify({"synced": ok})
    except Exception as e:
        return jsonify({"error": str(e)}), 500
