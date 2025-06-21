# import de las librerias a usar
from flask import Flask, render_template, jsonify
import serial
import threading
import time

app = Flask(__name__)

# Variables globales
ocupados = 0
libres = 0
espacios = ["desconocido"] * 4 # lista para los espacios
arduino_disponible = False # variable booleana para poder saber si el arduiono se conecto
ultimo_mensaje = "" # aguarda el ultimo mensaje para ver si hay cambio despues 

# Intento de conexión al Arduino
try:
    arduino = serial.Serial('COM5', 9600, timeout=1)
    time.sleep(2)
    arduino_disponible = True
except:
    print("Arduino no disponible")

# Función que lee el puerto serial 
def leer_serial():
    global ocupados, libres, espacios, arduino_disponible, ultimo_mensaje

    while True:
        if arduino_disponible and arduino.in_waiting:
            try:
                linea = arduino.readline().decode().strip().lower()
                if linea and linea != ultimo_mensaje and "ocupados" in linea and "libres" in linea: # que hay cambio en con respecto al ultimo estado, y que resibe en formato 'ocupado:#, libre:#'
                    print("Recibido del serial:", linea) # en terminal el contenido del serial
                    ultimo_mensaje = linea

                    partes = linea.replace(" ", "").split(",") # separa en partes lo de ocupado y libres para tomar los numeros y mandarlos al frontend
                    ocupados = int(partes[0].split(":")[1])
                    libres = int(partes[1].split(":")[1])
                    espacios = ["ocupado"] * ocupados + ["libre"] * libres #llena los espacios con ocupado o libre
                    while len(espacios) < 4:
                        espacios.append("desconocido") # si no sabe si esta libre u ocupado, ya sea porque el serial no se esta lleyendo bien
            except Exception as e:
                print("Error al procesar:", e)
        time.sleep(1)

# Hilo para lectura serial
threading.Thread(target=leer_serial, daemon=True).start()

@app.route("/")
def Home_web():
    return render_template("index.html")

@app.route("/estado_espacios")
def estado_espacios():  # los datos del formato json para mostrar el estado de los parqueos, y el total
    return jsonify({
        "ocupados": ocupados,
        "libres": libres,
        "espacios": espacios
    })

if __name__ == "__main__":
    app.run(debug=True)
