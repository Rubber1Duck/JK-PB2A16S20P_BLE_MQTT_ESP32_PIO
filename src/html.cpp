#include "html.h"

String formatTime(time_t t)
{
    if (t == 0)
        return "Keine Daten";
    struct tm *timeinfo = localtime(&t);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timeinfo);
    return String(buffer);
}

String get_reset_reason_string(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_UNKNOWN:
        return "Unknown";
    case ESP_RST_POWERON:
        return "Power-on";
    case ESP_RST_EXT:
        return "External pin";
    case ESP_RST_SW:
        return "Software reset";
    case ESP_RST_PANIC:
        return "Panic/Exception";
    case ESP_RST_INT_WDT:
        return "Interrupt Watchdog";
    case ESP_RST_TASK_WDT:
        return "Task Watchdog";
    case ESP_RST_WDT:
        return "Other Watchdog";
    case ESP_RST_DEEPSLEEP:
        return "Deep Sleep Wakeup";
    case ESP_RST_BROWNOUT:
        return "Brownout";
    case ESP_RST_SDIO:
        return "SDIO Reset";
    default:
        return "Invalid reason code";
    }
}

const char *get_reset_reason_class(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
    case ESP_RST_BROWNOUT:
        return "reason-critical";
    case ESP_RST_SW:
    case ESP_RST_EXT:
    case ESP_RST_SDIO:
        return "reason-warning";
    case ESP_RST_POWERON:
    case ESP_RST_DEEPSLEEP:
        return "reason-neutral";
    default:
        return "reason-unknown";
    }
}

void handleBmsPage(WebServer &server)
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    server.sendContent("<!DOCTYPE html><html lang='de'><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>JK-BMS Dashboard</title>"
        "<style>"
        "*{box-sizing:border-box;margin:0;padding:0}"
        "body{font-family:sans-serif;background:#1a1a2e;color:#eee;padding:12px}"
        "h1{font-size:1.4rem;color:#00d4ff;margin-bottom:8px}"
        "h2{font-size:.95rem;color:#aaa;margin-bottom:8px;border-bottom:1px solid #333;padding-bottom:4px}"
        ".card{background:#16213e;border-radius:8px;padding:12px;margin-bottom:12px}"
        ".g2{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}"
        ".kv{display:flex;justify-content:space-between;padding:4px 0;border-bottom:1px solid #222}"
        ".kv:last-child{border-bottom:none}"
        ".lbl{color:#aaa;font-size:.82rem}"
        ".val{font-weight:bold;color:#fff;font-size:.88rem}"
        ".status{display:flex;gap:10px;margin-bottom:12px;flex-wrap:wrap}"
        ".sc{background:#16213e;border-radius:8px;padding:10px 14px;flex:1;min-width:90px;text-align:center}"
        ".sv{font-size:1.5rem;font-weight:bold}"
        ".sl{font-size:.72rem;color:#888;margin-top:2px}"
        ".soc-bar{background:#222;border-radius:4px;height:10px;margin:4px 0;overflow:hidden}"
        ".soc-fill{height:100%;border-radius:4px;transition:width .5s}"
        ".cg{display:grid;grid-template-columns:repeat(4,1fr);gap:5px}"
        ".cell{border-radius:6px;padding:6px 2px;text-align:center;font-size:.78rem}"
        ".cn{color:rgba(255,255,255,.5);font-size:.65rem}"
        ".cv{font-weight:bold;font-size:.9rem;margin-top:1px}"
        ".ok{background:#0d3318;border:1px solid #1a6b30}"
        ".hi{background:#2d2000;border:1px solid #e6a800}"
        ".lo{background:#2d0a0a;border:1px solid #cc2200}"
        ".mx{border:2px solid #00ff88!important}"
        ".mn{border:2px solid #ff4444!important}"
        ".off{background:#222;border:1px solid #444;opacity:.35}"
        "#dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#555;margin-right:6px;vertical-align:middle}"
        ".on{background:#00cc66!important}.oo{background:#cc3300!important}"
        "#ts{font-size:.72rem;color:#555;text-align:right;margin-bottom:8px}"
        ".cv-meta{display:flex;gap:14px;margin-bottom:8px;font-size:.8rem;flex-wrap:wrap}"
        ".footer-links{display:flex;gap:16px;justify-content:center;flex-wrap:wrap;margin:18px 0 8px}"
        ".footer-links a{color:#00d4ff;text-decoration:none;font-size:.9rem}"
        ".footer-links a:hover{text-decoration:underline}"
        ".reason-badge{display:inline-block;padding:4px 10px;border-radius:999px;font-size:.82rem;font-weight:600;border:1px solid transparent}"
        ".reason-critical{background:#3a1111;color:#ff8d8d;border-color:#8c2d2d}"
        ".reason-warning{background:#3b280d;color:#ffc266;border-color:#8e5b16}"
        ".reason-neutral{background:#1c2538;color:#b9d3ff;border-color:#30486e}"
        ".reason-unknown{background:#2b2b2b;color:#d5d5d5;border-color:#555}"
        ".toolbar{display:flex;justify-content:space-between;align-items:center;gap:10px;margin:6px 0 10px;flex-wrap:wrap}"
        ".toolbar .ctrl{display:flex;align-items:center;gap:8px;background:#16213e;border-radius:8px;padding:6px 10px}"
        ".toolbar label{font-size:.78rem;color:#9fb3d1}"
        ".toolbar select{background:#0f172a;color:#e2e8f0;border:1px solid #334155;border-radius:6px;padding:2px 6px}"
        ".grid3{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}"
        ".mini{background:#111a31;border-radius:8px;padding:8px;border:1px solid #27314a}"
        ".mini .t{font-size:.72rem;color:#9fb3d1;margin-bottom:4px}"
        ".mini canvas{width:100%;height:48px;display:block}"
        ".chip-wrap{display:flex;gap:8px;flex-wrap:wrap}"
        ".chip{display:inline-block;padding:6px 10px;border-radius:999px;font-size:.78rem;font-weight:600;border:1px solid #555;background:#2b2b2b;color:#d5d5d5}"
        ".chip-critical{background:#3a1111;color:#ff8d8d;border-color:#8c2d2d}"
        ".chip-warning{background:#3b280d;color:#ffc266;border-color:#8e5b16}"
        ".chip-neutral{background:#1c2538;color:#b9d3ff;border-color:#30486e}"
        ".muted{color:#93a4bf;font-size:.8rem}"
        "@media (max-width:900px){.grid3{grid-template-columns:1fr}.g2{grid-template-columns:1fr}.status{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}.sc{min-width:0}.cg{grid-template-columns:repeat(3,1fr)}}"
        "@media (max-width:560px){.status{grid-template-columns:1fr}.cg{grid-template-columns:repeat(2,1fr)}.sv{font-size:1.25rem}}"
        "</style></head><body>");

    server.sendContent(
        "<h1><span id='dot'></span>JK-BMS Dashboard</h1>"
        "<p id='ts'>Warte auf Daten...</p>"
        "<div class='toolbar'>"
        "<div class='ctrl'><label for='refresh-ms'>Aktualisierung</label><select id='refresh-ms'><option value='1000'>1s</option><option value='2000' selected>2s</option><option value='5000'>5s</option><option value='10000'>10s</option></select></div>"
        "<div class='muted' id='alarm-summary'>Keine aktiven Alarme</div>"
        "</div>"
        "<div class='card'><h2>Live-Trends</h2><div class='grid3'>"
        "<div class='mini'><div class='t'>SOC %</div><canvas id='chart-soc' width='280' height='60'></canvas></div>"
        "<div class='mini'><div class='t'>Batteriespannung V</div><canvas id='chart-vol' width='280' height='60'></canvas></div>"
        "<div class='mini'><div class='t'>Batteriestrom A</div><canvas id='chart-cur' width='280' height='60'></canvas></div>"
        "</div></div>"
        "<div class='card'><h2>Ger&#228;teinformationen</h2><div class='g2'>"
        "<div>"
        "<div class='kv'><span class='lbl'>Ger&#228;tename</span><span class='val' id='di-name'>--</span></div>"
        "<div class='kv'><span class='lbl'>Hersteller-ID</span><span class='val' id='di-vid'>--</span></div>"
        "<div class='kv'><span class='lbl'>BMS - Laufzeit</span><span class='val' id='di-bms-up'>--</span></div>"
        "</div><div>"
        "<div class='kv'><span class='lbl'>HW-Version</span><span class='val' id='di-hw'>--</span></div>"
        "<div class='kv'><span class='lbl'>SW-Version</span><span class='val' id='di-sw'>--</span></div>"
        "<div class='kv'><span class='lbl'>ESP - Laufzeit</span><span class='val' id='di-esp-up'>--</span></div>"
        "<div class='kv'><span class='lbl'>Letzter Reset Grund</span><span class='val'><span id='di-reset-reason' class='reason-badge reason-unknown'>--</span></span></div>"
        "</div>"
        "</div></div>"
        "<div class='status'>"
        "<div class='sc'><div class='sv' id='sv-soc'>--</div><div class='sl'>SOC %</div>"
        "<div class='soc-bar'><div class='soc-fill' id='soc-bar' style='width:0%;background:#00cc66'></div></div></div>"
        "<div class='sc'><div class='sv' id='sv-vol'>--</div><div class='sl'>Spannung (V)</div></div>"
        "<div class='sc'><div class='sv' id='sv-cur'>--</div><div class='sl'>Strom (A)</div></div>"
        "<div class='sc'><div class='sv' id='sv-pwr'>--</div><div class='sl'>Leistung (W)</div></div>"
        "</div>");

    server.sendContent(
        "<div class='card'><h2>Alarme</h2>"
        "<div class='kv'><span class='lbl'>Aktive Alarme</span><span class='val' id='alarm-count'>0</span></div>"
        "<div class='chip-wrap' id='alarm-list'><span class='chip chip-neutral'>Keine aktiven Alarme</span></div>"
        "</div>");

    server.sendContent(
        "<div class='card'><h2>Zellspannungen</h2>"
        "<div class='cv-meta'>"
        "<span>&#216; <b id='cv-ave'>--</b> V</span>"
        "<span>&#916; <b id='cv-dif'>--</b></span>"
        "<span>Max: Zelle <b id='cv-max'>--</b></span>"
        "<span>Min: Zelle <b id='cv-min'>--</b></span>"
        "</div>"
        "<div class='cg' id='cg'></div></div>");

    server.sendContent(
        "<div class='card'><h2>Batterie &amp; Kapazit&#228;t</h2><div class='g2'><div>"
        "<div class='kv'><span class='lbl'>Kap. verf&#252;gbar</span><span class='val' id='cd-cap'>--</span></div>"
        "<div class='kv'><span class='lbl'>Zyklen</span><span class='val' id='cd-cyc'>--</span></div>"
        "<div class='kv'><span class='lbl'>Zykluskapazit&#228;t</span><span class='val' id='cd-ccap'>--</span></div>"
        "</div><div>"
        "<div class='kv' id='row-t1'><span class='lbl'>Batterie Temperatur 1</span><span class='val' id='cd-t1'>--</span></div>"
        "<div class='kv' id='row-t2'><span class='lbl'>Batterie Temperatur 2</span><span class='val' id='cd-t2'>--</span></div>"
        "<div class='kv' id='row-t3'><span class='lbl'>MOSFET Temperatur</span><span class='val' id='cd-t3'>--</span></div>"
        "<div class='kv' id='row-t4'><span class='lbl'>Batterie Temperatur 4</span><span class='val' id='cd-t4'>--</span></div>"
        "<div class='kv' id='row-t5'><span class='lbl'>Batterie Temperatur 5</span><span class='val' id='cd-t5'>--</span></div>"
        "</div></div></div>");

    server.sendContent(
        "<div class='card'><h2>System &amp; Schalter</h2><div class='g2'><div>"
        "<div class='kv'><span class='lbl'>SOH</span><span class='val' id='x-soh'>--</span></div>"
        "<div class='kv'><span class='lbl'>BMS Runtime</span><span class='val' id='x-runtime'>--</span></div>"
        "<div class='kv'><span class='lbl'>Balance Status</span><span class='val' id='x-balance'>--</span></div>"
        "<div class='kv'><span class='lbl'>Heizung</span><span class='val' id='x-heat'>--</span></div>"
        "</div><div>"
        "<div class='kv'><span class='lbl'>Charge MOS</span><span class='val' id='x-charge'>--</span></div>"
        "<div class='kv'><span class='lbl'>Discharge MOS</span><span class='val' id='x-discharge'>--</span></div>"
        "<div class='kv'><span class='lbl'>Precharge</span><span class='val' id='x-precharge'>--</span></div>"
        "<div class='kv'><span class='lbl'>Alarm Bitmaske</span><span class='val' id='x-alarm-mask'>--</span></div>"
        "</div></div></div>");

    server.sendContent(
        "<div class='card'><h2>Konfiguration</h2><div class='g2'><div>"
        "<div class='kv'><span class='lbl'>Zellanzahl</span><span class='val' id='cfg-cells'>--</span></div>"
        "<div class='kv'><span class='lbl'>Kapazit&#228;t (Ah)</span><span class='val' id='cfg-cap'>--</span></div>"
        "<div class='kv'><span class='lbl'>Port</span><span class='val' id='cfg-port'>--</span></div>"
        "</div><div>"
        "<div class='kv'><span class='lbl'>Charge EN</span><span class='val' id='cfg-charge'>--</span></div>"
        "<div class='kv'><span class='lbl'>Discharge EN</span><span class='val' id='cfg-discharge'>--</span></div>"
        "<div class='kv'><span class='lbl'>Balance EN</span><span class='val' id='cfg-balance'>--</span></div>"
        "<div class='kv'><span class='lbl'>Smart Sleep</span><span class='val' id='cfg-sleep'>--</span></div>"
        "</div></div></div>");

    server.sendContent(
        "<div class='footer-links'>"
        "<a href='/reset_history'>Reset-Historie</a>"
        "<a href='/update'>Firmware-Update</a>"
        "</div>");

    server.sendContent("<script>"
        "var hist={soc:[],vol:[],cur:[]},histMax=60,pollTimer=null;"
        "function s(id,v){var e=document.getElementById(id);if(e)e.textContent=v;}"
        "function cidx(v){var n=parseInt(v,10);return isNaN(n)?-1:n;}"
        "function setReasonBadge(id,text,cls){var e=document.getElementById(id);if(!e)return;e.textContent=text;e.className='reason-badge '+(cls||'reason-unknown');}"
        "function setTemp(id,rowId,val){var row=document.getElementById(rowId);if(!row)return;var n=parseFloat(val);var hide=(n===-200);row.style.display=hide?'none':'flex';if(!hide)s(id,val+' C');}"
        "function cc(v,a){var d=Math.abs(v-a);if(d>0.050)return 'lo';if(d>0.020)return 'hi';return 'ok';}"
        "function buildCells(d){var g=document.getElementById('cg'),h='',a=parseFloat(d.cells.vol_ave),maxIdx=cidx(d.cells.max_cell),minIdx=cidx(d.cells.min_cell);for(var i=0;i<32;i++){var b=(d.cells.sta>>i)&1,v=parseFloat(d.cells.vol[i]);if(!b)continue;var c=b?cc(v,a):'off';if(i===maxIdx)c+=' mx';if(i===minIdx)c+=' mn';h+='<div class=\"cell '+c+'\"><div class=\"cn\">Z'+(i+1)+'</div><div class=\"cv\">'+d.cells.vol[i]+'</div></div>';}g.innerHTML=h;}"
        "function pushTrend(k,v){if(isNaN(v))return;hist[k].push(v);if(hist[k].length>histMax)hist[k].shift();}"
        "function drawSpark(id,data,color){var c=document.getElementById(id);if(!c||data.length<2)return;var x=c.getContext('2d'),w=c.width,h=c.height,p=4,min=Math.min.apply(null,data),max=Math.max.apply(null,data),span=(max-min)||1;x.clearRect(0,0,w,h);x.strokeStyle='rgba(148,163,184,.25)';x.beginPath();x.moveTo(p,h-p);x.lineTo(w-p,h-p);x.stroke();x.strokeStyle=color;x.lineWidth=2;x.beginPath();for(var i=0;i<data.length;i++){var px=p+(i*(w-2*p))/Math.max(1,data.length-1);var py=h-p-((data[i]-min)/span)*(h-2*p);if(i===0)x.moveTo(px,py);else x.lineTo(px,py);}x.stroke();}"
        "function renderAlarms(d){var list=document.getElementById('alarm-list'),summary=document.getElementById('alarm-summary');if(!list||!summary)return;var a=d.cells.alarms||[];s('alarm-count',d.cells.alarm_count);if(!a.length){list.innerHTML='<span class=\\\"chip chip-neutral\\\">Keine aktiven Alarme</span>';summary.textContent='Keine aktiven Alarme';return;}var html='';for(var i=0;i<a.length;i++){html+='<span class=\\\"chip chip-critical\\\">'+a[i]+'</span>';}list.innerHTML=html;summary.textContent=a.length+' Alarm(e) aktiv';}"
        "function updateConfig(d){if(!d.config_ready){s('cfg-cells','--');s('cfg-cap','--');s('cfg-port','--');s('cfg-charge','--');s('cfg-discharge','--');s('cfg-balance','--');s('cfg-sleep','--');return;}s('cfg-cells',d.config.cell_count);s('cfg-cap',d.config.capacity_ah);s('cfg-port',d.config.port_switch);s('cfg-charge',d.config.charge_en);s('cfg-discharge',d.config.discharge_en);s('cfg-balance',d.config.balance_en);s('cfg-sleep',d.config.smart_sleep);}"
        "function upd(d){document.getElementById('dot').className=d.cells_ready?'on':'oo';s('ts','Zuletzt: '+d.ts);if(d.device_ready){s('di-name',d.device.name);s('di-vid',d.device.vendor_id);s('di-hw',d.device.hw_version);s('di-sw',d.device.sw_version);s('di-bms-up',d.device.bms_uptime);s('di-esp-up',d.device.esp_uptime);}setReasonBadge('di-reset-reason',d.device.last_reset_reason,d.device.last_reset_reason_class);if(d.cells_ready){s('sv-soc',d.cells.soc);s('sv-vol',d.cells.bat_vol);s('sv-cur',d.cells.bat_cur);s('sv-pwr',d.cells.bat_watt);var f=document.getElementById('soc-bar'),soc=parseInt(d.cells.soc),maxIdx=cidx(d.cells.max_cell),minIdx=cidx(d.cells.min_cell);f.style.width=soc+'%';f.style.background=soc>50?'#00cc66':soc>20?'#ffaa00':'#cc3300';s('cv-ave',d.cells.vol_ave);s('cv-dif',d.cells.vol_dif);s('cv-max',maxIdx>=0?(maxIdx+1):'--');s('cv-min',minIdx>=0?(minIdx+1):'--');s('cd-cap',d.cells.cap_remain+' Ah');s('cd-cyc',d.cells.cycles);s('cd-ccap',d.cells.cycle_cap+' Ah');setTemp('cd-t1','row-t1',d.cells.temp1);setTemp('cd-t2','row-t2',d.cells.temp2);setTemp('cd-t3','row-t3',d.cells.temp3);setTemp('cd-t4','row-t4',d.cells.temp4);setTemp('cd-t5','row-t5',d.cells.temp5);s('x-soh',d.cells.soh+' %');s('x-runtime',d.cells.runtime_fmt);s('x-balance',d.cells.balance_status);s('x-heat',d.cells.heating);s('x-charge',d.cells.charge_mos);s('x-discharge',d.cells.discharge_mos);s('x-precharge',d.cells.precharge);s('x-alarm-mask',d.cells.alarm_mask);renderAlarms(d);pushTrend('soc',parseFloat(d.cells.soc));pushTrend('vol',parseFloat(d.cells.bat_vol));pushTrend('cur',parseFloat(d.cells.bat_cur));drawSpark('chart-soc',hist.soc,'#00d084');drawSpark('chart-vol',hist.vol,'#38bdf8');drawSpark('chart-cur',hist.cur,'#f59e0b');buildCells(d);}updateConfig(d);}"
        "function poll(){fetch('/api/bms').then(function(r){return r.json();}).then(function(d){upd(d);}).catch(function(){document.getElementById('dot').className='oo';});}"
        "function applyRefresh(){var sel=document.getElementById('refresh-ms');var ms=parseInt(sel.value)||2000;if(pollTimer)clearInterval(pollTimer);poll();pollTimer=setInterval(poll,ms);}"
        "document.getElementById('refresh-ms').addEventListener('change',applyRefresh);"
        "applyRefresh();"
        "</script></body></html>");

    server.sendContent("");
}

void handleResetHistoryPage(WebServer &server, const ResetEntry *history, size_t historyCount)
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    server.sendContent("<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>ESP32 Time Log</title>");
    server.sendContent("<style>body{font-family:sans-serif;background:#0b0b12;color:#eee;padding:20px;}h1{color:#00d4ff;margin-bottom:16px;}table{width:100%;border-collapse:collapse;background:#16213e;border-radius:10px;overflow:hidden;}td,th{border-bottom:1px solid #24314f;padding:12px 10px;text-align:left;}th{background:#101a30;color:#9cb3d9;font-size:.85rem;text-transform:uppercase;letter-spacing:.04em;}tr:hover td{background:#1b2a49;}a{color:#00d4ff;text-decoration:none;}a:hover{text-decoration:underline;}.actions{display:flex;gap:18px;justify-content:center;flex-wrap:wrap;margin-top:18px;}.wrap{max-width:1100px;margin:0 auto;}.reason-badge{display:inline-block;padding:4px 10px;border-radius:999px;font-size:.82rem;font-weight:600;border:1px solid transparent;}.reason-critical{background:#3a1111;color:#ff8d8d;border-color:#8c2d2d;}.reason-warning{background:#3b280d;color:#ffc266;border-color:#8e5b16;}.reason-neutral{background:#1c2538;color:#b9d3ff;border-color:#30486e;}.reason-unknown{background:#2b2b2b;color:#d5d5d5;border-color:#555;}</style></head><body><div class='wrap'>");
    server.sendContent("<h1>Reset-Historie mit Zeitstempel</h1><table><tr><th>Nr.</th><th>Zeitpunkt</th><th>Grund</th></tr>");

    char row[320];
    for (size_t i = 0; i < historyCount; i++)
    {
        esp_reset_reason_t reason = (esp_reset_reason_t)history[i].reason;
        snprintf(row, sizeof(row), "<tr><td>%u</td><td>%s</td><td><span class='reason-badge %s'>%s</span></td></tr>",
            static_cast<unsigned int>(i + 1),
            formatTime(history[i].timestamp).c_str(),
            get_reset_reason_class(reason),
            get_reset_reason_string(reason).c_str());
        server.sendContent(row);
    }

    server.sendContent("</table><div class='actions'><a href='/clear'>Log l&#246;schen</a><a href='/'>Zur&#252;ck zur Startseite</a></div></div></body></html>");
    server.sendContent("");
}
