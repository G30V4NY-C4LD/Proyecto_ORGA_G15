# import de las librerias a usar
from flask import Flask, render_template, jsonify, request
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
    arduino = serial.Serial('COM9', 9600, timeout=1)
    time.sleep(2)
    arduino_disponible = True
except:
    print("Arduino no disponible")

# Función que lee el puerto serial 
def leer_serial():
    global arduino, arduino_disponible, ocupados, libres, espacios, ultimo_mensaje

    while True:
        if arduino_disponible:
            try:
                if arduino.in_waiting:
                    linea = arduino.readline().decode().strip().lower()
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
                print("Error al procesar serial:", e)
                arduino_disponible = False
                arduino.close()
        else:
            # Intentar reconexión si no está disponible
            try:
                print("Intentando reconectar con Arduino...")
                arduino = serial.Serial('COM9', 9600, timeout=1)
                time.sleep(2)
                arduino_disponible = True
                print("Reconexión exitosa.")
            except Exception as e:
                print("Fallo la reconexión:", e)

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


@app.route("/modo_con_contrasena", methods=["POST"])
def modo_con_contrasena():
    data = request.get_json()
    modo = data.get("modo", "").lower()
    codigo = data.get("codigo", "")

    if not arduino_disponible:
        return jsonify({"respuesta": "Arduino no disponible"}), 503

    if codigo != "GHJL":
        comando = "AccesoDenegado\n"
        arduino.write(comando.encode())
        return jsonify({"respuesta": "Código incorrecto"})

    # Diccionario para mapear modo → texto a enviar
    modos = {
        "normal": "ModoNormalActivado",
        "panico": "ModoPanicoActivado",
        "mantenimiento": "ModoMantenimientoActivado",
        "nocturno": "ModoNocturnoActivado",
        "evacuacion": "ModoEvacuacionActivado"
    }

    # Si no está en los modos válidos
    if modo not in modos:
        return jsonify({"respuesta": "Modo inválido"}), 400

    mensaje = modos[modo] + "\n"
    arduino.write(mensaje.encode())

    # Leer respuesta del Arduino si hay
    time.sleep(0.5)

    return jsonify({"respuesta": f"Modo {modo} enviado correctamente"})

if __name__ == "__main__":
    app.run(debug=False)
