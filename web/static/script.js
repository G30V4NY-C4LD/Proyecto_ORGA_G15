
console.log("Javascript funcionando.")

function actualizarDatos() {
    fetch('/estado_espacios')
      .then(response => response.json())
      .then(data => {
        // Actualizar contadores
        document.getElementById('libres').innerText = data.libres;
        document.getElementById('ocupados').innerText = data.ocupados;

        // Actualizar grilla
        const grid = document.getElementById('parqueoGrid');
        grid.innerHTML = '';  // Limpiar grilla actual

        data.espacios.forEach(estado => {
          const div = document.createElement('div');
          div.className = 'espacio ' + estado;

          const icon = document.createElement('i');
          if (estado === 'libre') {
            icon.className = 'fas fa-square-parking';
          } else if (estado === 'ocupado') {
            icon.className = 'fas fa-car-side';
          } else {
            icon.className = 'fas fa-question';
          }

          div.appendChild(icon);
          grid.appendChild(div);
        });
      })
      .catch(error => {
        console.error("Error en fetch:", error);
      });
  }

  //funcion para ingresar a los modos
  function enviarModo(modo) {
  const codigo = document.getElementById("inputCodigo").value;

  fetch('/modo_con_contrasena', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ modo: modo, codigo: codigo })
  })
  .then(res => res.json())
  .then(data => {
    alert("Arduino responde: " + data.respuesta);
    document.getElementById("inputCodigo").value = ""; // Limpia el campo
  })
  .catch(error => {
    console.error("Error:", error);
    alert("Error al comunicarse con el servidor.");
  });
}

  // Ejecutar al cargar y luego cada 4 segundos
  setInterval(actualizarDatos, 4000);
  actualizarDatos();