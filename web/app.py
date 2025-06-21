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
ultimo_mensaje = ""

# Intento de conexión al Arduino
try:
    arduino = serial.Serial('COM5', 9600, timeout=1)
    time.sleep(2)
    arduino_disponible = True
except:
    print("Arduino no disponible")

# Función que lee el serial continuamente
def leer_serial():
    global ocupados, libres, espacios, arduino_disponible, ultimo_mensaje

    while True:
        if arduino_disponible and arduino.in_waiting:
            try:
                linea = arduino.readline().decode().strip().lower()
                print("Recibido del serial:", linea)
                if linea and linea != ultimo_mensaje and "ocupados" in linea and "libres" in linea:
                    print("Recibido del serial:", linea)
                    ultimo_mensaje = linea

                    partes = linea.replace(" ", "").split(",")
                    ocupados = int(partes[0].split(":")[1])
                    libres = int(partes[1].split(":")[1])
                    espacios = ["ocupado"] * ocupados + ["libre"] * libres
                    while len(espacios) < 4:
                        espacios.append("desconocido")
            except Exception as e:
                print("Error al procesar:", e)
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
