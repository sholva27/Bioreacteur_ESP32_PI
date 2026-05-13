#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Probiotic Biofermenter</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); padding: 20px; width: 300px; display: inline-block; margin: 10px; border-radius: 10px;}
    h2 { color: #003366; }
    .value { font-size: 2rem; font-weight: bold; }
    .unit { font-size: 1.2rem; }
    button { padding: 10px 20px; font-size: 1rem; cursor: pointer; border-radius: 5px; border: none; background-color: #008CBA; color: white; }
    button:active { background-color: #005f7a; }
  </style>
</head>
<body>
  <h1>Probiotic Biofermenter Dashboard</h1>
  <div class="card">
    <h2>pH Level</h2>
    <p><span class="value" id="ph">--</span> <span class="unit">pH</span></p>
  </div>
  <div class="card">
    <h2>Optical Density</h2>
    <p><span class="value" id="od">--</span> <span class="unit">AU</span></p>
  </div>
  <div class="card">
    <h2>Control</h2>
    <button onclick="togglePump('nutrient')">Manual Feed</button>
  </div>
  <script>
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var data = JSON.parse(this.responseText);
          document.getElementById("ph").innerHTML = data.ph.toFixed(2);
          document.getElementById("od").innerHTML = data.od.toFixed(3);
        }
      };
      xhttp.open("GET", "/data", true);
      xhttp.send();
    }, 2000 ) ;

    function togglePump(pump) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/pump?type=" + pump, true);
      xhttp.send();
    }
  </script>
</body>
</html>
)rawliteral";

#endif
