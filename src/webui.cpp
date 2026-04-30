#include "webui.h"

// ─────────────────────────────────────────────────────────
// Embedded HTML — v4 UI (25% larger, better readability)
// ─────────────────────────────────────────────────────────
const char WebUI::_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>DDS Generator</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#0b0d14;--surface:#13162a;--surface2:#1c2040;
  --border:#2e3460;--border2:#3d4575;
  --purple:#9d7ef5;--purple-d:#6d42d4;--purple-bg:#1e1545;
  --green:#2dd4a0;--green-bg:#0a2e22;
  --text:#dce4f5;--text2:#8892b0;--text3:#4a5575;
}
html{font-size:17px}
body{
  font-family:system-ui,-apple-system,sans-serif;
  background:var(--bg);color:var(--text);
  min-height:100vh;display:flex;flex-direction:column;
  align-items:center;padding:24px 16px 48px;
}
.header{text-align:center;margin-bottom:28px}
.header h1{
  font-size:1.6rem;font-weight:800;letter-spacing:.08em;
  color:var(--purple);margin-bottom:8px;
}
.badge{
  display:inline-flex;align-items:center;gap:6px;
  background:var(--surface);border:1px solid var(--border);
  border-radius:20px;padding:5px 14px;
  font-size:.82rem;color:var(--text2);
}
.dot{
  width:8px;height:8px;border-radius:50%;
  background:var(--green);box-shadow:0 0 6px var(--green);
  animation:pulse 2s infinite;
}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.2}}
.card{
  background:var(--surface);border:1px solid var(--border);
  border-radius:18px;padding:26px;
  width:100%;max-width:480px;margin-bottom:18px;
}
.card-title{
  font-size:.72rem;font-weight:700;
  letter-spacing:.14em;text-transform:uppercase;
  color:var(--text3);margin-bottom:18px;
  display:flex;align-items:center;gap:10px;
}
.card-title::after{content:'';flex:1;height:1px;background:var(--border)}
.freq-display{text-align:center;padding:8px 0 14px}
.freq-value{
  font-size:3.4rem;font-weight:800;color:var(--purple);
  font-variant-numeric:tabular-nums;letter-spacing:-.01em;line-height:1;
}
.freq-unit{font-size:1.4rem;font-weight:600;color:var(--text2);margin-left:5px}
.freq-raw{font-size:.82rem;color:var(--text3);margin-top:6px}
.input-row{display:flex;gap:10px;margin-top:16px}
.freq-input{
  flex:1;padding:14px 16px;border-radius:11px;
  border:2px solid var(--border);background:var(--bg);
  color:var(--text);font-size:1.1rem;text-align:center;
  outline:none;transition:border .2s;
}
.freq-input:focus{border-color:var(--purple)}
.freq-input::placeholder{color:var(--text3)}
.btn{
  padding:13px 20px;border-radius:11px;border:none;
  font-size:.95rem;font-weight:700;cursor:pointer;
  transition:all .15s;letter-spacing:.02em;
}
.btn-purple{background:var(--purple-d);color:#fff}
.btn-purple:hover{background:var(--purple);transform:translateY(-1px)}
.step-row{display:flex;gap:10px;margin-top:10px}
.btn-step{
  flex:1;padding:14px 8px;border-radius:11px;
  border:2px solid var(--border);background:var(--surface2);
  color:var(--text2);font-size:1rem;font-weight:600;
  cursor:pointer;transition:all .15s;
}
.btn-step:hover{border-color:var(--purple);color:var(--purple);background:var(--purple-bg)}
.wave-grid{display:grid;grid-template-columns:1fr 1fr;gap:11px}
.wbtn{
  padding:18px 10px 14px;border:2px solid var(--border);
  border-radius:13px;background:transparent;
  color:var(--text2);font-size:.95rem;font-weight:600;
  cursor:pointer;transition:all .15s;text-align:center;
}
.wbtn svg{display:block;margin:0 auto 9px;width:38px;height:22px}
.wbtn.active{border-color:var(--purple);background:var(--purple-bg);color:var(--purple)}
.wbtn:hover:not(.active){border-color:var(--border2);color:var(--text)}
.step-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:9px}
.sbtn{
  padding:13px 6px;border:2px solid var(--border);
  border-radius:10px;background:transparent;
  color:var(--text2);font-size:.85rem;font-weight:600;
  cursor:pointer;transition:all .15s;text-align:center;
}
.sbtn.active{border-color:var(--green);background:var(--green-bg);color:var(--green)}
.sbtn:hover:not(.active){border-color:var(--border2);color:var(--text)}
.stat-row{
  display:flex;justify-content:space-between;align-items:center;
  padding:13px 0;border-bottom:1px solid var(--border);font-size:.95rem;
}
.stat-row:last-of-type{border:none}
.stat-label{color:var(--text2);font-weight:500}
.stat-val{color:var(--purple);font-weight:700;font-size:1rem;font-variant-numeric:tabular-nums}
.btn-save{
  width:100%;padding:15px;border-radius:12px;
  border:2px solid var(--green-bg);
  background:var(--green-bg);color:var(--green);
  font-size:1rem;font-weight:700;cursor:pointer;
  transition:all .15s;letter-spacing:.03em;margin-top:16px;
}
.btn-save:hover{background:var(--green);color:#051a12;border-color:var(--green)}
.toast{
  position:fixed;bottom:24px;left:50%;
  transform:translateX(-50%);
  padding:11px 24px;border-radius:10px;
  font-size:.9rem;font-weight:600;
  opacity:0;transition:opacity .3s;pointer-events:none;white-space:nowrap;
}
.toast.ok{background:var(--green-bg);color:var(--green);border:1px solid var(--green)}
.toast.err{background:#2e0a0a;color:#f87171;border:1px solid #f87171}
.toast.show{opacity:1}
</style>
</head>
<body>

<div class="header">
  <h1>&#9646; DDS GENERATOR</h1>
  <div><span class="badge"><span class="dot"></span>ESP32 + AD9833 &middot; Online</span></div>
</div>

<div class="card">
  <div class="card-title">Frequency</div>
  <div class="freq-display">
    <div>
      <span class="freq-value" id="fVal">—</span><span class="freq-unit" id="fUnit"></span>
    </div>
    <div class="freq-raw" id="fRaw">—</div>
  </div>
  <div class="input-row">
    <input class="freq-input" type="number" id="freqIn"
           placeholder="Enter Hz" min="0.1" max="12000000">
    <button class="btn btn-purple" onclick="setFreq()">Set</button>
  </div>
  <div class="step-row">
    <button class="btn-step" onclick="nudge(-1)">&#9664;&ensp;Step</button>
    <button class="btn-step" onclick="nudge(1)">Step&ensp;&#9654;</button>
  </div>
</div>

<div class="card">
  <div class="card-title">Waveform</div>
  <div class="wave-grid">
    <button class="wbtn" id="w0" onclick="setWave(0)">
      <svg viewBox="0 0 38 22" fill="none">
        <path d="M2 11 C7 2,12 2,19 11 C26 20,31 20,36 11"
              stroke="#8892b0" stroke-width="2.2" stroke-linecap="round"/>
      </svg>Sine
    </button>
    <button class="wbtn" id="w1" onclick="setWave(1)">
      <svg viewBox="0 0 38 22" fill="none">
        <polyline points="2,19 10,3 19,19 28,3 36,19"
                  stroke="#8892b0" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/>
      </svg>Triangle
    </button>
    <button class="wbtn" id="w2" onclick="setWave(2)">
      <svg viewBox="0 0 38 22" fill="none">
        <polyline points="2,19 2,4 19,4 19,19 19,4 36,4 36,19"
                  stroke="#8892b0" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/>
      </svg>Square
    </button>
    <button class="wbtn" id="w3" onclick="setWave(3)">
      <svg viewBox="0 0 38 22" fill="none">
        <polyline points="2,19 2,4 11,4 11,19 11,4 19,4 19,19 19,4 28,4 28,19"
                  stroke="#8892b0" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/>
      </svg>Square /2
    </button>
  </div>
</div>

<div class="card">
  <div class="card-title">Frequency Step</div>
  <div class="step-grid">
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
  <div class="card-title">Status</div>
  <div class="stat-row">
    <span class="stat-label">Frequency</span>
    <span class="stat-val" id="stFreq">—</span>
  </div>
  <div class="stat-row">
    <span class="stat-label">Waveform</span>
    <span class="stat-val" id="stWave">—</span>
  </div>
  <div class="stat-row">
    <span class="stat-label">Step</span>
    <span class="stat-val" id="stStep" style="color:var(--green)">—</span>
  </div>
  <button class="btn-save" onclick="saveSettings()">&#128190;&ensp;Save to memory</button>
</div>

<div class="toast" id="toast"></div>

<script>
let curStep = 4;
const STEPS = [0.1,1,10,100,1000,10000,100000,1000000];

function fmtSplit(hz){
  if(hz>=1e6) return [(hz/1e6).toFixed(4),'MHz'];
  if(hz>=1e3) return [(hz/1e3).toFixed(3),'kHz'];
  if(hz<1)    return [hz.toFixed(1),'Hz'];
  return [Math.round(hz).toString(),'Hz'];
}

function toast(msg,type='ok'){
  const t=document.getElementById('toast');
  t.textContent=msg; t.className='toast '+type+' show';
  setTimeout(()=>t.classList.remove('show'),2200);
}

function applyStatus(d){
  const [v,u]=fmtSplit(d.freq);
  document.getElementById('fVal').textContent   = v;
  document.getElementById('fUnit').textContent  = ' '+u;
  document.getElementById('fRaw').textContent   = d.freq.toFixed(2)+' Hz';
  document.getElementById('freqIn').value       = d.freq;
  document.getElementById('stFreq').textContent = v+' '+u;
  document.getElementById('stWave').textContent = d.wave;
  document.getElementById('stStep').textContent = d.step;

  for(let i=0;i<4;i++)
    document.getElementById('w'+i).classList.toggle('active',i===d.waveIdx);
  for(let i=0;i<8;i++)
    document.getElementById('s'+i).classList.toggle('active',i===d.stepIdx);

  document.querySelectorAll('.wbtn').forEach((btn,i)=>{
    const c = i===d.waveIdx ? 'var(--purple)' : '#8892b0';
    btn.querySelectorAll('path,polyline').forEach(el=>el.setAttribute('stroke',c));
  });
  curStep=d.stepIdx;
}

async function poll(){
  try{ const r=await fetch('/status'); if(r.ok) applyStatus(await r.json()); }
  catch(e){}
}

async function setFreq(){
  const v=parseFloat(document.getElementById('freqIn').value);
  if(isNaN(v)||v<0.1||v>12000000){toast('Valid: 0.1 Hz – 12 MHz','err');return;}
  await fetch('/set/freq?v='+v); poll();
}

async function setWave(i){ await fetch('/set/wave?v='+i); poll(); }
async function setStep(i){ await fetch('/set/step?v='+i); poll(); }

async function nudge(dir){
  const cur=parseFloat(document.getElementById('freqIn').value)||1000;
  const nv=Math.max(0.1,Math.min(12000000,cur+dir*STEPS[curStep]));
  await fetch('/set/freq?v='+nv); poll();
}

async function saveSettings(){
  await fetch('/save'); toast('Saved to memory \u2713');
}

document.getElementById('freqIn')
  .addEventListener('keydown',e=>{ if(e.key==='Enter') setFreq(); });

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

void WebUI::checkWiFi() {
    uint32_t now = millis();
    if (now - _lastWifiCheckMs < WIFI_RECONNECT_MS) return;
    _lastWifiCheckMs = now;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Lost, reconnecting...");
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

void WebUI::_connectWiFi() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(400); Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        _connected = true;
        Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        _connected = false;
        Serial.println("\n[WiFi] Failed — offline");
    }
}

void WebUI::_startServer() {
    if (MDNS.begin(MDNS_HOSTNAME))
        Serial.printf("[mDNS] http://%s.local\n", MDNS_HOSTNAME);
    _registerRoutes();
    _server.begin();
    Serial.printf("[Web] http://%s\n", WiFi.localIP().toString().c_str());
}

void WebUI::_registerRoutes() {
    _server.on("/",         [this](){ _handleRoot();    });
    _server.on("/status",   [this](){ _handleStatus();  });
    _server.on("/set/freq", [this](){ _handleSetFreq(); });
    _server.on("/set/wave", [this](){ _handleSetWave(); });
    _server.on("/set/step", [this](){ _handleSetStep(); });
    _server.on("/save",     [this](){ _handleSave();    });
}

void WebUI::_handleRoot()   { _server.send_P(200, "text/html", _HTML); }

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
        _gen.setFrequency(_server.arg("v").toFloat());
        Serial.printf("[Web] freq → %.2f Hz\n", _gen.getFrequency());
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSetWave() {
    if (_server.hasArg("v")) {
        _gen.setWaveByIndex(_server.arg("v").toInt());
        Serial.printf("[Web] wave → %s\n", _gen.waveLabel());
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSetStep() {
    if (_server.hasArg("v")) {
        _gen.setStepByIndex(_server.arg("v").toInt());
        Serial.printf("[Web] step → %s\n", _gen.stepLabel());
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSave() {
    _gen.saveSettings();
    _server.send(200, "text/plain", "ok");
}
