# Container image for the campus result-portal web service (a small Flask app).
FROM python:3.12-slim

# TODO 1: Set the working directory inside the image to /app
WORKDIR /app

# Install the dependencies first so that this layer is cached between builds.
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# TODO 2: Copy the application source into the image and run it as a non-root user named
#   portal
COPY app.py .
RUN useradd --create-home portal
USER portal

# TODO 3: Document that the service listens on port 8000 and start it with gunicorn on
#   0.0.0.0:8000 using the app object in app.py
EXPOSE 8000
CMD ["gunicorn", "--bind", "0.0.0.0:8000", "app:app"]
