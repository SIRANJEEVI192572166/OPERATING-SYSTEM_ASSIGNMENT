"""Small Flask stand-in for the University Result Portal."""
import hashlib
import os
import socket
from flask import Flask, jsonify

app = Flask(__name__)

@app.route("/")
def index():
    return "<h1>University Result Portal</h1><p>Served by %s</p>" % socket.gethostname()

@app.route("/health")
def health():
    return jsonify(status="ok")

@app.route("/result/<regno>")
def result(regno):
    # a little CPU work per request so that load tests are meaningful
    digest = hashlib.sha256(regno.encode()).hexdigest()
    for _ in range(200):
        digest = hashlib.sha256(digest.encode()).hexdigest()
    return jsonify(register_number=regno, grade="A", token=digest[:12])

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 8000)))
