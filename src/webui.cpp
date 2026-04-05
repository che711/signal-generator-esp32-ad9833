#include "webui.h"
#include "config.h"

// ─── Embedded HTML page ───────────────────────────────────
const char WebUI::_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html lang="ru"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>DDS Generator</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:system-ui,sans-serif;background:#0f1117;color:#e2e8f0;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:24px 16px}
  h1{font-size:1.4rem;font-weight:600;color:#a78bfa;margin-bottom:6px;letter-spacing:.05em}
  .sub{font-size:.8rem;color:#64748b;margin-bottom:28px}
  .card{background:#1e2130;border:1px solid #2d3148;border-radius:16px;padding:24px;width:100%;max-width:480px;margin-bottom:20px}
  .card h2{font-size:.75rem;font-weight:600;letter-spacing:.1em;color:#64748b;text-transform:uppercase;margin-bottom:16px}
  .freq-display{font-size:3rem;font-weight:700;color:#a78bfa;text-align:center;letter-spacing:.02em;padding:12px 0;font-variant-numeric:tabular-nums}
  .freq-unit{font-size:1.2rem;color:#7c86a2;margin-left:4px}
  .wave-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
  .wave-btn{padding:14px 10px;border:2px solid #2d3148;border-radius:12px;background:transparent;color:#94a3b8;font-size:.95rem;font-weight:500;cursor:pointer;transition:all .15s}
  .wave-btn.active{border-color:#a78bfa;background:#2d1f5e;color:#a78bfa}
  .wave-btn:hover:not(.active){border-color:#4a5180;color:#e2e8f0}
  .step-row{display:flex;flex-wrap:wrap;gap:8px;justify-content:center}
  .step-btn{padding:8px 14px;border:2px solid #2d3148;border-radius:8px;background:transparent;color:#94a3b8;font-size:.82rem;cursor:pointer;transition:all .15s}
  .step-btn.active{border-color:#34d399;background:#0d2e22;color:#34d399}
  .step-btn:hover:not(.active){border-color:#4a5180;color:#e2e8f0}
  .freq-controls{display:flex;align-items:center;gap:12px;margin-top:16px}
  .freq-input{flex:1;padding:12px;border-radius:10px;border:2px solid #2d3148;background:#141621;color:#e2e8f0;font-size:1.1rem;text-align:center;outline:none}
  .freq-input:focus{border-color:#a78bfa}
  .set-btn{padding:12px 20px;border-radius:10px;border:none;background:#4c1d95;color:#e2e8f0;font-size:.95rem;font-weight:600;cursor:pointer;transition:background .15s}
  .set-btn:hover{background:#5b21b6}
  .arrow-row{display:flex;gap:10px;margin-top:12px}
  .arrow-btn{flex:1;padding:14px;border-radius:10px;border:2px solid #2d3148;background:transparent;color:#94a3b8;font-size:1.4rem;cursor:pointer;transition:all .15s}
  .arrow-btn:hover{border-color:#a78bfa;color:#a78bfa;background:#2d1f5e}
  .status-dot{width:8px;height:8px;border-radius:50%;background:#34d399;display:inline-block;margin-right:6px;animation:pulse 2s infinite}
  @keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}
  .info-row{display:flex;justify-content:space-between;font-size:.8rem;color:#64748b;padding:4px 0}
</style>
</head>
<body>
<h1>&#9646; DDS Signal Generator</h1>
<p class="sub"><span class="status-dot"></span>ESP32 + AD9833 · Online</p>

<div class="card">
  <h2>Frequency</h2>
  <div class="freq-display" id="freqDisplay">---</div>
  <div class="freq-controls">
    <input class="freq-input" type="number" id="freqInput" placeholder="Hz" min="0.1" max="12000000" step="1">
    <button class="set-btn" onclick="setFreq()">Set</button>
  </div>
  <div class="arrow-row">
    <button class="arrow-btn" onclick="stepDir(-1)">&#9664; Step</button>
    <button class="arrow-btn" onclick="stepDir(1)">Step &#9654;</button>
  </div>
</div>

<div class="card">
  <h2>Waveform</h2>
  <div class="wave-grid">
    <button class="wave-btn" id="w0" onclick="setWave(0)">&#9675; Sine</button>
    <button class="wave-btn" id="w1" onclick="setWave(1)">&#9651; Triangle</button>
    <button class="wave-btn" id="w2" onclick="setWave(2)">&#9645; Square</button>
    <button class="wave-btn" id="w3" onclick="setWave(3)">&#9645; Square /2</button>
  </div>
</div>

<div class="card">
  <h2>Frequency Step</h2>
  <div class="step-row">
    <button class="step-btn" id="s0" onclick="setStep(0)">0.1 Hz</button>
    <button class="step-btn" id="s1" onclick="setStep(1)">1 Hz</button>
    <button class="step-btn" id="s2" onclick="setStep(2)">10 Hz</button>
    <button class="step-btn" id="s3" onclick="setStep(3)">100 Hz</button>
    <button class="step-btn" id="s4" onclick="setStep(4)">1 kHz</button>
    <button class="step-btn" id="s5" onclick="setStep(5)">10 kHz</button>
    <button class="step-btn" id="s6" onclick="setStep(6)">100 kHz</button>
    <button class="step-btn" id="s7" onclick="setStep(7)">1 MHz</button>
  </div>
</div>

<div class="card">
  <h2>Status</h2>
  <div class="info-row"><span>Frequency</span><span id="stFreq" style="color:#a78bfa">—</span></div>
  <div class="info-row"><span>Waveform</span><span id="stWave" style="color:#a78bfa">—</span></div>
  <div class="info-row"><span>Step</span><span id="stStep" style="color:#34d399">—</span></div>
</div>

<script>
let currentWave = 0;
let currentStep = 4;

function fmtFreq(hz) {
  if (hz >= 1e6) return (hz/1e6).toFixed(4) + ' MHz';
  if (hz >= 1e3) return (hz/1e3).toFixed(3) + ' kHz';
  if (hz < 1)   return hz.toFixed(1) + ' Hz';
  return Math.round(hz) + ' Hz';
}

function updateUI(data) {
  document.getElementById('freqDisplay').textContent = fmtFreq(data.freq);
  document.getElementById('freqInput').value = data.freq;
  document.getElementById('stFreq').textContent = fmtFreq(data.freq);
  document.getElementById('stWave').textContent = data.wave;
  document.getElementById('stStep').textContent = data.step;
  for(let i=0;i<4;i++) document.getElementById('w'+i).classList.toggle('active', i===data.waveIdx);
  for(let i=0;i<8;i++) document.getElementById('s'+i).classList.toggle('active', i===data.stepIdx);
  currentWave = data.waveIdx;
  currentStep = data.stepIdx;
}

async function fetchStatus() {
  try {
    const r = await fetch('/status');
    const d = await r.json();
    updateUI(d);
  } catch(e) {}
}

async function setFreq() {
  const v = parseFloat(document.getElementById('freqInput').value);
  if (isNaN(v)) return;
  await fetch('/set/freq?v=' + v);
  fetchStatus();
}

async function setWave(idx) {
  await fetch('/set/wave?v=' + idx);
  fetchStatus();
}

async function setStep(idx) {
  await fetch('/set/step?v=' + idx);
  fetchStatus();
}

async function stepDir(dir) {
  const step = [0.1,1,10,100,1000,10000,100000,1000000][currentStep];
  const cur = parseFloat(document.getElementById('freqInput').value) || 1000;
  const nv = Math.max(0.1, Math.min(12000000, cur + dir * step));
  document.getElementById('freqInput').value = nv;
  await fetch('/set/freq?v=' + nv);
  fetchStatus();
}

fetchStatus();
setInterval(fetchStatus, 2000);
</script>
</body></html>
)rawhtml";

// ─── Implementation ───────────────────────────────────────

WebUI::WebUI(SignalGenerator& gen)
    : _gen(gen), _server(WEB_PORT), _connected(false)
{}

void WebUI::begin() {
    _connectWiFi();
    if (_connected) {
        if (MDNS.begin(MDNS_HOSTNAME)) {
            Serial.printf("[mDNS] http://%s.local\n", MDNS_HOSTNAME);
        }
        _registerRoutes();
        _server.begin();
        Serial.printf("[Web] Server started at http://%s\n",
                      WiFi.localIP().toString().c_str());
    }
}

void WebUI::handle() {
    if (_connected) _server.handleClient();
}

String WebUI::ipAddress() const {
    if (!_connected) return "No WiFi";
    return WiFi.localIP().toString();
}

void WebUI::_connectWiFi() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        _connected = true;
        Serial.printf("\n[WiFi] Connected! IP: %s\n",
                      WiFi.localIP().toString().c_str());
    } else {
        _connected = false;
        Serial.println("\n[WiFi] Failed — running offline");
    }
}

void WebUI::_registerRoutes() {
    _server.on("/",           [this]() { _handleRoot();    });
    _server.on("/status",     [this]() { _handleStatus();  });
    _server.on("/set/freq",   [this]() { _handleSetFreq(); });
    _server.on("/set/wave",   [this]() { _handleSetWave(); });
    _server.on("/set/step",   [this]() { _handleSetStep(); });
}

void WebUI::_handleRoot() {
    _server.send_P(200, "text/html", _HTML);
}

void WebUI::_handleStatus() {
    String json = "{";
    json += "\"freq\":"   + String(_gen.getFrequency(), 2) + ",";
    json += "\"wave\":\""  + String(_gen.waveLabel()) + "\",";
    json += "\"step\":\""  + String(_gen.stepLabel()) + "\",";
    json += "\"waveIdx\":" + String((int)_gen.getWave()) + ",";
    json += "\"stepIdx\":" + String((int)_gen.getStep());
    json += "}";
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", json);
}

void WebUI::_handleSetFreq() {
    if (_server.hasArg("v")) {
        float f = _server.arg("v").toFloat();
        _gen.setFrequency(f);
        Serial.printf("[Web] freq → %.2f Hz\n", f);
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSetWave() {
    if (_server.hasArg("v")) {
        int w = _server.arg("v").toInt();
        for (int i = 0; i < w - (int)_gen.getWave(); i++) _gen.nextWave();
        // Cycle to target wave index
        int target = w % 4;
        while ((int)_gen.getWave() != target) _gen.nextWave();
        Serial.printf("[Web] wave → %s\n", _gen.waveLabel());
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSetStep() {
    if (_server.hasArg("v")) {
        int target = _server.arg("v").toInt() % 8;
        while ((int)_gen.getStep() != target) _gen.nextStep();
        Serial.printf("[Web] step → %s\n", _gen.stepLabel());
    }
    _server.send(200, "text/plain", "ok");
}
