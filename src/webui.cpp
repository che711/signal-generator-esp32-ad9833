#include "webui.h"

// ─────────────────────────────────────────────────────────
// Embedded HTML
// ─────────────────────────────────────────────────────────
const char WebUI::_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>DDS Generator</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,sans-serif;background:#0f1117;color:#e2e8f0;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:20px 14px}
h1{font-size:1.3rem;font-weight:700;color:#a78bfa;margin-bottom:4px;letter-spacing:.06em}
.sub{font-size:.78rem;color:#64748b;margin-bottom:22px}
.dot{width:7px;height:7px;border-radius:50%;background:#34d399;display:inline-block;margin-right:5px;animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
.card{background:#1a1d2e;border:1px solid #2a2d45;border-radius:14px;padding:20px;width:100%;max-width:440px;margin-bottom:16px}
.card h2{font-size:.7rem;font-weight:700;letter-spacing:.12em;color:#4a5180;text-transform:uppercase;margin-bottom:14px}
.freq-big{font-size:2.6rem;font-weight:700;color:#a78bfa;text-align:center;padding:8px 0;font-variant-numeric:tabular-nums;letter-spacing:.02em}
.row{display:flex;gap:10px;margin-top:12px}
.freq-input{flex:1;padding:11px;border-radius:9px;border:2px solid #2a2d45;background:#11131f;color:#e2e8f0;font-size:1rem;text-align:center;outline:none;transition:border .15s}
.freq-input:focus{border-color:#a78bfa}
.btn{padding:11px 16px;border-radius:9px;border:none;font-size:.9rem;font-weight:600;cursor:pointer;transition:all .15s}
.btn-purple{background:#4c1d95;color:#e2e8f0}.btn-purple:hover{background:#5b21b6}
.btn-gray{background:#1e2335;color:#94a3b8;border:2px solid #2a2d45}.btn-gray:hover{border-color:#a78bfa;color:#a78bfa}
.btn-green{background:#065f46;color:#34d399}.btn-green:hover{background:#047857}
.wave-grid{display:grid;grid-template-columns:1fr 1fr;gap:9px}
.wbtn{padding:13px 8px;border:2px solid #2a2d45;border-radius:10px;background:transparent;color:#94a3b8;font-size:.9rem;font-weight:500;cursor:pointer;transition:all .15s;text-align:center}
.wbtn.active{border-color:#a78bfa;background:#2d1f5e;color:#c4b5fd}
.wbtn:hover:not(.active){border-color:#4a5180;color:#e2e8f0}
.wbtn .icon{display:block;font-size:1.1rem;margin-bottom:3px}
.step-wrap{display:flex;flex-wrap:wrap;gap:7px;justify-content:center}
.sbtn{padding:7px 12px;border:2px solid #2a2d45;border-radius:7px;background:transparent;color:#94a3b8;font-size:.78rem;cursor:pointer;transition:all .15s}
.sbtn.active{border-color:#34d399;background:#0d2e22;color:#34d399}
.sbtn:hover:not(.active){border-color:#4a5180;color:#e2e8f0}
.info{display:flex;justify-content:space-between;font-size:.78rem;padding:5px 0;color:#64748b;border-bottom:1px solid #1e2335}
.info:last-child{border:none}
.info span:last-child{color:#a78bfa;font-weight:600}
.save-row{display:flex;justify-content:flex-end;margin-top:10px}
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);background:#065f46;color:#34d399;padding:8px 20px;border-radius:8px;font-size:.85rem;opacity:0;transition:opacity .3s;pointer-events:none}
.toast.show{opacity:1}
</style>
</head>
<body>
<h1>&#9644; DDS Signal Generator</h1>
<p class="sub"><span class="dot"></span>ESP32 + AD9833</p>

<div class="card">
  <h2>Frequency</h2>
  <div class="freq-big" id="freqBig">—</div>
  <div class="row">
    <input class="freq-input" type="number" id="freqIn" placeholder="Hz" min="0.1" max="12000000">
    <button class="btn btn-purple" onclick="setFreq()">Set</button>
  </div>
  <div class="row">
    <button class="btn btn-gray" style="flex:1" onclick="nudge(-1)">&#9664; Step</button>
    <button class="btn btn-gray" style="flex:1" onclick="nudge(1)">Step &#9654;</button>
  </div>
</div>

<div class="card">
  <h2>Waveform</h2>
  <div class="wave-grid">
    <button class="wbtn" id="w0" onclick="setWave(0)"><span class="icon">&#8767;</span>Sine</button>
    <button class="wbtn" id="w1" onclick="setWave(1)"><span class="icon">/\/\</span>Triangle</button>
    <button class="wbtn" id="w2" onclick="setWave(2)"><span class="icon">&#8988;</span>Square</button>
    <button class="wbtn" id="w3" onclick="setWave(3)"><span class="icon">&#8988;</span>Square /2</button>
  </div>
</div>

<div class="card">
  <h2>Frequency Step</h2>
  <div class="step-wrap">
    <button class="sbtn" id="s0" onclick="setStep(0)">0.1 Hz</button>
    <button class="sbtn" id="s1" onclick="setStep(1)">1 Hz</button>
    <button class="sbtn" id="s2" onclick="setStep(2)">10 Hz</button>
    <button class="sbtn" id="s3" onclick="setStep(3)">100 Hz</button>
    <button class="sbtn" id="s4" onclick="setStep(4)">1 kHz</button>
    <button class="sbtn" id="s5" onclick="setStep(5)">10 kHz</button>
    <button class="sbtn" id="s6" onclick="setStep(6)">100 kHz</button>
    <button class="sbtn" id="s7" onclick="setStep(7)">1 MHz</button>
  </div>
</div>

<div class="card">
  <h2>Status</h2>
  <div class="info"><span>Frequency</span><span id="iFreq">—</span></div>
  <div class="info"><span>Waveform</span><span id="iWave">—</span></div>
  <div class="info"><span>Step</span><span id="iStep">—</span></div>
  <div class="save-row">
    <button class="btn btn-green" onclick="saveSettings()">&#128190; Save to memory</button>
  </div>
</div>

<div class="toast" id="toast"></div>

<script>
let curWave=0, curStep=4;

function fmt(hz){
  if(hz>=1e6) return (hz/1e6).toFixed(4)+' MHz';
  if(hz>=1e3) return (hz/1e3).toFixed(3)+' kHz';
  if(hz<1)    return hz.toFixed(1)+' Hz';
  return Math.round(hz)+' Hz';
}

function toast(msg,color='#34d399'){
  const t=document.getElementById('toast');
  t.textContent=msg; t.style.background=color==='err'?'#7f1d1d':'#065f46';
  t.style.color=color==='err'?'#fca5a5':'#34d399';
  t.classList.add('show');
  setTimeout(()=>t.classList.remove('show'),2000);
}

function applyStatus(d){
  document.getElementById('freqBig').textContent=fmt(d.freq);
  document.getElementById('freqIn').value=d.freq;
  document.getElementById('iFreq').textContent=fmt(d.freq);
  document.getElementById('iWave').textContent=d.wave;
  document.getElementById('iStep').textContent=d.step;
  for(let i=0;i<4;i++) document.getElementById('w'+i).classList.toggle('active',i===d.waveIdx);
  for(let i=0;i<8;i++) document.getElementById('s'+i).classList.toggle('active',i===d.stepIdx);
  curWave=d.waveIdx; curStep=d.stepIdx;
}

async function poll(){
  try{ const r=await fetch('/status'); applyStatus(await r.json()); }catch(e){}
}

async function setFreq(){
  const v=parseFloat(document.getElementById('freqIn').value);
  if(isNaN(v)||v<0.1||v>12000000){toast('Invalid frequency','err');return;}
  await fetch('/set/freq?v='+v);
  poll();
}

async function setWave(i){ await fetch('/set/wave?v='+i); poll(); }
async function setStep(i){ await fetch('/set/step?v='+i); poll(); }

async function nudge(dir){
  const steps=[0.1,1,10,100,1000,10000,100000,1000000];
  const cur=parseFloat(document.getElementById('freqIn').value)||1000;
  const nv=Math.max(0.1,Math.min(12000000,cur+dir*steps[curStep]));
  await fetch('/set/freq?v='+nv);
  poll();
}

async function saveSettings(){
  await fetch('/save');
  toast('Saved to memory!');
}

poll();
setInterval(poll,2000);
</script>
</body></html>
)rawhtml";

// ─────────────────────────────────────────────────────────

WebUI::WebUI(SignalGenerator& gen)
    : _gen(gen), _server(WEB_PORT), _connected(false), _lastWifiCheckMs(0)
{}

void WebUI::begin() {
    _connectWiFi();
    if (_connected) _startServer();
}

void WebUI::handle() {
    if (_connected) _server.handleClient();
}

// ── WiFi watchdog ─────────────────────────────────────────
void WebUI::checkWiFi() {
    uint32_t now = millis();
    if (now - _lastWifiCheckMs < WIFI_RECONNECT_MS) return;
    _lastWifiCheckMs = now;

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Connection lost, reconnecting...");
        _connected = false;
        WiFi.disconnect();
        delay(500);
        _connectWiFi();
        if (_connected) _startServer();
    }
}

String WebUI::ipAddress() const {
    return _connected ? WiFi.localIP().toString() : "No WiFi";
}

// ── Private ───────────────────────────────────────────────

void WebUI::_connectWiFi() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(400);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        _connected = true;
        Serial.printf("\n[WiFi] IP: %s\n",
                      WiFi.localIP().toString().c_str());
    } else {
        _connected = false;
        Serial.println("\n[WiFi] Failed — offline mode");
    }
}

void WebUI::_startServer() {
    if (MDNS.begin(MDNS_HOSTNAME))
        Serial.printf("[mDNS] http://%s.local\n", MDNS_HOSTNAME);

    _registerRoutes();
    _server.begin();
    Serial.printf("[Web] http://%s\n",
                  WiFi.localIP().toString().c_str());
}

void WebUI::_registerRoutes() {
    _server.on("/",         [this]() { _handleRoot();    });
    _server.on("/status",   [this]() { _handleStatus();  });
    _server.on("/set/freq", [this]() { _handleSetFreq(); });
    _server.on("/set/wave", [this]() { _handleSetWave(); });
    _server.on("/set/step", [this]() { _handleSetStep(); });
    _server.on("/save",     [this]() { _handleSave();    });
}

void WebUI::_handleRoot() {
    _server.send_P(200, "text/html", _HTML);
}

void WebUI::_handleStatus() {
    String j = "{";
    j += "\"freq\":"    + String(_gen.getFrequency(), 2) + ",";
    j += "\"wave\":\""  + String(_gen.waveLabel())       + "\",";
    j += "\"step\":\""  + String(_gen.stepLabel())       + "\",";
    j += "\"waveIdx\":" + String((int)_gen.getWave())    + ",";
    j += "\"stepIdx\":" + String((int)_gen.getStep());
    j += "}";
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", j);
}

void WebUI::_handleSetFreq() {
    if (_server.hasArg("v")) {
        float f = _server.arg("v").toFloat();
        _gen.setFrequency(f);
        Serial.printf("[Web] freq → %.2f Hz\n", f);
    }
    _server.send(200, "text/plain", "ok");
}

// ИСПРАВЛЕНО: использует setWaveByIndex — нет цикла, нет зависания
void WebUI::_handleSetWave() {
    if (_server.hasArg("v")) {
        int idx = _server.arg("v").toInt();
        _gen.setWaveByIndex(idx);
        Serial.printf("[Web] wave → %s\n", _gen.waveLabel());
    }
    _server.send(200, "text/plain", "ok");
}

// ИСПРАВЛЕНО: использует setStepByIndex — нет цикла, нет зависания
void WebUI::_handleSetStep() {
    if (_server.hasArg("v")) {
        int idx = _server.arg("v").toInt();
        _gen.setStepByIndex(idx);
        Serial.printf("[Web] step → %s\n", _gen.stepLabel());
    }
    _server.send(200, "text/plain", "ok");
}

// НОВОЕ: сохранение настроек в NVS по кнопке из браузера
void WebUI::_handleSave() {
    _gen.saveSettings();
    _server.send(200, "text/plain", "ok");
}
