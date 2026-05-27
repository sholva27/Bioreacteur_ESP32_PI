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
    button:focus-visible { outline: 3px solid #f39c12; outline-offset: 2px; }
    button:disabled { background-color: #bdc3c7; cursor: not-allowed; }
    .btn-download { background-color: #27ae60; }
    .status-error { color: #e74c3c; font-weight: bold; }
    input { padding: 8px; width: 60px; margin: 5px; }
  </style>
</head>
<body>
  <h1>Probiotic Biofermenter Dashboard</h1>
  <div id="error-msg" class="status-error" role="alert" aria-live="assertive" style="display:none;">SENSOR ERROR DETECTED - SYSTEM IN FAILSAFE</div>

  <div class="card">
    <h2>pH Level</h2>
    <p><span class="value" id="ph">--</span> <span class="unit">pH</span></p>
  </div>

  <div class="card">
    <h2>Optical Density</h2>
    <p><span class="value" id="od">--</span> <span class="unit">AU</span></p>
  </div>

  <div class="card">
    <h2>Headspace Pressure</h2>
    <p><span class="value" id="pres-v">--</span> <span class="unit">V</span></p>
  </div>

  <div class="card">
    <h2>Temperature</h2>
    <p><span class="value" id="temp">--</span> <span class="unit">°C</span></p>
  </div>

  <div class="card">
    <h2>Growth Rate (µ)</h2>
    <p><span class="value" id="mu">--</span> <span class="unit">h⁻¹</span></p>
  </div>

  <div class="card">
    <h2>NADH (Metabolism)</h2>
    <p><span class="value" id="fluo">--</span> <span class="unit">RFU</span></p>
    <p><small>Riboflavin Noise: <span id="ribo">--</span></small></p>
  </div>

  <div class="card">
    <h2>UV Monitoring</h2>
    <p><span class="value" id="uv-v">--</span> <span class="unit">V</span></p>
  </div>

  <div class="card">
    <h2>Configuration</h2>
    <label>pH Target:</label> <input type="number" id="target-ph" step="0.1"><br>
    <label>Temp Target:</label> <input type="number" id="target-temp" step="0.5"><br>
    <label>Stirrer Speed (0-255):</label> <input type="number" id="stirrer-speed"><br>
    <label>Kp:</label> <input type="number" id="kp"><br>
    <label>Ki:</label> <input type="number" id="ki"><br>
    <label>Kd:</label> <input type="number" id="kd"><br>
    <hr>
    <label>Enable MQTT:</label> <input type="checkbox" id="mqtt-enabled"><br>
    <label>MQTT Broker:</label> <input type="text" id="mqtt-broker"><br>
    <button id="save-btn" onclick="updateSettings()" aria-label="Save all settings">Save Settings</button>
    <button class="btn-download" onclick="window.location.href='/download_log'">Download Log</button>
    <button onclick="togglePump('nutrient')">Manual Feed</button>
  </div>

  <div class="card">
    <h2>Calibration</h2>
    <p>pH (Current Volts: <span id="ph-v">--</span>)</p>
    <button onclick="calibratePH(7.0)">Calibrate pH 7.0</button>
    <button onclick="calibratePH(4.0)">Calibrate pH 4.0</button>
    <p>OD (Current Volts: <span id="od-v">--</span>)</p>
    <button onclick="calibrateODZero()">Set OD Blank (Zero)</button>
    <hr>
    <h3>Pump Calibration</h3>
    <select id="cal-pump-select">
      <option value="acid">Acid Pump</option>
      <option value="base">Base Pump</option>
      <option value="nutrient">Nutrient Pump</option>
    </select>
    <button onclick="runCalibrationPump()">Run for 60s</button>
    <br>
    <label>Measured Vol (mL):</label> <input type="number" id="cal-vol" step="0.1">
    <button onclick="savePumpCal()">Save Flow Rate</button>
  </div>

  <div class="chart-container">
    <canvas id="bioChart"></canvas>
  </div>

  <script>
    function provideBtnFeedback(btnId, loadingText, successText) {
      const btn = document.getElementById(btnId);
      const originalText = btn.innerHTML;
      btn.innerHTML = loadingText;
      btn.disabled = true;
      return (success = true) => {
        btn.innerHTML = success ? successText : "Error!";
        setTimeout(() => { btn.innerHTML = originalText; btn.disabled = false; }, 2000);
      };
    }

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
        document.getElementById('target-temp').value = data.tempTarget;
        document.getElementById('stirrer-speed').value = data.stirrerSpeed;
        document.getElementById('kp').value = data.kp;
        document.getElementById('ki').value = data.ki;
        document.getElementById('kd').value = data.kd;
        document.getElementById('mqtt-enabled').checked = data.mqttEnabled;
        document.getElementById('mqtt-broker').value = data.mqttBroker;
      });
    }

    var lastPh7V = 0;
    var lastPh4V = 0;

    function calibratePH(value) {
       fetch('/data').then(r => r.json()).then(data => {
          if (value == 7.0) {
            lastPh7V = data.ph_v;
            alert("pH 7.0 set to " + lastPh7V + "V. Now place in pH 4.0 and calibrate.");
            updateSettings({phOffset: -lastPh7V});
          } else if (value == 4.0) {
            lastPh4V = data.ph_v;
            var newSlope = 3.0 / (lastPh7V - lastPh4V);
            var newOffset = 0 - (lastPh7V * newSlope);
            updateSettings({phSlope: newSlope, phOffset: newOffset});
            alert("pH Calibrated! Slope: " + newSlope.toFixed(2));
          }
       });
    }

    function calibrateODZero() {
       fetch('/data').then(r => r.json()).then(data => {
          updateSettings({odZero: data.od_v});
          alert("OD Blank set to " + data.od_v + "V");
       });
    }

    function updateSettings(extra = {}) {
      const isManual = !Object.keys(extra).length;
      let done = isManual ? provideBtnFeedback('save-btn', 'Saving...', 'Saved!') : null;
      fetch('/settings').then(r => r.json()).then(data => {
        var settings = data;
        settings.mqttEnabled = document.getElementById('mqtt-enabled').checked;
        settings.mqttBroker = document.getElementById('mqtt-broker').value;
        settings.phTarget = parseFloat(document.getElementById('target-ph').value);
        settings.tempTarget = parseFloat(document.getElementById('target-temp').value);
        settings.stirrerSpeed = parseInt(document.getElementById('stirrer-speed').value);
        settings.kp = parseFloat(document.getElementById('kp').value);
        settings.ki = parseFloat(document.getElementById('ki').value);
        settings.kd = parseFloat(document.getElementById('kd').value);

        Object.assign(settings, extra);
        return fetch('/set', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify(settings)
        });
      }).then(() => done && done(true)).catch(() => done && done(false));
    }

    setInterval(function ( ) {
      fetch('/data').then(r => r.json()).then(data => {
        document.getElementById("ph").innerHTML = data.ph.toFixed(2);
        document.getElementById("od").innerHTML = data.od.toFixed(3);
        document.getElementById("temp").innerHTML = data.temp.toFixed(1);
        document.getElementById("mu").innerHTML = data.mu.toFixed(2);
        document.getElementById("fluo").innerHTML = data.fluo.toFixed(4);
        document.getElementById("ribo").innerHTML = data.ribo.toFixed(4);
        document.getElementById("uv-v").innerHTML = data.uv_v.toFixed(4);
        document.getElementById("pres-v").innerHTML = data.pres_v.toFixed(4);
        document.getElementById("ph-v").innerHTML = data.ph_v.toFixed(4);
        document.getElementById("od-v").innerHTML = data.od_v.toFixed(4);

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

    function runCalibrationPump() {
      var pump = document.getElementById('cal-pump-select').value;
      fetch("/pump?type=" + pump + "&duration=60000");
      alert("Pump running for 60 seconds. Collect liquid and measure volume.");
    }

    function savePumpCal() {
      var pump = document.getElementById('cal-pump-select').value;
      var vol = document.getElementById('cal-vol').value;
      var flowRate = vol / 60.0; // mL/s
      // This is a logic placeholder, actual storage can be added to settings
      alert("Flow rate for " + pump + " calculated as " + flowRate.toFixed(4) + " mL/s");
    }

    window.onload = loadSettings;
  </script>
</body>
</html>
)rawliteral";

#endif
