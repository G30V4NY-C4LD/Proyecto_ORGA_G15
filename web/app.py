from flask import Flask, render_template, jsonify
import serial
import threading
import time

app = Flask(__name__)

# Variables globales
ocupados = 0
libres = 0
espacios = ["desconocido"] * 4
arduino_disponible = False

# Intento de conexión al Arduino
try:
    arduino = serial.Serial('COM4', 9600, timeout=1)
    time.sleep(2)
    arduino_disponible = True
except:
    print("Arduino no disponible")

# Función que lee el serial continuamente
def leer_serial():
    global ocupados, libres, espacios, arduino_disponible
    while True:
        if arduino_disponible and arduino.in_waiting:
            linea = arduino.readline().decode().strip().lower()
            if "ocupados" in linea and "libres" in linea:
                try:
                    partes = linea.split(',')
                    datos = {}
                    for parte in partes:
                        key, val = parte.split(':')
                        datos[key.strip()] = int(val.strip())
                    ocupados = datos.get('ocupados', 0)
                    libres = datos.get('libres', 0)
                    espacios = ["ocupado"] * ocupados + ["libre"] * libres
                    while len(espacios) < 4:
                        espacios.append("desconocido")
                except Exception as e:
                    print("Error al procesar:", linea, e)
        else:
            ocupados = 0
            libres = 0
            espacios = ["desconocido"] * 4
        time.sleep(1)

# Hilo para lectura serial
threading.Thread(target=leer_serial, daemon=True).start()

@app.route("/")
def Home_web():
    return render_template("index.html")

@app.route("/estado_espacios")
def estado_espacios():
    return jsonify({
        "ocupados": ocupados,
        "libres": libres,
        "espacios": espacios
    })

if __name__ == "__main__":
    app.run(debug=True)
