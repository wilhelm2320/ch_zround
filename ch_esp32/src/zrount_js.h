const char index_html[] PROGMEM = R"rawliteral(
<html>
<head>
<meta content="text/html; charset=ISO-8859-1"
http-equiv="content-type">
<title></title>
</head>
<body>
<table style="text-align: left; width: 249px; height: 120px;" border="1"
cellpadding="2" cellspacing="2">
<tbody>
<tr>
<td style="vertical-align: top; text-align: center;"><br>
</td>
<td style="vertical-align: top; text-align: center;">01<br>
</td>
<td style="vertical-align: top; text-align: center;">02<br>
</td>
<td style="vertical-align: top; text-align: center;">03<br>
</td>
<td style="vertical-align: top; text-align: center; height: 30px;">04<br>
</td>
<td style="vertical-align: top; text-align: center;">05<br>
</td>
</tr>
<tr>
<td style="vertical-align: top; width: 44px; text-align: left;"><span
style="color: red; background-color: white;">RED</span><br>
</td>
<td style="vertical-align: top; text-align: center;"><input
id="i15" name="15" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"><input
id="i14" name="14" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"><input
id="i13" name="13" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"><input
id="i12" name="12" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"><input
id="i11" name="11" type="checkbox"></td>
</tr>
<tr>
<td
style="vertical-align: top; width: 44px; background-color: white; text-align: left;"><span
style="color: rgb(51, 204, 0);">GREEN</span><br>
</td>
<td style="vertical-align: top; text-align: center;"> <input
name="110" id="i110" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="19" id="i19" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="18" id="i18" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="17" id="i17" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="16" id="i16" type="checkbox"></td>
</tr>
<tr>
<td style="vertical-align: top; width: 44px; text-align: left;"><span
style="background-color: white; color: rgb(51, 102, 255);">BLUE</span><br>
</td>
<td style="vertical-align: top; text-align: center;"> <input
name="115" id="i115" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="114" id="i114" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="113" id="i113" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="112" id="i112" type="checkbox"></td>
<td style="vertical-align: top; text-align: center;"> <input
name="111" id="i111" type="checkbox"></td>
</tr>
</tbody>
</table>
<br>
<input value="Connect ZRound" name="Connect" id="connect" type="button"><br>
<br>
<input name="start" id="start" value="Start Race" type="button">
<input name="Stop" id="stop" value="Stop Race" type="button"><br>
<script>



const const_color =
{
red: 2,
green: 1,
blue: 0

}

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function initWebSocket()
{
console.log('Trying to open a WebSocket connection...');
websocket = new WebSocket(gateway);
websocket.onopen = onOpen;
websocket.onclose = onClose;
websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event)
{
console.log('Connection opened');
}
function onClose(event)
{
console.log('Connection closed');
setTimeout(initWebSocket, 2000);
}
function onMessage( )
{




/*
//console.log('Messeage received...'+event.data);
var state;
const cb = document.querySelector('#r2');
// alert("BIN DO" + event.data);

const data = JSON.parse(event.data);

// Nach Farbe gruppieren
const grouped = {};

data.ampeln.forEach(ampel =>
{
const color = ampel.color;
if (!grouped[color])
{
grouped[color] = [];
}
grouped[color].push(
{
nr: ampel.nr,
status: ampel.status
}
);
});

// Ausgabe
for (const color in grouped)
{
grouped[color].forEach(ampel =>
{

console.log(`Color ${color}: Ampel ${ampel.nr}: Status ${ampel.status}`);
//const_color.red?0:color==const_color.green?const_color.green:const_color.blue
console.log('i1'+((color==const_color.red?0:color==const_color.green?1:2)*5+ ampel.nr +1));
document.getElementById( 'i1'+((color==const_color.red?0:color==const_color.green?1:2)*5+ ampel.nr+1 )).checked=ampel.status==2?false:true;
});
}*/

 
const p = document.createElement('p');
p.textContent = event.data;
document.body.appendChild(p);
 
}


function onLoad(event)
{
console.log('on load...');
initWebSocket();
initCheckbox();
}

function initCheckbox()
{
console.log('init checkbox...');
document.getElementById('i11').addEventListener('change', f_r11);
document.getElementById('i12').addEventListener('change', f_r11);
document.getElementById('i13').addEventListener('change', f_r11);
document.getElementById('i14').addEventListener('change', f_r11);
document.getElementById('i15').addEventListener('change', f_r11);
document.getElementById('i16').addEventListener('change', f_r11);
document.getElementById('i17').addEventListener('change', f_r11);
document.getElementById('i18').addEventListener('change', f_r11);
document.getElementById('i19').addEventListener('change', f_r11);
document.getElementById('i110').addEventListener('change', f_r11);
document.getElementById('i111').addEventListener('change', f_r11);
document.getElementById('i112').addEventListener('change', f_r11);
document.getElementById('i113').addEventListener('change', f_r11);
document.getElementById('i114').addEventListener('change', f_r11);
document.getElementById('i115').addEventListener('change', f_r11);

document.getElementById('start').addEventListener('click', f_button_start);
document.getElementById('stop').addEventListener('click', f_button_stop);
document.getElementById('connect').addEventListener('click', f_button_connect);
document.getElementById('rfid').addEventListener('click', f_button_rfid);

}
function f_button( iCmd)
{
var send = {};
var commandos = [];
send.commandos=commandos;
var commando =
{
"cmd": iCmd

}
send.commandos.push(commando);
console.log(JSON.stringify(send));
websocket.send(JSON.stringify(send));

}

function f_button_rfid()
{
f_button(4);
}

function f_button_start()
{
f_button(2);
}

function f_button_connect()
{
f_button(1);
}
function f_button_stop()
{
f_button(3);
}


function f_r11()
{
var send = {};
var ampeln = []
send.ampeln = ampeln;

for(let i = 0;i<15;i++)
{
var ampel =
{
"color": i<5?const_color.red:i<10?const_color.green:const_color.blue,
"nr": i%5,
"status": (document.getElementById('i1'+(i+1)).checked)?1:2
}
send.ampeln.push(ampel);
}
console.log(JSON.stringify(send));
websocket.send(JSON.stringify(send));
}


</script>

</body>
</html>


)rawliteral";