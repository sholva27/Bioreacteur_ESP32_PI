#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html lang="en">
<head>
  <title>Probiotic Biofermenter</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px; background-color: #f4f7f6; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); padding: 20px; width: 300px; display: inline-block; margin: 10px; border-radius: 10px;}
    h2 { color: #003366; }
    .value { font-size: 2.5rem; font-weight: bold; color: #2c3e50; }
    .unit { font-size: 1.2rem; color: #7f8c8d; }
    button { padding: 12px 24px; font-size: 1rem; cursor: pointer; border-radius: 5px; border: none; background-color: #3498db; color: white; margin: 5px; transition: background 0.3s; }
    button:hover { background-color: #2980b9; }
    button:disabled { background-color: #95a5a6; cursor: not-allowed; opacity: 0.7; }
    .btn-download { background-color: #27ae60; }
    .btn-download:hover { background-color: #219150; }
    .status-error { color: #e74c3c; font-weight: bold; }
    button:focus-visible { outline: 3px solid #3498db; outline-offset: 2px; }
  </style>
</head>
<body>
  <h1>Probiotic Biofermenter Dashboard</h1>
  <div id="error-msg" class="status-error" style="display:none;" role="alert" aria-live="assertive">SENSOR ERROR DETECTED - SYSTEM IN FAILSAFE</div>

  <div class="card">
    <h2>pH Level</h2>
    <p><span class="value" id="ph">--</span> <span class="unit">pH</span></p>
  </div>

  <div class="card">
    <h2>Optical Density</h2>
    <p><span class="value" id="od">--</span> <span class="unit">AU</span></p>
  </div>

  <div class="card">
    <h2>Actions</h2>
    <button id="btn-feed" onclick="togglePump('nutrient')">Manual Feed</button>
    <button class="btn-download" onclick="window.location.href='/download_log'">Download CSV Log</button>
  </div>

  <script>
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var data = JSON.parse(this.responseText);
          document.getElementById("ph").innerHTML = data.ph.toFixed(2);
          document.getElementById("od").innerHTML = data.od.toFixed(3);
          if (data.error) {
            document.getElementById("error-msg").style.display = "block";
          } else {
            document.getElementById("error-msg").style.display = "none";
          }
        }
      };
      xhttp.open("GET", "/data", true);
      xhttp.send();
    }, 2000 ) ;

    function togglePump(pump) {
      var btn = document.getElementById("btn-feed");
      var originalText = btn.innerHTML;

      btn.disabled = true;
      btn.innerHTML = "Feeding...";

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status != 200) {
          btn.innerHTML = "Error!";
          setTimeout(function() {
            btn.disabled = false;
            btn.innerHTML = originalText;
          }, 2000);
        }
      };
      xhttp.open("GET", "/pump?type=" + pump, true);
      xhttp.send();

      // Re-enable after 5 seconds (matching FEEDING_DURATION_MS)
      setTimeout(function() {
        if (btn.innerHTML === "Feeding...") {
          btn.disabled = false;
          btn.innerHTML = originalText;
        }
      }, 5000);
    }
  </script>
</body>
</html>
)rawliteral";

#endif
