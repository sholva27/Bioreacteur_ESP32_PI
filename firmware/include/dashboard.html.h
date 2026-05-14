#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Probiotic Biofermenter</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px; background-color: #f4f7f6; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); padding: 20px; width: 300px; display: inline-block; margin: 10px; border-radius: 10px; vertical-align: top;}
    .chart-container { width: 80%; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); margin-top: 20px;}
    h2 { color: #003366; }
    .value { font-size: 2.5rem; font-weight: bold; color: #2c3e50; }
    .unit { font-size: 1.2rem; color: #7f8c8d; }
    button { padding: 12px 24px; font-size: 1rem; cursor: pointer; border-radius: 5px; border: none; background-color: #3498db; color: white; margin: 5px; transition: background 0.3s; }
    button:hover { background-color: #2980b9; }
    .btn-download { background-color: #27ae60; }
    .status-error { color: #e74c3c; font-weight: bold; }
    input { padding: 8px; width: 60px; margin: 5px; }
  </style>
</head>
<body>
  <h1>Probiotic Biofermenter Dashboard</h1>
  <div id="error-msg" class="status-error" style="display:none;">SENSOR ERROR DETECTED - SYSTEM IN FAILSAFE</div>

  <div class="card">
    <h2>pH Level</h2>
    <p><span class="value" id="ph">--</span> <span class="unit">pH</span></p>
  </div>

  <div class="card">
    <h2>Optical Density</h2>
    <p><span class="value" id="od">--</span> <span class="unit">AU</span></p>
  </div>

  <div class="card">
    <h2>Temperature</h2>
    <p><span class="value" id="temp">--</span> <span class="unit">°C</span></p>
  </div>

  <div class="card">
    <h2>Configuration</h2>
    <label>pH Target:</label> <input type="number" id="target-ph" step="0.1"><br>
    <button onclick="updateSettings()">Save Settings</button>
    <button class="btn-download" onclick="window.location.href='/download_log'">Download Log</button>
    <button onclick="togglePump('nutrient')">Manual Feed</button>
  </div>

  <div class="chart-container">
    <canvas id="bioChart"></canvas>
  </div>

  <script>
    var ctx = document.getElementById('bioChart').getContext('2d');
    var chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'pH',
                borderColor: '#3498db',
                data: [],
                yAxisID: 'y'
            }, {
                label: 'OD',
                borderColor: '#e67e22',
                data: [],
                yAxisID: 'y1'
            }]
        },
        options: {
            scales: {
                y: { type: 'linear', position: 'left' },
                y1: { type: 'linear', position: 'right', grid: { drawOnChartArea: false } }
            }
        }
    });

    function loadSettings() {
      fetch('/settings').then(r => r.json()).then(data => {
        document.getElementById('target-ph').value = data.phTarget;
      });
    }

    function updateSettings() {
      var settings = {
        phTarget: parseFloat(document.getElementById('target-ph').value)
      };
      fetch('/set', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(settings)
      });
    }

    setInterval(function ( ) {
      fetch('/data').then(r => r.json()).then(data => {
        document.getElementById("ph").innerHTML = data.ph.toFixed(2);
        document.getElementById("od").innerHTML = data.od.toFixed(3);
        document.getElementById("temp").innerHTML = data.temp.toFixed(1);

        // Update Chart
        var now = new Date().toLocaleTimeString();
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(data.ph);
        chart.data.datasets[1].data.push(data.od);
        if(chart.data.labels.length > 20) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
          chart.data.datasets[1].data.shift();
        }
        chart.update();

        if (data.error) document.getElementById("error-msg").style.display = "block";
        else document.getElementById("error-msg").style.display = "none";
      });
    }, 5000);

    function togglePump(pump) { fetch("/pump?type=" + pump); }
    window.onload = loadSettings;
  </script>
</body>
</html>
)rawliteral";

#endif
