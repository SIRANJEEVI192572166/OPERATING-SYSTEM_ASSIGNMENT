"""Second service: stand-in for the learning-management / online-exam system."""
import os
from flask import Flask, jsonify
app = Flask(__name__)

@app.route("/")
def index():
    return "<h1>Learning Management System</h1>"

@app.route("/health")
def health():
    return jsonify(status="ok", service="lms")

@app.route("/exam/<int:n>")
def exam(n):
    return jsonify(exam=n, questions=40)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 8000)))
