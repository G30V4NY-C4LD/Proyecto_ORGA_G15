
console.log("Javascript funcionando.")

let datosOcupados = [];
let labelsHoras = [];
let miGrafica = null;

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
          //establece un icono segun el estado del parkeo.
          icon.className = estado === 'libre' ? 'fas fa-square-parking'
                      : estado === 'ocupado' ? 'fas fa-car-side'
                      : 'fas fa-question';


          div.appendChild(icon);
          grid.appendChild(div);
        });


        // Promedio
        const timestamp = new Date(); //crea un objeto con la fecha y hora actual del sistema.
        //crea una etiqueta de tiempo en formato HH:MM para mostrarla en el eje x de la grafica.
        const hora = timestamp.getHours() + ':' + String(timestamp.getMinutes()).padStart(2,'0');

        // Guardar valores, no guarda mas de 12 para que no crezca indefinidamente.
      if (datosOcupados.length >= 12) {
        datosOcupados.shift();  //elimina el primer dato del  array de ocupados(el mas viejo).
        labelsHoras.shift();  //elimina el primer label de hora 
      }

      //Agrega el numero de parqueos ocupados al final del arreglo datosOcupados.
      datosOcupados.push(data.ocupados);
      //Agrega la hora con su respectivo formato.
      labelsHoras.push(hora);

      // Calcular promedio del arreglo de datosOcupados.
      const promedio = datosOcupados.reduce((a, b) => a + b, 0) / datosOcupados.length;
      //muestra el promedio redondeado a dos decimales en el elemento con id promedio del html.
      document.getElementById('promedio').innerText = promedio.toFixed(2);

      //Actualizar gráfica
      if (miGrafica) {
        miGrafica.data.labels = labelsHoras;  //Actualiza las etiquetas del eje x(horas).
        miGrafica.data.datasets[0].data = datosOcupados;  //Actualiza los valores de datosOcupados.
        miGrafica.update(); 
      }


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
  .then(res => res.json(),
    document.getElementById("inputCodigo").value = "" // Limpia el campo
)
  .catch(error => {
    console.error("Error:", error)
  });
}


//Busca el camvas con el id graficaOcupados en el html.
const ctx = document.getElementById('graficaOcupados').getContext('2d');
//crea una nueva grafica de chart.js
miGrafica = new Chart(ctx, {
  type: 'line',
  data: {
    labels: labelsHoras,
    datasets: [{
      label: 'Parqueos Ocupados',
      data: datosOcupados,
      borderColor: 'rgba(75, 192, 192, 1)',
      backgroundColor: 'rgba(75, 192, 192, 0.2)',
      tension: 0.3
    }]
  },
  options: {
    responsive: true,
    scales: {
      y: { beginAtZero: true }
    }
  }
});




  // Ejecutar al cargar y luego cada 2 segundos
  setInterval(actualizarDatos, 500);
  actualizarDatos();