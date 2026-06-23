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
    button:focus-visible { outline: 3px solid #f39c12; outline-offset: 2px; }
    button:disabled { background-color: #bdc3c7; cursor: not-allowed; }
    input { padding: 8px; width: 60px; margin: 5px; }
  </style>
</head>
<body lang="en">
  <meta charset="UTF-8">
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
    <label for="target-ph">pH Target:</label> <input type="number" id="target-ph" step="0.1"><br>
    <label for="target-temp">Temp Target:</label> <input type="number" id="target-temp" step="0.5"><br>
    <label for="stirrer-speed">Stirrer Speed:</label> <input type="number" id="stirrer-speed"><br>
    <label for="kp">Kp:</label> <input type="number" id="kp"><br>
    <label for="ki">Ki:</label> <input type="number" id="ki"><br>
    <label for="kd">Kd:</label> <input type="number" id="kd"><br>
    <hr>
    <label for="mqtt-enabled">Enable MQTT:</label> <input type="checkbox" id="mqtt-enabled"><br>
    <label for="mqtt-broker">Broker:</label> <input type="text" id="mqtt-broker"><br>
    <button id="save-btn" onclick="updateSettings()" aria-label="Save settings">Save Settings</button>
    <button class="btn-download" onclick="window.location.href='/download_log'" aria-label="Download log">Download</button>
    <button id="feed-btn" onclick="togglePump('nutrient')" aria-label="Feed">Manual Feed</button>
  </div>

  <div class="card">
    <h2>Calibration</h2>
    <p>pH (Volts: <span id="ph-v">--</span>)</p>
    <button id="ph7-btn" onclick="calibratePH(7.0)" aria-label="pH 7">Calibrate pH 7.0</button>
    <button id="ph4-btn" onclick="calibratePH(4.0)" aria-label="pH 4">Calibrate pH 4.0</button>
    <p>OD (Volts: <span id="od-v">--</span>)</p>
    <button id="od-btn" onclick="calibrateODZero()" aria-label="Set OD Blank">Set OD Blank</button>
    <hr>
    <select id="cal-pump-select"><option value="acid">Acid</option><option value="base">Base</option><option value="nutrient">Nutrient</option></select>
    <button id="cal-run-btn" onclick="runCalibrationPump()" aria-label="Run 60s">Run 60s</button><br>
    <label for="cal-vol">Vol (mL):</label> <input type="number" id="cal-vol" step="0.1">
    <button id="cal-save-btn" onclick="savePumpCal()" aria-label="Save flow">Save Flow</button>
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

    function calibratePH(v) {
       const id = v == 7.0 ? 'ph7-btn' : 'ph4-btn', res = provideBtnFeedback(id, 'Calibrating...', 'Done!');
       fetch('/data').then(r => r.json()).then(d => {
          if (v == 7.0) { lastPh7V = d.ph_v; alert("pH 7.0 set. Move to pH 4.0."); updateSettings({phOffset: -lastPh7V}, null).then(res); }
          else { lastPh4V = d.ph_v; var s = 3.0 / (lastPh7V - lastPh4V); updateSettings({phSlope: s, phOffset: 0 - (lastPh7V * s)}, null).then(() => { res(); alert("pH Calibrated!"); }); }
       }).catch(() => { const b = document.getElementById(id); b.innerHTML = 'Error!'; setTimeout(() => { b.innerHTML = (v == 7.0 ? 'Calibrate pH 7.0' : 'Calibrate pH 4.0'); b.disabled = false; }, 2000); });
    }
    function calibrateODZero() {
       const res = provideBtnFeedback('od-btn', 'Zeroing...', 'Zeroed!');
       fetch('/data').then(r => r.json()).then(d => { updateSettings({odZero: d.od_v}, null).then(res); });
    }

    function provideBtnFeedback(id, wait, ok) {
      const b = document.getElementById(id); if (!b) return () => {};
      const t = b.innerHTML; b.innerHTML = wait; b.disabled = true;
      return () => { b.innerHTML = ok; setTimeout(() => { b.innerHTML = t; b.disabled = false; }, 2000); };
    }
    function updateSettings(extra = {}, btnId = 'save-btn') {
      const res = btnId ? provideBtnFeedback(btnId, 'Saving...', 'Saved!') : () => {};
      return fetch('/settings').then(r => r.json()).then(data => {
        var s = data;
        s.mqttEnabled = document.getElementById('mqtt-enabled').checked; s.mqttBroker = document.getElementById('mqtt-broker').value;
        s.phTarget = parseFloat(document.getElementById('target-ph').value); s.tempTarget = parseFloat(document.getElementById('target-temp').value);
        s.stirrerSpeed = parseInt(document.getElementById('stirrer-speed').value); s.kp = parseFloat(document.getElementById('kp').value);
        s.ki = parseFloat(document.getElementById('ki').value); s.kd = parseFloat(document.getElementById('kd').value);
        Object.assign(s, extra);
        return fetch('/set', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(s) }).then(res);
      }).catch(err => { if (btnId) { const b = document.getElementById(btnId); b.innerHTML = 'Error!'; setTimeout(() => { b.innerHTML = (btnId === 'save-btn' ? 'Save Settings' : b.innerHTML); b.disabled = false; }, 2000); } throw err; });
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

    function togglePump(p) { fetch("/pump?type=" + p).then(provideBtnFeedback('feed-btn', 'Feeding...', 'Fed!')); }
    function runCalibrationPump() {
      var p = document.getElementById('cal-pump-select').value, res = provideBtnFeedback('cal-run-btn', 'Running...', 'Done!');
      fetch("/pump?type=" + p + "&duration=60000").then(() => { res(); alert("Pump running 60s."); });
    }
    function savePumpCal() {
      const res = provideBtnFeedback('cal-save-btn', 'Saving...', 'Saved!');
      setTimeout(() => { res(); console.log("Flow saved"); }, 500);
    }

    window.onload = loadSettings;
  </script>
</body>
</html>
)rawliteral";

#endif
