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
  --bg:      #080c10;
  --surface: #0d1520;
  --border:  #1a2a3a;
  --glow:    #00e5ff;
  --green:   #00ff88;
  --red:     #ff3860;
  --amber:   #ffb830;
  --dim:     #2a4a5a;
  --text:    #c8dde8;
  --muted:   #3a5a6a;
  --mono:    'Share Tech Mono', monospace;
  --sans:    'Exo 2', sans-serif;
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
  background-image: radial-gradient(ellipse 80% 40% at 50% -10%, rgba(0,229,255,.07) 0%, transparent 70%);
}

header {
  width: 100%; max-width: 500px;
  display: flex; justify-content: space-between; align-items: center;
  padding-bottom: 18px;
  border-bottom: 1px solid var(--border);
  margin-bottom: 22px;
}
.logo { font-family: var(--mono); font-size: 1.1rem; color: var(--glow); letter-spacing: .12em; text-shadow: 0 0 12px rgba(0,229,255,.5); }
.logo span { color: var(--muted); }
.status-pill { display: flex; align-items: center; gap: 7px; font-size: .72rem; color: var(--muted); letter-spacing: .06em; text-transform: uppercase; }
.dot { width: 7px; height: 7px; border-radius: 50%; background: var(--green); box-shadow: 0 0 6px var(--green); animation: blink 2.2s ease-in-out infinite; }
@keyframes blink { 0%,100%{opacity:1} 50%{opacity:.25} }

.card {
  width: 100%; max-width: 500px;
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: 14px;
  padding: 20px;
  margin-bottom: 16px;
  position: relative; overflow: hidden;
}
.card::before {
  content: ''; position: absolute; top: 0; left: 0; right: 0; height: 1px;
  background: linear-gradient(90deg, transparent, var(--glow), transparent);
  opacity: .18;
}
.card-title {
  font-size: .65rem; font-weight: 700; letter-spacing: .15em;
  text-transform: uppercase; color: var(--muted); margin-bottom: 16px;
}

/* ── Frequency display ── */
.freq-big {
  font-family: var(--mono); font-size: 3.2rem; color: var(--glow);
  text-align: center; letter-spacing: .04em; padding: 8px 0 14px;
  text-shadow: 0 0 20px rgba(0,229,255,.35); transition: color .3s;
}
.freq-big.beat-mode { color: var(--red); text-shadow: 0 0 20px rgba(255,56,96,.4); }

.freq-row { display: flex; gap: 10px; align-items: center; }
.freq-input {
  flex: 1; padding: 11px 14px; background: var(--bg);
  border: 1px solid var(--border); border-radius: 9px;
  color: var(--glow); font-family: var(--mono); font-size: 1rem;
  outline: none; transition: border-color .2s;
}
.freq-input:focus { border-color: var(--glow); }
.freq-input::placeholder { color: var(--muted); }
.btn-set {
  padding: 11px 18px; background: linear-gradient(135deg,#004466,#006688);
  border: 1px solid rgba(0,229,255,.3); border-radius: 9px;
  color: var(--glow); font-family: var(--sans); font-size: .85rem;
  font-weight: 700; letter-spacing: .07em; cursor: pointer; transition: all .15s; white-space: nowrap;
}
.btn-set:hover { background: linear-gradient(135deg,#005577,#007799); box-shadow: 0 0 10px rgba(0,229,255,.2); }

.step-row { display: flex; gap: 8px; margin-top: 12px; }
.btn-step {
  flex: 1; padding: 12px; background: transparent;
  border: 1px solid var(--border); border-radius: 9px;
  color: var(--muted); font-size: 1.1rem; cursor: pointer; transition: all .15s;
}
.btn-step:hover { border-color: var(--glow); color: var(--glow); background: rgba(0,229,255,.05); }

/* ── Wave buttons ── */
.wave-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 9px; }
.wave-btn {
  padding: 15px 10px; background: transparent;
  border: 1px solid var(--border); border-radius: 10px;
  color: var(--muted); font-family: var(--sans); font-size: .88rem;
  font-weight: 500; cursor: pointer; transition: all .18s;
  display: flex; flex-direction: column; align-items: center; gap: 8px;
}
.wave-btn svg { opacity: .45; transition: opacity .18s; }
.wave-btn:hover { border-color: var(--dim); color: var(--text); }
.wave-btn:hover svg { opacity: .7; }
.wave-btn.active { border-color: var(--glow); background: rgba(0,229,255,.07); color: var(--glow); box-shadow: 0 0 12px rgba(0,229,255,.1); }
.wave-btn.active svg { opacity: 1; }
.wave-btn.active.heart { border-color: var(--red); background: rgba(255,56,96,.07); color: var(--red); box-shadow: 0 0 12px rgba(255,56,96,.15); }

/* ── BPM panel ── */
.bpm-panel { display: none; margin-top: 16px; padding-top: 16px; border-top: 1px solid var(--border); }
.bpm-panel.visible { display: block; }
.bpm-label { font-size: .65rem; letter-spacing: .15em; text-transform: uppercase; color: var(--muted); margin-bottom: 12px; }
.bpm-controls { display: flex; align-items: center; gap: 10px; }
.bpm-num { font-family: var(--mono); font-size: 2.4rem; color: var(--red); text-shadow: 0 0 14px rgba(255,56,96,.4); min-width: 80px; text-align: center; }
.bpm-sub { font-size: .7rem; color: var(--muted); text-align: center; margin-top: -4px; }
.bpm-btn { width: 44px; height: 44px; border-radius: 50%; border: 1px solid var(--border); background: transparent; color: var(--muted); font-size: 1.3rem; cursor: pointer; transition: all .15s; display: flex; align-items: center; justify-content: center; }
.bpm-btn:hover { border-color: var(--red); color: var(--red); background: rgba(255,56,96,.07); }
.bpm-slider { flex: 1; -webkit-appearance: none; height: 4px; border-radius: 2px; background: linear-gradient(90deg, var(--red) 0%, var(--red) var(--pct,50%), var(--border) var(--pct,50%)); outline: none; }
.bpm-slider::-webkit-slider-thumb { -webkit-appearance: none; width: 18px; height: 18px; border-radius: 50%; background: var(--red); box-shadow: 0 0 8px rgba(255,56,96,.5); cursor: pointer; }
.hb-icon { font-size: 1.8rem; display: inline-block; }
.hb-icon.beating { animation: heartbeat 0.8s ease-in-out infinite; }
@keyframes heartbeat { 0%{transform:scale(1)} 14%{transform:scale(1.22)} 28%{transform:scale(1)} 42%{transform:scale(1.15)} 70%{transform:scale(1)} }

/* ── Step buttons ── */
.step-grid { display: flex; flex-wrap: wrap; gap: 7px; justify-content: center; }
.step-btn { padding: 8px 13px; background: transparent; border: 1px solid var(--border); border-radius: 8px; color: var(--muted); font-family: var(--mono); font-size: .78rem; cursor: pointer; transition: all .15s; }
.step-btn:hover { border-color: var(--dim); color: var(--text); }
.step-btn.active { border-color: var(--green); background: rgba(0,255,136,.07); color: var(--green); box-shadow: 0 0 8px rgba(0,255,136,.12); }

/* ── System info ── */
.sysinfo-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
}
.sys-box {
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 9px;
  padding: 12px 14px;
  position: relative;
}
.sys-lbl {
  font-size: .58rem; letter-spacing: .13em;
  text-transform: uppercase; color: var(--muted); margin-bottom: 6px;
}
.sys-val {
  font-family: var(--mono); font-size: 1.05rem; color: var(--glow);
  transition: color .4s;
}
.sys-val.warn  { color: var(--amber); }
.sys-val.hot   { color: var(--red);   }
.sys-val.green { color: var(--green); }

/* Bar gauge under value */
.bar-track {
  height: 3px; border-radius: 2px;
  background: var(--border); margin-top: 8px; overflow: hidden;
}
.bar-fill {
  height: 100%; border-radius: 2px;
  background: var(--glow);
  transition: width .6s ease, background .4s;
  width: 0%;
}
.bar-fill.warn { background: var(--amber); }
.bar-fill.hot  { background: var(--red);   }

/* Signal summary row — spans full width */
.sig-summary {
  grid-column: span 2;
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 9px;
  padding: 12px 16px;
  display: flex; justify-content: space-between; align-items: center;
  flex-wrap: wrap; gap: 10px;
}
.sig-chip {
  display: flex; flex-direction: column; align-items: center; gap: 3px;
}
.sig-chip .lbl { font-size: .55rem; letter-spacing: .12em; text-transform: uppercase; color: var(--muted); }
.sig-chip .val { font-family: var(--mono); font-size: .92rem; color: var(--glow); }
.sig-chip .val.red   { color: var(--red);   }
.sig-chip .val.green { color: var(--green); }
.sig-chip .val.amber { color: var(--amber); }

/* uptime */
.uptime-val { font-family: var(--mono); font-size: .95rem; color: var(--green); }

.wave-svg { width: 52px; height: 22px; }
</style>
</head>
<body>

<header>
  <div class="logo"><span>//</span> DDS GEN</div>
  <div class="status-pill"><div class="dot"></div>ESP32 · AD9833</div>
</header>

<!-- ── Frequency ── -->
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
  <div class="bpm-panel" id="bpmPanel">
    <div class="bpm-label">Heart Rate</div>
    <div class="bpm-controls">
      <div><div class="bpm-num" id="bpmNum">70</div><div class="bpm-sub">BPM</div></div>
      <button class="bpm-btn" onclick="changeBPM(-5)">&#8722;&#8722;</button>
      <button class="bpm-btn" onclick="changeBPM(-1)">&#8722;</button>
      <input class="bpm-slider" type="range" id="bpmSlider" min="30" max="200" value="70" oninput="onBPMSlider(this.value)">
      <button class="bpm-btn" onclick="changeBPM(1)">&#43;</button>
      <button class="bpm-btn" onclick="changeBPM(5)">&#43;&#43;</button>
    </div>
  </div>
</div>

<!-- ── Waveform ── -->
<div class="card">
  <div class="card-title">Waveform</div>
  <div class="wave-grid">
    <button class="wave-btn" id="w0" onclick="setWave(0)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <path d="M2 11 Q8 2,13 11 Q18 20,23 11 Q28 2,33 11 Q38 20,43 11 Q48 2,50 11"/>
      </svg>Sine
    </button>
    <button class="wave-btn" id="w1" onclick="setWave(1)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <polyline points="2,11 13,2 26,20 39,2 50,11"/>
      </svg>Triangle
    </button>
    <button class="wave-btn" id="w2" onclick="setWave(2)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <polyline points="2,18 2,4 26,4 26,18 26,4 50,4 50,18"/>
      </svg>Square
    </button>
    <button class="wave-btn" id="w3" onclick="setWave(3)">
      <svg class="wave-svg" viewBox="0 0 52 22" fill="none" stroke="currentColor" stroke-width="1.5">
        <polyline points="2,18 2,4 14,4 14,18 26,18 26,4 38,4 38,18 50,18"/>
      </svg>Square /2
    </button>
    <button class="wave-btn heart" id="w4" onclick="setWave(4)" style="grid-column:span 2">
      <span class="hb-icon" id="hbIcon">&#10084;&#65039;</span>Heartbeat
    </button>
  </div>
</div>

<!-- ── Step ── -->
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

<!-- ── System Info ── -->
<div class="card">
  <div class="card-title">System &amp; Signal</div>
  <div class="sysinfo-grid">

    <!-- CPU temp -->
    <div class="sys-box">
      <div class="sys-lbl">&#127777; CPU Temperature</div>
      <div class="sys-val" id="sysTemp">—</div>
      <div class="bar-track"><div class="bar-fill" id="barTemp"></div></div>
    </div>

    <!-- Heap -->
    <div class="sys-box">
      <div class="sys-lbl">&#128190; Free Heap</div>
      <div class="sys-val" id="sysHeap">—</div>
      <div style="font-family:var(--mono);font-size:.7rem;color:var(--muted);margin-top:3px" id="sysHeapPct">—</div>
      <div class="bar-track"><div class="bar-fill" id="barHeap"></div></div>
    </div>

    <!-- CPU freq -->
    <div class="sys-box">
      <div class="sys-lbl">&#9889; CPU Clock</div>
      <div class="sys-val green" id="sysCpu">—</div>
      <div style="font-family:var(--mono);font-size:.7rem;color:var(--muted);margin-top:3px" id="sysCpuPct">—</div>
      <div class="bar-track"><div class="bar-fill" id="barCpu" style="background:var(--green)"></div></div>
    </div>

    <!-- Uptime -->
    <div class="sys-box">
      <div class="sys-lbl">&#9200; Uptime</div>
      <div class="sys-val uptime-val" id="sysUptime">—</div>
    </div>

    <!-- Signal summary — full width -->
    <div class="sig-summary">
      <div class="sig-chip">
        <div class="lbl">Waveform</div>
        <div class="val" id="sigWave">—</div>
      </div>
      <div class="sig-chip">
        <div class="lbl">Frequency</div>
        <div class="val amber" id="sigFreq">—</div>
      </div>
      <div class="sig-chip">
        <div class="lbl">Step</div>
        <div class="val green" id="sigStep">—</div>
      </div>
      <div class="sig-chip">
        <div class="lbl">Period</div>
        <div class="val" id="sigPeriod">—</div>
      </div>
      <div class="sig-chip">
        <div class="lbl">Heart Rate</div>
        <div class="val red" id="sigBPM">—</div>
      </div>
    </div>

  </div>
</div>

<script>
let currentWave = 0, currentStep = 4, currentBPM = 70;
const STEPS = [0.1,1,10,100,1000,10000,100000,1000000];

function fmtFreq(hz) {
  if (hz >= 1e6) return (hz/1e6).toFixed(4)+' MHz';
  if (hz >= 1e3) return (hz/1e3).toFixed(3)+' kHz';
  if (hz < 1)   return hz.toFixed(1)+' Hz';
  return Math.round(hz)+' Hz';
}

function fmtPeriod(hz) {
  if (!hz || hz <= 0) return '—';
  const s = 1 / hz;
  if (s >= 1)    return s.toFixed(3)+' s';
  if (s >= 1e-3) return (s*1e3).toFixed(3)+' ms';
  if (s >= 1e-6) return (s*1e6).toFixed(3)+' µs';
  return (s*1e9).toFixed(1)+' ns';
}

function fmtUptime(ms) {
  const s = Math.floor(ms/1000);
  const h = Math.floor(s/3600), m = Math.floor((s%3600)/60), ss = s%60;
  if (h > 0) return `${h}h ${m}m`;
  if (m > 0) return `${m}m ${ss}s`;
  return `${ss}s`;
}

function updateUI(d) {
  const isHB = (d.waveIdx === 4);

  // Frequency display
  const disp = document.getElementById('freqDisplay');
  disp.textContent = isHB ? d.bpm+' BPM' : fmtFreq(d.freq);
  disp.classList.toggle('beat-mode', isHB);

  // Show/hide panels
  document.getElementById('freqRow').style.display    = isHB ? 'none':'flex';
  document.getElementById('stepArrows').style.display = isHB ? 'none':'flex';
  document.getElementById('stepCard').style.display   = isHB ? 'none':'block';

  // BPM panel
  const bpmPanel = document.getElementById('bpmPanel');
  bpmPanel.classList.toggle('visible', isHB);
  if (isHB) {
    document.getElementById('bpmNum').textContent = d.bpm;
    const slider = document.getElementById('bpmSlider');
    slider.value = d.bpm;
    slider.style.setProperty('--pct', ((d.bpm-30)/170*100).toFixed(1)+'%');
  }

  // Heartbeat icon
  const hbIcon = document.getElementById('hbIcon');
  hbIcon.classList.toggle('beating', isHB);
  if (isHB) hbIcon.style.animationDuration = (60/d.bpm).toFixed(2)+'s';

  document.getElementById('freqInput').value = isHB ? '' : d.freq;

  // Wave / step buttons
  for(let i=0;i<5;i++) document.getElementById('w'+i).classList.toggle('active', i===d.waveIdx);
  for(let i=0;i<8;i++) document.getElementById('s'+i).classList.toggle('active', i===d.stepIdx);

  currentWave = d.waveIdx; currentStep = d.stepIdx; currentBPM = d.bpm||70;

  // Signal summary
  document.getElementById('sigWave').textContent   = d.wave;
  document.getElementById('sigFreq').textContent   = isHB ? d.bpm+' BPM' : fmtFreq(d.freq);
  document.getElementById('sigStep').textContent   = isHB ? '—' : d.step;
  document.getElementById('sigPeriod').textContent = isHB ? (60/d.bpm).toFixed(2)+'s' : fmtPeriod(d.freq);
  document.getElementById('sigBPM').textContent    = isHB ? d.bpm+' BPM' : '—';
}

function updateSys(s) {
  // Temperature
  const t = s.tempC;
  const tempEl  = document.getElementById('sysTemp');
  const barTemp = document.getElementById('barTemp');
  tempEl.textContent = t.toFixed(1)+' °C';
  const pctT = Math.min(100, Math.max(0, (t-20)/60*100));
  barTemp.style.width = pctT+'%';
  const hot  = t > 70, warn = t > 55;
  tempEl.className  = 'sys-val'+(hot?' hot':warn?' warn':'');
  barTemp.className = 'bar-fill'+(hot?' hot':warn?' warn':'');

  // Heap
  const heapEl  = document.getElementById('sysHeap');
  const barHeap = document.getElementById('barHeap');
  const pctH = Math.min(100, s.freeHeap / s.totalHeap * 100);
  heapEl.textContent = (s.freeHeap/1024).toFixed(0)+' KB free';
  barHeap.style.width = pctH+'%';
  const lowHeap = pctH < 20;
  heapEl.className  = 'sys-val'+(lowHeap?' warn':'');
  barHeap.className = 'bar-fill'+(lowHeap?' warn':'');

  // heap percent labels
  const usedHeapPct = (100 - pctH).toFixed(1);
  document.getElementById('sysHeapPct').textContent =
    'used ' + usedHeapPct + '%  /  free ' + pctH.toFixed(1) + '%';

  // CPU clock + load bar (show % of max 240 MHz)
  const cpuPct = Math.round(s.cpuMhz / 240 * 100);
  document.getElementById('sysCpu').textContent    = s.cpuMhz+' MHz';
  document.getElementById('sysCpuPct').textContent = cpuPct+'% of 240 MHz';
  document.getElementById('barCpu').style.width    = cpuPct+'%';
  document.getElementById('sysUptime').textContent = fmtUptime(s.uptimeMs);
}

async function fetchStatus() {
  try {
    const r = await fetch('/status');
    updateUI(await r.json());
  } catch(e){}
}

async function fetchSys() {
  try {
    const r = await fetch('/sysinfo');
    updateSys(await r.json());
  } catch(e){}
}

async function setFreq() {
  const v = parseFloat(document.getElementById('freqInput').value);
  if (isNaN(v)) return;
  await fetch('/set/freq?v='+v); fetchStatus();
}
async function setWave(i) { await fetch('/set/wave?v='+i); fetchStatus(); }
async function setStep(i) { await fetch('/set/step?v='+i); fetchStatus(); }
async function stepDir(d) {
  const step = STEPS[currentStep];
  const cur  = parseFloat(document.getElementById('freqInput').value)||1000;
  const nv   = Math.max(0.1, Math.min(12000000, cur+d*step));
  document.getElementById('freqInput').value = nv;
  await fetch('/set/freq?v='+nv); fetchStatus();
}
async function changeBPM(d) {
  const nv = Math.max(30, Math.min(200, currentBPM+d));
  await fetch('/set/bpm?v='+nv); currentBPM=nv; fetchStatus();
}
let bpmTimer=null;
function onBPMSlider(v) {
  document.getElementById('bpmNum').textContent=v;
  clearTimeout(bpmTimer);
  bpmTimer=setTimeout(()=>changeBPM(parseInt(v)-currentBPM),120);
}

// Different poll rates: signal fast, sysinfo slower
fetchStatus(); fetchSys();
setInterval(fetchStatus, 2000);
setInterval(fetchSys,    5000);
</script>
</body></html>
)rawhtml";

// ─── WebUI Implementation ─────────────────────────────────

WebUI::WebUI(SignalGenerator& gen)
    : _gen(gen), _server(WEB_PORT), _connected(false)
{}

void WebUI::begin() {
    _connectWiFi();
    if (_connected) {
        if (MDNS.begin(MDNS_HOSTNAME))
            Serial.printf("[mDNS] http://%s.local\n", MDNS_HOSTNAME);
        _registerRoutes();
        _server.begin();
        Serial.printf("[Web] http://%s\n", WiFi.localIP().toString().c_str());
    }
}

void WebUI::handle() {
    if (_connected) _server.handleClient();
}

String WebUI::ipAddress() const {
    return _connected ? WiFi.localIP().toString() : "No WiFi";
}

void WebUI::_connectWiFi() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis()-start < 10000) {
        delay(500); Serial.print(".");
    }
    _connected = (WiFi.status() == WL_CONNECTED);
    if (_connected)
        Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    else
        Serial.println("\n[WiFi] Failed — offline");
}

void WebUI::_registerRoutes() {
    _server.on("/",         [this](){ _handleRoot();    });
    _server.on("/status",   [this](){ _handleStatus();  });
    _server.on("/sysinfo",  [this](){ _handleSysinfo(); });
    _server.on("/set/freq", [this](){ _handleSetFreq(); });
    _server.on("/set/wave", [this](){ _handleSetWave(); });
    _server.on("/set/step", [this](){ _handleSetStep(); });
    _server.on("/set/bpm",  [this](){ _handleSetBPM();  });
}

void WebUI::_handleRoot() {
    _server.send_P(200, "text/html", _HTML);
}

void WebUI::_handleStatus() {
    String j = "{";
    j += "\"freq\":"    + String(_gen.getFrequency(), 2) + ",";
    j += "\"wave\":\""  + String(_gen.waveLabel())  + "\",";
    j += "\"step\":\""  + String(_gen.stepLabel())  + "\",";
    j += "\"waveIdx\":" + String((int)_gen.getWave()) + ",";
    j += "\"stepIdx\":" + String((int)_gen.getStep()) + ",";
    j += "\"bpm\":"     + String(_gen.getBPM());
    j += "}";
    _server.sendHeader("Access-Control-Allow-Origin","*");
    _server.send(200, "application/json", j);
}

void WebUI::_handleSysinfo() {
    // ESP32 internal temperature sensor
    // temperatureRead() returns °C on ESP32 (not available on C3 via this call)
    float tempC = temperatureRead();

    uint32_t freeHeap  = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t cpuMhz    = getCpuFrequencyMhz();
    uint32_t upMs      = millis();

    String j = "{";
    j += "\"tempC\":"    + String(tempC, 1)   + ",";
    j += "\"freeHeap\":" + String(freeHeap)   + ",";
    j += "\"totalHeap\":"+ String(totalHeap)  + ",";
    j += "\"cpuMhz\":"   + String(cpuMhz)     + ",";
    j += "\"uptimeMs\":" + String(upMs);
    j += "}";
    _server.sendHeader("Access-Control-Allow-Origin","*");
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
        _gen.setBPM(_server.arg("v").toInt());
        Serial.printf("[Web] BPM → %d\n", _gen.getBPM());
    }
    _server.send(200, "text/plain", "ok");
}
