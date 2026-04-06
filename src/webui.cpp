#include "webui.h"
#include "config.h"

// ─── Embedded HTML page ───────────────────────────────────
const char WebUI::_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>DDS Generator</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Exo+2:wght@300;500;700&display=swap');

:root {
  --bg:       #080c10;
  --surface:  #0d1520;
  --border:   #1a2a3a;
  --glow:     #00e5ff;
  --green:    #00ff88;
  --red:      #ff3860;
  --dim:      #2a4a5a;
  --text:     #c8dde8;
  --muted:    #3a5a6a;
  --mono:     'Share Tech Mono', monospace;
  --sans:     'Exo 2', sans-serif;
}

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

body {
  font-family: var(--sans);
  background: var(--bg);
  color: var(--text);
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 20px 14px 40px;
  background-image:
    radial-gradient(ellipse 80% 40% at 50% -10%, rgba(0,229,255,.07) 0%, transparent 70%);
}

/* ── Header ── */
header {
  width: 100%;
  max-width: 500px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding-bottom: 18px;
  border-bottom: 1px solid var(--border);
  margin-bottom: 22px;
}
.logo {
  font-family: var(--mono);
  font-size: 1.1rem;
  color: var(--glow);
  letter-spacing: .12em;
  text-shadow: 0 0 12px rgba(0,229,255,.5);
}
.logo span { color: var(--muted); }
.status-pill {
  display: flex;
  align-items: center;
  gap: 7px;
  font-size: .72rem;
  color: var(--muted);
  letter-spacing: .06em;
  text-transform: uppercase;
}
.dot {
  width: 7px; height: 7px;
  border-radius: 50%;
  background: var(--green);
  box-shadow: 0 0 6px var(--green);
  animation: blink 2.2s ease-in-out infinite;
}
@keyframes blink { 0%,100%{opacity:1} 50%{opacity:.25} }

/* ── Cards ── */
.card {
  width: 100%;
  max-width: 500px;
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: 14px;
  padding: 20px;
  margin-bottom: 16px;
  position: relative;
  overflow: hidden;
}
.card::before {
  content: '';
  position: absolute;
  top: 0; left: 0; right: 0;
  height: 1px;
  background: linear-gradient(90deg, transparent, var(--glow), transparent);
  opacity: .18;
}
.card-title {
  font-size: .65rem;
  font-weight: 700;
  letter-spacing: .15em;
  text-transform: uppercase;
  color: var(--muted);
  margin-bottom: 16px;
}

/* ── Frequency display ── */
.freq-big {
  font-family: var(--mono);
  font-size: 3.2rem;
  color: var(--glow);
  text-align: center;
  letter-spacing: .04em;
  padding: 8px 0 14px;
  text-shadow: 0 0 20px rgba(0,229,255,.35);
  transition: color .3s;
}
.freq-big.beat-mode { color: var(--red); text-shadow: 0 0 20px rgba(255,56,96,.4); }

.freq-row {
  display: flex;
  gap: 10px;
  align-items: center;
}
.freq-input {
  flex: 1;
  padding: 11px 14px;
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 9px;
  color: var(--glow);
  font-family: var(--mono);
  font-size: 1rem;
  outline: none;
  transition: border-color .2s;
}
.freq-input:focus { border-color: var(--glow); }
.freq-input::placeholder { color: var(--muted); }

.btn-set {
  padding: 11px 18px;
  background: linear-gradient(135deg, #004466, #006688);
  border: 1px solid rgba(0,229,255,.3);
  border-radius: 9px;
  color: var(--glow);
  font-family: var(--sans);
  font-size: .85rem;
  font-weight: 700;
  letter-spacing: .07em;
  cursor: pointer;
  transition: all .15s;
  white-space: nowrap;
}
.btn-set:hover { background: linear-gradient(135deg, #005577, #007799); box-shadow: 0 0 10px rgba(0,229,255,.2); }

.step-row {
  display: flex;
  gap: 8px;
  margin-top: 12px;
}
.btn-step {
  flex: 1;
  padding: 12px;
  background: transparent;
  border: 1px solid var(--border);
  border-radius: 9px;
  color: var(--muted);
  font-size: 1.1rem;
  cursor: pointer;
  transition: all .15s;
}
.btn-step:hover { border-color: var(--glow); color: var(--glow); background: rgba(0,229,255,.05); }

/* ── Wave buttons ── */
.wave-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 9px;
}
.wave-btn {
  padding: 15px 10px;
  background: transparent;
  border: 1px solid var(--border);
  border-radius: 10px;
  color: var(--muted);
  font-family: var(--sans);
  font-size: .88rem;
  font-weight: 500;
  cursor: pointer;
  transition: all .18s;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
}
.wave-btn svg { opacity: .45; transition: opacity .18s; }
.wave-btn:hover { border-color: var(--dim); color: var(--text); }
.wave-btn:hover svg { opacity: .7; }
.wave-btn.active {
  border-color: var(--glow);
  background: rgba(0,229,255,.07);
  color: var(--glow);
  box-shadow: 0 0 12px rgba(0,229,255,.1);
}
.wave-btn.active svg { opacity: 1; }
.wave-btn.active.heart {
  border-color: var(--red);
  background: rgba(255,56,96,.07);
  color: var(--red);
  box-shadow: 0 0 12px rgba(255,56,96,.15);
}

/* ── BPM panel (shown only in heartbeat mode) ── */
.bpm-panel {
  display: none;
  margin-top: 16px;
  padding-top: 16px;
  border-top: 1px solid var(--border);
}
.bpm-panel.visible { display: block; }

.bpm-label {
  font-size: .65rem;
  letter-spacing: .15em;
  text-transform: uppercase;
  color: var(--muted);
  margin-bottom: 12px;
}
.bpm-controls {
  display: flex;
  align-items: center;
  gap: 10px;
}
.bpm-num {
  font-family: var(--mono);
  font-size: 2.4rem;
  color: var(--red);
  text-shadow: 0 0 14px rgba(255,56,96,.4);
  min-width: 80px;
  text-align: center;
}
.bpm-sub { font-size: .7rem; color: var(--muted); text-align: center; margin-top: -4px; }
.bpm-btn {
  width: 44px; height: 44px;
  border-radius: 50%;
  border: 1px solid var(--border);
  background: transparent;
  color: var(--muted);
  font-size: 1.3rem;
  cursor: pointer;
  transition: all .15s;
  display: flex;
  align-items: center;
  justify-content: center;
}
.bpm-btn:hover { border-color: var(--red); color: var(--red); background: rgba(255,56,96,.07); }

.bpm-slider {
  flex: 1;
  -webkit-appearance: none;
  height: 4px;
  border-radius: 2px;
  background: linear-gradient(90deg, var(--red) 0%, var(--red) var(--pct, 50%), var(--border) var(--pct, 50%));
  outline: none;
}
.bpm-slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 18px; height: 18px;
  border-radius: 50%;
  background: var(--red);
  box-shadow: 0 0 8px rgba(255,56,96,.5);
  cursor: pointer;
}

/* ── Heartbeat animation ── */
.hb-icon {
  font-size: 1.8rem;
  display: inline-block;
}
.hb-icon.beating {
  animation: heartbeat 0.8s ease-in-out infinite;
}
@keyframes heartbeat {
  0%   { transform: scale(1);    }
  14%  { transform: scale(1.22); }
  28%  { transform: scale(1);    }
  42%  { transform: scale(1.15); }
  70%  { transform: scale(1);    }
  100% { transform: scale(1);    }
}

/* ── Step buttons ── */
.step-grid {
  display: flex;
  flex-wrap: wrap;
  gap: 7px;
  justify-content: center;
}
.step-btn {
  padding: 8px 13px;
  background: transparent;
  border: 1px solid var(--border);
  border-radius: 8px;
  color: var(--muted);
  font-family: var(--mono);
  font-size: .78rem;
  cursor: pointer;
  transition: all .15s;
}
.step-btn:hover { border-color: var(--dim); color: var(--text); }
.step-btn.active {
  border-color: var(--green);
  background: rgba(0,255,136,.07);
  color: var(--green);
  box-shadow: 0 0 8px rgba(0,255,136,.12);
}

/* ── Status grid ── */
.status-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
}
.stat-box {
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 9px;
  padding: 12px 14px;
}
.stat-lbl { font-size: .6rem; letter-spacing: .12em; text-transform: uppercase; color: var(--muted); margin-bottom: 6px; }
.stat-val { font-family: var(--mono); font-size: 1rem; color: var(--glow); }
.stat-val.green { color: var(--green); }
.stat-val.red   { color: var(--red); }

/* ── Scope mini waveform SVGs ── */
.wave-svg { width: 52px; height: 22px; }
</style>
</head>
<body>

<header>
  <div class="logo"><span>//</span> DDS GEN</div>
  <div class="status-pill">
    <div class="dot"></div>
    ESP32 · AD9833
  </div>
</header>

<!-- ── Frequency card ─────────────────────────────── -->
<div class="card">
  <div class="card-title">Output Frequency</div>
  <div class="freq-big" id="freqDisplay">---</div>

  <div class="freq-row" id="freqRow">
    <input class="freq-input" type="number" id="freqInput"
           placeholder="Enter Hz…" min="0.1" max="12000000" step="1">
    <button class="btn-set" onclick="setFreq()">SET</button>
  </div>

  <div class="step-row" id="stepArrows">
    <button class="btn-step" onclick="stepDir(-1)">&#9664; Step</button>
    <button class="btn-step" onclick="stepDir(1)">Step &#9654;</button>
  </div>

  <!-- BPM panel — visible only in heartbeat mode -->
  <div class="bpm-panel" id="bpmPanel">
    <div class="bpm-label">Heart Rate</div>
    <div class="bpm-controls">
      <div>
        <div class="bpm-num" id="bpmNum">70</div>
        <div class="bpm-sub">BPM</div>
      </div>
      <button class="bpm-btn" onclick="changeBPM(-5)">&#8722;&#8722;</button>
      <button class="bpm-btn" onclick="changeBPM(-1)">&#8722;</button>
      <input class="bpm-slider" type="range" id="bpmSlider"
             min="30" max="200" value="70"
             oninput="onBPMSlider(this.value)">
      <button class="bpm-btn" onclick="changeBPM(1)">&#43;</button>
      <button class="bpm-btn" onclick="changeBPM(5)">&#43;&#43;</button>
    </div>
  </div>
</div>

<!-- ── Waveform card ──────────────────────────────── -->
<div class="card">
  <div class="card-title">Waveform</div>
  <div class="wave-grid">

    <button class="wave-btn" id="w0" onclick="setWave(0)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <path d="M2 11 Q8 2,13 11 Q18 20,23 11 Q28 2,33 11 Q38 20,43 11 Q48 2,50 11"/>
      </svg>
      Sine
    </button>

    <button class="wave-btn" id="w1" onclick="setWave(1)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <polyline points="2,11 13,2 26,20 39,2 50,11"/>
      </svg>
      Triangle
    </button>

    <button class="wave-btn" id="w2" onclick="setWave(2)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <polyline points="2,18 2,4 26,4 26,18 26,4 50,4 50,18"/>
      </svg>
      Square
    </button>

    <button class="wave-btn" id="w3" onclick="setWave(3)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <polyline points="2,18 2,4 14,4 14,18 26,18 26,4 38,4 38,18 50,18"/>
      </svg>
      Square /2
    </button>

    <button class="wave-btn heart" id="w4" onclick="setWave(4)"
            style="grid-column: span 2;">
      <span class="hb-icon" id="hbIcon">&#10084;&#65039;</span>
      Heartbeat
    </button>

  </div>
</div>

<!-- ── Frequency step card ────────────────────────── -->
<div class="card" id="stepCard">
  <div class="card-title">Frequency Step</div>
  <div class="step-grid">
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

<!-- ── Status card ────────────────────────────────── -->
<div class="card">
  <div class="card-title">Status</div>
  <div class="status-grid">
    <div class="stat-box">
      <div class="stat-lbl">Frequency</div>
      <div class="stat-val" id="stFreq">—</div>
    </div>
    <div class="stat-box">
      <div class="stat-lbl">Waveform</div>
      <div class="stat-val" id="stWave">—</div>
    </div>
    <div class="stat-box">
      <div class="stat-lbl">Step</div>
      <div class="stat-val green" id="stStep">—</div>
    </div>
    <div class="stat-box">
      <div class="stat-lbl">Heart Rate</div>
      <div class="stat-val red" id="stBPM">—</div>
    </div>
  </div>
</div>

<script>
let currentWave = 0;
let currentStep = 4;
let currentBPM  = 70;
let beatAnim    = null;

const STEPS = [0.1, 1, 10, 100, 1000, 10000, 100000, 1000000];

function fmtFreq(hz) {
  if (hz >= 1e6) return (hz / 1e6).toFixed(4) + ' MHz';
  if (hz >= 1e3) return (hz / 1e3).toFixed(3) + ' kHz';
  if (hz < 1)    return hz.toFixed(1) + ' Hz';
  return Math.round(hz) + ' Hz';
}

function updateUI(data) {
  const isHB = (data.waveIdx === 4);

  // ── Frequency display
  const display = document.getElementById('freqDisplay');
  if (isHB) {
    display.textContent = data.bpm + ' BPM';
    display.classList.add('beat-mode');
  } else {
    display.textContent = fmtFreq(data.freq);
    display.classList.remove('beat-mode');
  }

  // ── Freq input / step arrows
  document.getElementById('freqRow').style.display  = isHB ? 'none' : 'flex';
  document.getElementById('stepArrows').style.display = isHB ? 'none' : 'flex';
  document.getElementById('stepCard').style.display   = isHB ? 'none' : 'block';

  // ── BPM panel
  const bpmPanel = document.getElementById('bpmPanel');
  if (isHB) {
    bpmPanel.classList.add('visible');
    document.getElementById('bpmNum').textContent = data.bpm;
    const slider = document.getElementById('bpmSlider');
    slider.value = data.bpm;
    const pct = ((data.bpm - 30) / 170 * 100).toFixed(1);
    slider.style.setProperty('--pct', pct + '%');
  } else {
    bpmPanel.classList.remove('visible');
  }

  // ── Heartbeat animation
  const hbIcon = document.getElementById('hbIcon');
  if (isHB) {
    hbIcon.classList.add('beating');
    // sync animation speed to BPM
    const period = (60 / data.bpm).toFixed(2) + 's';
    hbIcon.style.animationDuration = period;
  } else {
    hbIcon.classList.remove('beating');
  }

  // ── Freq input value
  document.getElementById('freqInput').value = isHB ? '' : data.freq;

  // ── Status
  document.getElementById('stFreq').textContent = isHB ? data.bpm + ' BPM' : fmtFreq(data.freq);
  document.getElementById('stWave').textContent = data.wave;
  document.getElementById('stStep').textContent = data.step;
  document.getElementById('stBPM').textContent  = isHB ? data.bpm + ' BPM' : '—';

  // ── Wave buttons
  for (let i = 0; i < 5; i++) {
    document.getElementById('w' + i).classList.toggle('active', i === data.waveIdx);
  }

  // ── Step buttons
  for (let i = 0; i < 8; i++) {
    document.getElementById('s' + i).classList.toggle('active', i === data.stepIdx);
  }

  currentWave = data.waveIdx;
  currentStep = data.stepIdx;
  currentBPM  = data.bpm || 70;
}

async function fetchStatus() {
  try {
    const r = await fetch('/status');
    const d = await r.json();
    updateUI(d);
  } catch (e) {}
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
  const step = STEPS[currentStep];
  const cur  = parseFloat(document.getElementById('freqInput').value) || 1000;
  const nv   = Math.max(0.1, Math.min(12000000, cur + dir * step));
  document.getElementById('freqInput').value = nv;
  await fetch('/set/freq?v=' + nv);
  fetchStatus();
}

async function changeBPM(delta) {
  const nv = Math.max(30, Math.min(200, currentBPM + delta));
  await fetch('/set/bpm?v=' + nv);
  currentBPM = nv;
  fetchStatus();
}

let bpmTimer = null;
function onBPMSlider(val) {
  document.getElementById('bpmNum').textContent = val;
  clearTimeout(bpmTimer);
  bpmTimer = setTimeout(() => changeBPM(parseInt(val) - currentBPM), 120);
}

fetchStatus();
setInterval(fetchStatus, 2000);
</script>
</body>
</html>
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
    _server.on("/",         [this]() { _handleRoot();    });
    _server.on("/status",   [this]() { _handleStatus();  });
    _server.on("/set/freq", [this]() { _handleSetFreq(); });
    _server.on("/set/wave", [this]() { _handleSetWave(); });
    _server.on("/set/step", [this]() { _handleSetStep(); });
    _server.on("/set/bpm",  [this]() { _handleSetBPM();  });
}

void WebUI::_handleRoot() {
    _server.send_P(200, "text/html", _HTML);
}

void WebUI::_handleStatus() {
    String json = "{";
    json += "\"freq\":"    + String(_gen.getFrequency(), 2) + ",";
    json += "\"wave\":\""  + String(_gen.waveLabel())  + "\",";
    json += "\"step\":\""  + String(_gen.stepLabel())  + "\",";
    json += "\"waveIdx\":" + String((int)_gen.getWave()) + ",";
    json += "\"stepIdx\":" + String((int)_gen.getStep()) + ",";
    json += "\"bpm\":"     + String(_gen.getBPM());
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
        int target = _server.arg("v").toInt() % WAVE_COUNT;
        while ((int)_gen.getWave() != target) _gen.nextWave();
        Serial.printf("[Web] wave → %s\n", _gen.waveLabel());
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSetStep() {
    if (_server.hasArg("v")) {
        int target = _server.arg("v").toInt() % STEP_COUNT;
        while ((int)_gen.getStep() != target) _gen.nextStep();
        Serial.printf("[Web] step → %s\n", _gen.stepLabel());
    }
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleSetBPM() {
    if (_server.hasArg("v")) {
        int bpm = _server.arg("v").toInt();
        _gen.setBPM(bpm);
        Serial.printf("[Web] BPM → %d\n", _gen.getBPM());
    }
    _server.send(200, "text/plain", "ok");
}
