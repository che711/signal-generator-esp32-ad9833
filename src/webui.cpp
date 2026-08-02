#include "webui.h"

volatile uint32_t WebUI::_s_idle0 = 0;
volatile uint32_t WebUI::_s_idle1 = 0;

// ─────────────────────────────────────────────────────────
// Embedded HTML — v5 (без изменений UI, только JS улучшен)
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
.sys-divider{height:1px;background:var(--border);margin:12px 0}
.bar-row{display:flex;align-items:center;gap:10px;margin-bottom:9px}
.bar-label{font-size:.82rem;color:var(--text2);width:68px;flex-shrink:0}
.bar-track{flex:1;height:7px;border-radius:4px;background:var(--surface2);overflow:hidden}
.bar-fill{height:100%;border-radius:4px;transition:width .5s ease}
.bar-cpu{background:var(--purple)}
.bar-ram{background:#f5a623}
.bar-free{background:var(--green)}
.bar-pct{font-size:.82rem;font-weight:700;color:var(--text2);width:38px;text-align:right;flex-shrink:0}
.bar-val{font-size:.82rem;font-weight:700;color:var(--green);width:60px;text-align:right;flex-shrink:0}
.card-title.tgl{cursor:pointer;user-select:none}
.card-title.tgl .chev{
  color:var(--purple);font-size:.9rem;transition:transform .2s;margin-left:2px
}
.card.folded .card-body{display:none}
.card.folded .card-title{margin-bottom:0}
.card.folded .chev{transform:rotate(-90deg)}
.btn-reboot{
  width:100%;margin-top:14px;padding:11px;border-radius:10px;
  border:1px solid var(--border);background:transparent;color:var(--text3);
  font-size:.88rem;font-weight:600;cursor:pointer;transition:all .15s;
}
.btn-reboot:hover{border-color:#a03030;color:#e05555}
.btn-out{
  width:100%;padding:16px;border-radius:12px;border:2px solid var(--border);
  font-size:1.05rem;font-weight:800;letter-spacing:.08em;cursor:pointer;
  transition:all .15s;background:var(--surface2);color:var(--text3);
}
.btn-out.on{border-color:var(--green);background:var(--green-bg);color:var(--green);
  box-shadow:0 0 14px rgba(45,212,160,.25)}
.btn-out:hover{transform:translateY(-1px)}
.sw-grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;margin-bottom:12px}
.sw-grid input{
  width:100%;padding:12px 10px;border-radius:10px;
  border:2px solid var(--border);background:var(--bg);
  color:var(--text);font-size:.95rem;text-align:center;outline:none;
}
.sw-grid input:focus{border-color:var(--purple)}
.sw-lbl{font-size:.72rem;color:var(--text3);margin-bottom:4px;text-align:center}
.mode-row{display:flex;gap:10px;margin-bottom:12px}
.mbtn{
  flex:1;padding:11px;border-radius:10px;border:2px solid var(--border);
  background:transparent;color:var(--text2);font-size:.9rem;font-weight:600;
  cursor:pointer;transition:all .15s;
}
.mbtn.active{border-color:var(--green);background:var(--green-bg);color:var(--green)}
.btn-sweep{
  width:100%;padding:14px;border-radius:12px;border:none;
  background:var(--purple-d);color:#fff;font-size:1rem;font-weight:700;
  cursor:pointer;transition:all .15s;
}
.btn-sweep:hover{background:var(--purple)}
.btn-sweep.stop{background:#7a2626}
.btn-sweep.stop:hover{background:#a03030}
.api-item{
  display:flex;align-items:center;gap:8px;
  padding:9px 0;border-bottom:1px solid var(--border);
}
.api-item:last-child{border:none}
.api-cmd{
  flex:1;font-family:ui-monospace,monospace;font-size:.76rem;
  color:var(--text2);overflow-x:auto;white-space:nowrap;
}
.api-desc{font-size:.72rem;color:var(--text3);margin-top:2px}
.btn-copy{
  padding:7px 12px;border-radius:8px;border:1px solid var(--border);
  background:var(--surface2);color:var(--text2);font-size:.78rem;
  font-weight:600;cursor:pointer;flex-shrink:0;transition:all .15s;
}
.btn-copy:hover{border-color:var(--purple);color:var(--purple)}
details.api-details summary{
  cursor:pointer;color:var(--text3);font-size:.85rem;
  list-style:none;user-select:none;
}
details.api-details summary::before{content:'\25B8  ';color:var(--purple)}
details.api-details[open] summary::before{content:'\25BE  '}
</style>
</head>
<body>

<div class="header">
  <h1>&#9646; DDS GENERATOR</h1>
  <div><span class="badge"><span class="dot"></span>ESP32 + AD9833 &middot; Online</span></div>
</div>

<div class="card">
  <div class="card-title">Output</div>
  <button class="btn-out on" id="outBtn" onclick="toggleOut()">OUTPUT ON</button>
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
           placeholder="Enter Hz" min="0.1" max="12000000" step="any">
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
  <div class="card-title tgl" data-k="step" onclick="tglCard(this)">Frequency Step<span class="chev">&#9662;</span></div>
  <div class="card-body">
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
</div>

<div class="card">
  <div class="card-title tgl" data-k="sweep" onclick="tglCard(this)">Sweep<span class="chev">&#9662;</span></div>
  <div class="card-body">
  <div class="sw-grid">
    <div><div class="sw-lbl">From, Hz</div>
      <input type="number" id="swF0" value="100" min="0.1" max="12000000" step="any"></div>
    <div><div class="sw-lbl">To, Hz</div>
      <input type="number" id="swF1" value="100000" min="0.1" max="12000000" step="any"></div>
    <div><div class="sw-lbl">Time, s</div>
      <input type="number" id="swT" value="10" min="0.2" max="3600" step="any"></div>
  </div>
  <div class="mode-row">
    <button class="mbtn active" id="mLin" onclick="setSwMode('lin')">Linear</button>
    <button class="mbtn" id="mLog" onclick="setSwMode('log')">Logarithmic</button>
  </div>
  <div class="bar-row" id="swProgRow" style="display:none">
    <div class="bar-label">Progress</div>
    <div class="bar-track"><div class="bar-fill bar-cpu" id="bSw"></div></div>
    <div class="bar-pct" id="pSw">0%</div>
  </div>
  <button class="btn-sweep" id="swBtn" onclick="toggleSweep()">&#9654;&ensp;Start sweep</button>
  </div>
</div>

<div class="card">
  <div class="card-title">HTTP API</div>
  <details class="api-details" open>
    <summary>curl examples &mdash; scripts, CI, lab automation</summary>
    <div id="apiList" style="margin-top:10px"></div>
  </details>
</div>

<div class="card">
  <div class="card-title tgl" data-k="system" onclick="tglCard(this)">System<span class="chev">&#9662;</span></div>
  <div class="card-body">
  <div class="stat-row">
    <span class="stat-label">WiFi</span>
    <span class="stat-val" id="sySsid" style="color:var(--green)">—</span>
  </div>
  <div class="stat-row">
    <span class="stat-label">IP address</span>
    <span class="stat-val" id="syIp">—</span>
  </div>
  <div class="stat-row">
    <span class="stat-label">Signal</span>
    <span class="stat-val" id="syRssi">—</span>
  </div>
  <div class="sys-divider"></div>
  <div class="bar-row">
    <div class="bar-label">CPU load</div>
    <div class="bar-track"><div class="bar-fill bar-cpu" id="bCpu"></div></div>
    <div class="bar-pct" id="pCpu">—</div>
  </div>
  <div class="bar-row">
    <div class="bar-label">RAM used</div>
    <div class="bar-track"><div class="bar-fill bar-ram" id="bRam"></div></div>
    <div class="bar-pct" id="pRam">—</div>
  </div>
  <div class="bar-row">
    <div class="bar-label">RAM free</div>
    <div class="bar-track"><div class="bar-fill bar-free" id="bFree"></div></div>
    <div class="bar-val" id="pFree">—</div>
  </div>
  <div class="stat-row" style="margin-top:12px">
    <span class="stat-label">Chip temp</span>
    <span class="stat-val" id="syTemp" style="color:#f5a623">—</span>
  </div>
  <div class="stat-row">
    <span class="stat-label">Uptime</span>
    <span class="stat-val" id="syUp" style="color:var(--text2)">—</span>
  </div>
  </div>
</div>

<div class="card" style="padding:16px">
  <div style="display:flex;gap:10px">
    <button class="btn-save" style="margin-top:0;flex:1" onclick="saveSettings()">&#128190;&ensp;Save to memory</button>
    <button class="btn-reboot" style="margin-top:0;width:auto;flex:1" onclick="rebootDev()">&#8635;&ensp;Reboot</button>
  </div>
</div>

<div class="toast" id="toast"></div>

<script>
let curStep = 4;
const STEPS = [0.1,1,10,100,1000,10000,100000,1000000];

function fmtSplit(hz){
  if(hz>=1e6) return [(hz/1e6).toFixed(4),'MHz'];
  if(hz>=1e3) return [(hz/1e3).toFixed(3),'kHz'];
  // не терять дробную часть (шаг 0.1 Гц): 123.5 → "123.5", 123.0 → "123"
  return [(hz % 1 > 1e-3 ? hz.toFixed(1) : Math.round(hz).toString()),'Hz'];
}

function toast(msg,type='ok'){
  const t=document.getElementById('toast');
  t.textContent=msg; t.className='toast '+type+' show';
  setTimeout(()=>t.classList.remove('show'),2200);
}

function applyStatus(d){
  // Частота — берём реально установленное значение из ответа сервера
  const [v,u]=fmtSplit(d.freq);
  document.getElementById('fVal').textContent   = v;
  document.getElementById('fUnit').textContent  = ' '+u;
  document.getElementById('fRaw').textContent   = d.freq.toFixed(2)+' Hz';
  // Не затирать поле ввода, пока пользователь в нём печатает —
  // раньше poll() каждые 2 c сбрасывал недонабранное значение
  const fin=document.getElementById('freqIn');
  if(document.activeElement!==fin) fin.value=d.freq;

  for(let i=0;i<4;i++)
    document.getElementById('w'+i).classList.toggle('active',i===d.waveIdx);
  for(let i=0;i<8;i++)
    document.getElementById('s'+i).classList.toggle('active',i===d.stepIdx);

  document.querySelectorAll('.wbtn').forEach((btn,i)=>{
    const c = i===d.waveIdx ? 'var(--purple)' : '#8892b0';
    btn.querySelectorAll('path,polyline').forEach(el=>el.setAttribute('stroke',c));
  });
  curStep=d.stepIdx;
  applySweep(d.sweep);
  applyOut(d.out!==false);

  if(d.sys){
    const s=d.sys;
    document.getElementById('sySsid').textContent = s.ssid||'—';
    document.getElementById('syIp').textContent   = s.ip||'—';

    const rssiPct = Math.max(0,Math.min(100, 2*(s.rssi+100)));
    const rssiBar = rssiPct<30?'#e24b4a':rssiPct<60?'#f5a623':'#2dd4a0';
    document.getElementById('syRssi').textContent = s.rssi+'dBm ('+rssiPct+'%)';
    document.getElementById('syRssi').style.color = rssiBar;

    const cpuW = Math.min(100,s.cpu)+'%';
    document.getElementById('bCpu').style.width = cpuW;
    document.getElementById('pCpu').textContent = s.cpu+'%';
    document.getElementById('bCpu').style.background =
      s.cpu>80?'#e24b4a':s.cpu>50?'#f5a623':'var(--purple)';

    const ramPct = Math.round((s.totalHeap-s.freeHeap)/s.totalHeap*100);
    document.getElementById('bRam').style.width = ramPct+'%';
    document.getElementById('pRam').textContent = ramPct+'%';

    const freePct = Math.round(s.freeHeap/s.totalHeap*100);
    document.getElementById('bFree').style.width = freePct+'%';
    document.getElementById('pFree').textContent = Math.round(s.freeHeap/1024)+'kB';

    document.getElementById('syTemp').textContent = s.temp.toFixed(1)+' °C';
    document.getElementById('syTemp').style.color =
      s.temp>70?'#e24b4a':'#f5a623';

    const u=s.uptime;
    const h=Math.floor(u/3600),m=Math.floor((u%3600)/60),sec=u%60;
    document.getElementById('syUp').textContent =
      (h?h+'h ':'')+m+'m '+sec+'s';
  }
}

async function poll(){
  try{
    const r=await fetch('/status');
    if(r.ok) applyStatus(await r.json());
  }catch(e){}
}

async function api(url){
  try{
    const r=await fetch(url);
    if(!r.ok){ toast('Error '+r.status,'err'); return null; }
    return await r.json();
  }catch(e){ toast('No connection','err'); return null; }
}

async function setFreq(){
  const el=document.getElementById('freqIn');
  const v=parseFloat(el.value);
  if(isNaN(v)||v<0.1||v>12000000){toast('Valid: 0.1 Hz – 12 MHz','err');return;}
  el.blur();                       // вернуть поле под управление poll()
  await fetch('/set/freq?v='+v); poll();
}

async function setWave(i){
  const d=await api('/set/wave?v='+i);
  if(d) applyStatus(d);
}

async function setStep(i){
  const d=await api('/set/step?v='+i);
  if(d) applyStatus(d);
}

async function nudge(dir){
  const cur=parseFloat(document.getElementById('freqIn').value)||1000;
  const nv=Math.max(0.1,Math.min(12000000,cur+dir*STEPS[curStep]));
  const d=await api('/set/freq?v='+nv);
  if(d) applyStatus(d);
}

async function saveSettings(){
  const r=await fetch('/save');
  if(r.ok) toast('Saved to memory \u2713');
  else toast('Save failed','err');
}

document.getElementById('freqIn')
  .addEventListener('keydown',e=>{ if(e.key==='Enter') setFreq(); });

// ── Output toggle ──
let outOn=true;
async function toggleOut(){
  const d=await api('/set/out?v='+(outOn?0:1));
  if(d){ applyStatus(d); toast(d.out?'Output ON':'Output OFF'); }
}
function applyOut(on){
  outOn=on;
  const b=document.getElementById('outBtn');
  b.textContent=on?'OUTPUT ON':'OUTPUT OFF';
  b.classList.toggle('on',on);
  document.querySelector('.freq-value').style.opacity=on?'1':'.35';
}

// ── Sweep UI ──
let swMode='lin', swActive=false;

function setSwMode(m){
  swMode=m;
  document.getElementById('mLin').classList.toggle('active',m==='lin');
  document.getElementById('mLog').classList.toggle('active',m==='log');
}

async function toggleSweep(){
  if(swActive){
    const d=await api('/sweep/stop');
    if(d) applyStatus(d);
    return;
  }
  const f0=parseFloat(document.getElementById('swF0').value);
  const f1=parseFloat(document.getElementById('swF1').value);
  const t =parseFloat(document.getElementById('swT').value);
  if([f0,f1,t].some(isNaN)){toast('Fill all sweep fields','err');return;}
  const d=await api(`/sweep/start?f0=${f0}&f1=${f1}&t=${t}&mode=${swMode}`);
  if(d){ applyStatus(d); toast('Sweep started'); }
}

function applySweep(sw){
  if(!sw) return;
  swActive=sw.active;
  const btn=document.getElementById('swBtn');
  const row=document.getElementById('swProgRow');
  if(sw.active){
    btn.closest('.card').classList.remove('folded');
    btn.innerHTML='&#9632;&ensp;Stop sweep';
    btn.classList.add('stop');
    row.style.display='flex';
    document.getElementById('bSw').style.width=sw.progress+'%';
    document.getElementById('pSw').textContent=sw.progress+'%';
  }else{
    btn.innerHTML='&#9654;&ensp;Start sweep';
    btn.classList.remove('stop');
    row.style.display='none';
  }
}

// ── API examples with copy buttons ──
function buildApiList(){
  const h=location.host||'dds-gen.local';
  const EX=[
    ['Device state (freq, wave, RSSI, heap, uptime)', `curl http://${h}/status`],
    ['Set frequency: 10 kHz',                          `curl "http://${h}/set/freq?v=10000"`],
    ['Waveform: sine | tri | sqr | sqr2',              `curl "http://${h}/set/wave?v=sine"`],
    ['Encoder step, Hz per click: 0.1 ... 1m',         `curl "http://${h}/set/step?v=1k"`],
    ['Log sweep 10 Hz to 100 kHz over 10 s (returns to prior freq when done)', `curl "http://${h}/sweep/start?f0=10&f1=100000&t=10&mode=log"`],
    ['Stop sweep mid-run (freq stays where it was)',   `curl http://${h}/sweep/stop`],
    ['Output enable: 1 on, 0 off (mute, keeps settings)', `curl "http://${h}/set/out?v=0"`],
    ['Persist current settings to NVS',                `curl http://${h}/save`],
    ['Reboot the device (settings saved first)',       `curl http://${h}/reboot`],
  ];
  document.getElementById('apiList').innerHTML = EX.map(([d,c],i)=>`
    <div class="api-item">
      <div style="flex:1;min-width:0">
        <div class="api-cmd" id="cmd${i}">${c.replace(/&/g,'&amp;')}</div>
        <div class="api-desc">${d}</div>
      </div>
      <button class="btn-copy" onclick="copyCmd(${i})">Copy</button>
    </div>`).join('');
}

async function copyCmd(i){
  const txt=document.getElementById('cmd'+i).textContent;
  try{
    await navigator.clipboard.writeText(txt);
    toast('Copied \u2713');
  }catch(e){
    // clipboard API требует HTTPS/localhost — фоллбэк для http://
    const ta=document.createElement('textarea');
    ta.value=txt; document.body.appendChild(ta);
    ta.select(); document.execCommand('copy'); ta.remove();
    toast('Copied \u2713');
  }
}

// ── Reboot ──
async function rebootDev(){
  if(!confirm('Reboot the generator?')) return;
  try{ await fetch('/reboot'); }catch(e){}
  toast('Rebooting\u2026');
  // страница сама оживёт: poll() каждые 2 с начнёт получать /status,
  // как только девайс поднимет Wi-Fi
}

// ── Collapsible cards (state in localStorage) ──
function tglCard(el){
  const card=el.parentElement;
  card.classList.toggle('folded');
  try{localStorage.setItem('fold_'+el.dataset.k,
      card.classList.contains('folded')?'1':'0');}catch(e){}
}
document.querySelectorAll('.card-title.tgl').forEach(el=>{
  try{
    if(localStorage.getItem('fold_'+el.dataset.k)==='1')
      el.parentElement.classList.add('folded');
  }catch(e){}
});

buildApiList();
poll();
setInterval(poll,2000);
</script>
</body></html>
)rawhtml";

// ─────────────────────────────────────────────────────────

WebUI::WebUI(SignalGenerator& gen, SemaphoreHandle_t genMutex)
    : _gen(gen), _genMutex(genMutex), _server(WEB_PORT), _connected(false),
      _serverStarted(false), _mdnsStarted(false), _changedFlag(false),
      _lastWifiCheckMs(0)
{}

void WebUI::begin() {
    _initCpuMon();
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
        delay(100);
        _connectWiFi();
        // БАГ был здесь: повторный _startServer() каждый реконнект заново
        // регистрировал маршруты (WebServer::on() выделяет память под каждый
        // handler → утечка) и вызывал MDNS.begin() поверх работающего.
        if (_connected) _startServer();
    }
}

String WebUI::ipAddress() const {
    return _connected ? WiFi.localIP().toString() : "No WiFi";
}

// ── Мьютекс-хелпер ────────────────────────────────────────
bool WebUI::_withGen(std::function<void()> fn, TickType_t timeout) {
    if (xSemaphoreTake(_genMutex, timeout) == pdTRUE) {
        fn();
        xSemaphoreGive(_genMutex);
        return true;
    }
    Serial.println("[Web] mutex timeout!");
    return false;
}

// ── WiFi ──────────────────────────────────────────────────
void WebUI::_connectWiFi() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);

    // Лог причины обрыва — регистрируем один раз.
    // Коды: 200 BEACON_TIMEOUT / 201 NO_AP_FOUND — радио или питание;
    //       8 — точка сняла ассоциацию сама; 2/15/202 — аутентификация
    static bool evtHooked = false;
    if (!evtHooked) {
        evtHooked = true;
        WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info){
            Serial.printf("[WiFi] disconnected, reason=%d\n",
                          (int)info.wifi_sta_disconnected.reason);
        }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    }

    WiFi.persistent(false);       // не переписывать креды во флеш каждый begin()
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);         // modem sleep OFF: девайс на проводе, латентность
                                  // и пропуски маяков важнее ~60 мА экономии
    WiFi.setAutoReconnect(true);  // стек реконнектится сам, вотчдог — страховка
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
    // mDNS перезапускаем после реконнекта (иначе .local перестаёт отвечать)
    if (_mdnsStarted) MDNS.end();
    _mdnsStarted = MDNS.begin(MDNS_HOSTNAME);
    if (_mdnsStarted)
        Serial.printf("[mDNS] http://%s.local\n", MDNS_HOSTNAME);

    // Маршруты и сам сервер запускаем ровно один раз
    if (!_serverStarted) {
        _registerRoutes();
        _server.begin();
        _serverStarted = true;
    }
    Serial.printf("[Web] http://%s\n", WiFi.localIP().toString().c_str());
}

void WebUI::_registerRoutes() {
    _server.on("/",         [this](){ _handleRoot();    });
    _server.on("/status",   [this](){ _handleStatus();  });
    _server.on("/set/freq", [this](){ _handleSetFreq(); });
    _server.on("/set/wave", [this](){ _handleSetWave(); });
    _server.on("/set/step", [this](){ _handleSetStep(); });
    _server.on("/save",        [this](){ _handleSave();       });
    _server.on("/set/out",     [this](){ _handleSetOut();     });
    _server.on("/reboot",      [this](){ _handleReboot();     });
    _server.on("/sweep/start", [this](){ _handleSweepStart(); });
    _server.on("/sweep/stop",  [this](){ _handleSweepStop();  });
    _server.onNotFound([this](){ _server.send(404, "text/plain", "not found"); });
}

void WebUI::_handleRoot() {
    _server.send_P(200, "text/html", _HTML);
}

// ── Общий JSON-ответ с текущим состоянием ─────────────────
// Используется всеми set-хендлерами и /status
void WebUI::_handleStatus() {
    uint32_t freeHeap  = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    float    temp      = temperatureRead();
    int32_t  rssi      = WiFi.RSSI();
    uint32_t uptime    = millis() / 1000;

    float freq = 0; int waveIdx = 0; int stepIdx = 0;
    const char* waveLbl = ""; const char* stepLbl = "";
    bool outOn = true;
    bool swAct = false, swLog = false;
    float swF0 = 0, swF1 = 0; uint32_t swT = 0; int swPct = 0;

    _withGen([&](){
        freq    = _gen.getFrequency();
        waveIdx = (int)_gen.getWave();
        stepIdx = (int)_gen.getStep();
        waveLbl = _gen.waveLabel();
        stepLbl = _gen.stepLabel();
        outOn   = _gen.getOutput();
        swAct   = _gen.sweepActive();
        swLog   = _gen.sweepIsLog();
        swF0    = _gen.sweepF0();
        swF1    = _gen.sweepF1();
        swT     = _gen.sweepDurMs();
        swPct   = _gen.sweepProgress();
    });

    String j = "{";
    j += "\"freq\":"      + String(freq, 2)    + ",";
    j += "\"wave\":\""    + String(waveLbl)    + "\",";
    j += "\"step\":\""    + String(stepLbl)    + "\",";
    j += "\"waveIdx\":"   + String(waveIdx)    + ",";
    j += "\"stepIdx\":"   + String(stepIdx)    + ",";
    j += "\"out\":"       + String(outOn ? "true" : "false") + ",";
    j += "\"sweep\":{";
    j += "\"active\":"   + String(swAct ? "true" : "false") + ",";
    j += "\"f0\":"       + String(swF0, 1)  + ",";
    j += "\"f1\":"       + String(swF1, 1)  + ",";
    j += "\"t\":"        + String(swT)      + ",";
    j += "\"mode\":\""  + String(swLog ? "log" : "lin") + "\",";
    j += "\"progress\":" + String(swPct);
    j += "},";
    j += "\"sys\":{";
    j += "\"ssid\":\""    + String(WIFI_SSID)                + "\",";
    j += "\"ip\":\""      + WiFi.localIP().toString()        + "\",";
    j += "\"rssi\":"      + String(rssi)                     + ",";
    j += "\"cpu\":"       + String(_cpuLoad)                 + ",";
    j += "\"freeHeap\":"  + String(freeHeap)                 + ",";
    j += "\"totalHeap\":" + String(totalHeap)                + ",";
    j += "\"temp\":"      + String(temp, 1)                  + ",";
    j += "\"uptime\":"    + String(uptime);
    j += "}}";

    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", j);
}

void WebUI::_handleSetFreq() {
    if (!_server.hasArg("v")) {
        _server.send(400, "text/plain", "missing arg v");
        return;
    }
    float requested = _server.arg("v").toFloat();
    float applied = 0;
    // applied читаем внутри мьютекса: раньше _gen.getFrequency() дёргался
    // уже после _withGen — гонка с UI-задачей на ядре 1
    _withGen([&](){ applied = _gen.setFrequency(requested); });
    _changedFlag = true;
    Serial.printf("[Web] freq → %.2f Hz (requested %.2f)\n",
                  applied, requested);
    _handleStatus();   // вернуть реально установленное состояние
}

void WebUI::_handleSetWave() {
    if (!_server.hasArg("v")) {
        _server.send(400, "text/plain", "usage: /set/wave?v=sine|tri|sqr|sqr2");
        return;
    }
    String v = _server.arg("v");
    v.toLowerCase();
    int idx = -1;
    if      (v == "sine" || v == "sin")                       idx = WAVE_SINE;
    else if (v == "tri"  || v == "triangle")                  idx = WAVE_TRIANGLE;
    else if (v == "sqr"  || v == "square")                    idx = WAVE_SQUARE;
    else if (v == "sqr2" || v == "square2" || v == "sqr/2")   idx = WAVE_SQUARE2;
    else if (v.length() && isDigit(v[0]))                     idx = v.toInt();  // legacy 0-3
    if (idx < 0 || idx >= WAVE_COUNT) {
        _server.send(400, "text/plain", "bad wave: sine|tri|sqr|sqr2 (or 0-3)");
        return;
    }
    const char* lbl = "";
    _withGen([&](){ _gen.setWaveByIndex(idx); lbl = _gen.waveLabel(); });
    _changedFlag = true;
    Serial.printf("[Web] wave → %s\n", lbl);
    _handleStatus();
}

void WebUI::_handleSetStep() {
    if (!_server.hasArg("v")) {
        _server.send(400, "text/plain",
            "usage: /set/step?v=0.1|1|10|100|1k|10k|100k|1m (Hz per encoder click)");
        return;
    }
    String v = _server.arg("v");
    v.toLowerCase();
    // Шаг энкодера: на сколько Гц двигается частота за один щелчок ручки
    static const char* names[STEP_COUNT] =
        {"0.1", "1", "10", "100", "1k", "10k", "100k", "1m"};
    int idx = -1;
    for (int i = 0; i < STEP_COUNT; i++)
        if (v == names[i]) { idx = i; break; }
    if (idx < 0 && v.length() && isDigit(v[0]) && v.length() == 1)
        idx = v.toInt();                                      // legacy 0-7
    if (idx < 0 || idx >= STEP_COUNT) {
        _server.send(400, "text/plain",
            "bad step: 0.1|1|10|100|1k|10k|100k|1m (or 0-7)");
        return;
    }
    const char* lbl = "";
    _withGen([&](){ _gen.setStepByIndex(idx); lbl = _gen.stepLabel(); });
    _changedFlag = true;
    Serial.printf("[Web] step → %s\n", lbl);
    _handleStatus();
}

void WebUI::_handleSave() {
    _withGen([&](){ _gen.saveSettings(); });
    _server.send(200, "text/plain", "ok");
}

void WebUI::_handleReboot() {
    // Сначала сохранить настройки: автосейв дебаунсится 5 с, и ребут сразу
    // после смены частоты иначе теряет её
    _withGen([&](){ _gen.saveSettings(); });
    _server.send(200, "text/plain", "rebooting");
    _server.client().stop();     // дожать ответ клиенту до рестарта
    Serial.println("[Web] reboot requested");
    delay(200);
    ESP.restart();
}

void WebUI::_handleSetOut() {
    if (!_server.hasArg("v")) {
        _server.send(400, "text/plain", "missing arg v (0|1)");
        return;
    }
    bool on = _server.arg("v").toInt() != 0;
    _withGen([&](){ _gen.setOutput(on); });
    _changedFlag = true;
    _handleStatus();
}

void WebUI::_handleSweepStart() {
    if (!_server.hasArg("f0") || !_server.hasArg("f1") || !_server.hasArg("t")) {
        _server.send(400, "text/plain", "need args: f0, f1, t (seconds)");
        return;
    }
    float f0 = _server.arg("f0").toFloat();
    float f1 = _server.arg("f1").toFloat();
    float ts = _server.arg("t").toFloat();
    bool  lg = _server.arg("mode") == "log";
    if (lg && (f0 <= 0 || f1 <= 0)) {
        _server.send(400, "text/plain", "log sweep needs f0,f1 > 0");
        return;
    }

    bool ok = false;
    _withGen([&](){ ok = _gen.sweepStart(f0, f1, (uint32_t)(ts * 1000.0f), lg); });
    if (!ok) {
        _server.send(400, "text/plain",
                     "bad sweep params (freq 0.1-12e6 Hz, t 0.2-3600 s, f0 != f1)");
        return;
    }
    _changedFlag = true;
    _handleStatus();
}

void WebUI::_handleSweepStop() {
    _withGen([&](){ _gen.sweepStop(); });
    _changedFlag = true;
    _handleStatus();
}

// ── CPU load monitor ──────────────────────────────────────
bool IRAM_ATTR WebUI::_idleHook0() { _s_idle0 = _s_idle0 + 1; return false; }
bool IRAM_ATTR WebUI::_idleHook1() { _s_idle1 = _s_idle1 + 1; return false; }

void WebUI::_initCpuMon() {
    _cpuLoad        = 0;
    _cpuSampleMs    = millis();
    _cpuIdle0Prev   = 0;
    _cpuIdle1Prev   = 0;
    _cpuIdleRateMax = 0.0f;
    _cpuFirstSample = true;

    esp_register_freertos_idle_hook_for_cpu(_idleHook0, 0);
    esp_register_freertos_idle_hook_for_cpu(_idleHook1, 1);
}

void WebUI::updateCpuLoad() {
    uint32_t now = millis();
    uint32_t elapsed = now - _cpuSampleMs;
    if (elapsed < 2000) return;

    uint32_t i0 = _s_idle0;
    uint32_t i1 = _s_idle1;
    uint32_t idleTotal = (i0 - _cpuIdle0Prev) + (i1 - _cpuIdle1Prev);
    _cpuIdle0Prev = i0;
    _cpuIdle1Prev = i1;
    _cpuSampleMs  = now;

    // БАГ был здесь: baseline брался из первого интервала, который включал
    // блокирующее подключение WiFi (до 8 с) — калибровка получалась
    // случайной, и проценты дальше врали. Теперь:
    //  1) нормируем по фактически прошедшему времени (тиков/мс);
    //  2) первый интервал пропускаем;
    //  3) baseline самокорректируется вверх, если система оказалась
    //     ещё более "пустой", чем при калибровке.
    float rate = (float)idleTotal / (float)elapsed;

    if (_cpuFirstSample) {
        _cpuFirstSample = false;
        return;
    }
    if (rate > _cpuIdleRateMax) _cpuIdleRateMax = rate;
    if (_cpuIdleRateMax <= 0.0f) return;

    int load = 100 - (int)(rate / _cpuIdleRateMax * 100.0f);
    _cpuLoad = constrain(load, 0, 100);
}
