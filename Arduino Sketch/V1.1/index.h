
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Arduino and ESP32 Websocket</title>
  <meta name='viewport' content='width=device-width, initial-scale=1.0' />
  <meta charset='UTF-8'>
  <style>

         body {
         text-align: center;
         margin: 5;
         padding: 5;
         background-color: rgba(72,72,72,0.4);
         }
         .slider {
         -webkit-appearance: none;
         width: 100%;
         height: 15px;
         background: #ffff00;
         outline: none;
         opacity: 0.7;
         -webkit-transition: .2s;
         transition: opacity .2s;
         }
         .slider:hover {
         opacity: 1;
         }
         .slider::-moz-range-thumb {
         width: 15px;
         height: 25px;
         background: #FF0F50;
         cursor: pointer;
         }
         .data-input {
         background-color: #bbbbff;
         font-size:26px;
         color:red;
         border: 5px solid #444444;
         -moz-border-radius: 7px;
         -webkit-border-radius: 7px;
         }
         .button-blue {
  padding: 5px 5px 5px 5px;
  width: 10%;
  border: #1111ff solid 3px;
  background-color: #4444ff;
  color: white;
  font-size: 12px;
  padding-bottom: 5px;
  font-weight: 700;
  -moz-border-radius: 7px;
  -webkit-border-radius: 7px;
         }
         .button-blue:hover {
         background-color: #2222aa;
         color: #ff93bd;
         }
         .text {
         background-color: #ff0000;
         font-size:76px;
         color: #ffff99;
         }
         marquee{
         width: 100%;
         font-size: 30px;
         font-weight:700;
         color: #ff0000;
         font-family: sans-serif;
         }
         table {
  text-align: right;
  border: 2px solid #ff00ff;
  background-color: #ffffff;
  width: 100%;
  color: #0000ff;
  -moz-border-radius: 7px;
  -webkit-border-radius: 7px;
         }
         h1 {
         color: #ff0000;
         background-color: #ffff00;
         -moz-border-radius: 7px;
         -webkit-border-radius: 7px;
         }
         div {
  border: 2px solid #ff0000;
  background-color: #bbbbff;
  color: #000000;
  -moz-border-radius: 7px;
  -webkit-border-radius: 7px;
         }
         td {
  border:0px solid #ff0000;
  background-color: #FFFFFF;
  padding: 16px;
  -moz-border-radius: 7px;
  -webkit-border-radius: 7px;
         }
      .textbox {
  font-size: 16px;
  background-color: #BBBBFF;
  border-radius: 8px;
  padding-left: 5px;
}
  </style>
  </style>
</head>

<body>
<div id="kop"><h1>Talking Tetris Clock</h1></div>

  <br>
<table> 
<tbody>
<tr>
<td></td>
<td>
<div>
  <h1>Play Sounds </h1>
  <button type='button' id='Sound_1'> <h1>Work</h1> </button>
  <button type='button' id='Sound_2'> <h1>Bell</h1> </button> 
  <button type='button' id='Sound_3'> <h1>Simpsons</h1> </button> 
  <button type='button' id='Sound_4'> <h1>Failure</h1> </button> 
  <button type='button' id='Sound_5'> <h1>Coffee</h1> </button>
  <button type='button' id='Sound_6'> <h1>Dinner</h1> </button>  
  <button type='button' id='Sound_7'> <h1>Happy Homer</h1> </button> 
  <button type='button' id='Sound_8'> <h1>Ugly</h1> </button> 
  <button type='button' id='Sound_9'> <h1>Manager</h1> </button> 
  <button type='button' id='Sound_10'> <h1>Crap</h1> </button><br><br>
  Volume<input type="range" name="Speed4" min="0" max="21" value="18" class="slider" style="width:40%" id="volume">    
</div>
<br>
<div>
  <p>Brightness   
    <input type="range" name="Speed1" min="10" max="100" value="18" class="slider" style="width:40%" id="mybrightness">
  </p>
</div>
<br>
<div>Animation delay<input type="range" name="Speed2" min="5" max="60" value="25" class="slider" style="width:40%" id="tetspeed"></div>  

<br>
<div>Top Text Speed<input type="range" name="Speed3" min="10" max="95" value="25" class="slider" style="width:40%" id="topspeed"> </div>
<br>
<div>Bottom Text Speed <input type="range" name="Speed3" min="10" max="95" value="25" class="slider" style="width:40%" id="botspeed"> </div>
<br>
<div>Timezone set to : <input type="text" name="Message" value="Europe/Amsterdam" maxlength="100" id="currentzone" class="textbox" style="width:100;" readonly > 
Change it here: <input type="text" name="Message" value="Europe/Berlin" maxlength="100" id="Country" class="textbox" style="width:100;" >
<input type="submit" value="Set" style="width:200" class="button-blue" id="zonebutton">
</div>

<div>

<input type="text" name="Message" value="Text Top Screen" maxlength="100" id="TopTxt" class="textbox" style="width: 85%;" readonly>
<input type="text" name="Message" value="Text Top Screen" maxlength="100" id="TopTxtNew" class="textbox" style="width: 85%;" >
<input type="submit" value="Set" style="width:200" class="button-blue" id="texttopbutton">
</div>

<div>
<input type="text" name="Message" value="Text Bottom Screen" maxlength="100" id="BotTxt" class="textbox" style="width:85%;" readonly>
<input type="text" name="Message" value="Text Bottom Screen" maxlength="100" id="BotTxtNew" class="textbox" style="width:85%;" >
<input type="submit" value="Set" style="width:200" class="button-blue" id="textbotbutton">
</div>

<div>
Set Current Time Manually: ( Only works if there is no WiFi connection)<input type="text" id="currentTime">
<input type="submit" value="Set" style="width:200" class="button-blue" id="settime">
</div>

<div>Network Credentials currently set to:<input type="text" name="Message" value="emdee" maxlength="30" id="SIDOLD" class="textbox" style="width:100;" readonly > (SID/PASSWORD) change here:
<input type="text" name="Message" value="GeniusWeb" maxlength="30" id="SID" class="textbox" style="width:100;" >
<input type="password" name="Message" value="" maxlength="50" id="PW" class="button-white" style="width:100;" >

<input type="submit" value="Set" style="width:200" class="button-blue" id="Network">
</div>
<br>


         <div class="slidecontainer">
         Currently set to:<input type="text" name="Message" value="Text Bottom Screen" maxlength="100" id="Displayold" class="textbox" style="width:200;" readonly>
         <input type="text" name="Message" value="Text Bottom Screen" maxlength="100" id="Chipold" class="textbox" style="width:200;" readonly>
         <br><label for="DisplayType">What display are you using:</label>
         <select name="Displays" id="DisplayType">
         <option value="64x64x1NC">64 x 64px single panel Normal Clock</option>
         <option value="64x64x2NC">64 x 64px double panel Normal Clock</option>
         <option value="64x32x1NC">64 x 32px single panel Normal Clock</option>
         <option value="64x32x2NC">64 x 32px double panel Normal Clock</option>
         <option value="64x64x1IC">64 x 64px single panel Inverted Clock</option>
         <option value="64x64x2IC">64 x 64px double panel Inverted Clock</option>
         <option value="64x32x1IC">64 x 32px single panel Inverted Clock</option>
         <option value="64x32x2IC">64 x 32px double panel Inverted Clock</option>         
         </select>
         <br><br>

         <br><label for="ChipType">What chipset are you using:</label>
         <select name="Chipset" id="ChipsetType">
         <option value="ICN2038S">FM6126A AKA ICN2038S</option>
         <option value="FM6124">FM6124</option>
         <option value="FM6126A">FM6126A</option>
         <option value="MBI5124">MBI5124</option> 
         </select>
         <br><br>
         <input type="submit" value="Confirm Display Type" style="width:20%" class="button-blue" id="displaybutton">
         </div>
</td>
<td></td>
</tbody>
</table> 
</body>
<script>
var today = new Date();
var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
  document.getElementById("currentTime").value = time;
</script>

<script>
  var Socket;
  document.getElementById('Sound_1').addEventListener('click', Sound_1_pressed);
  document.getElementById('Sound_2').addEventListener('click', Sound_2_pressed);
    document.getElementById('Sound_3').addEventListener('click', Sound_3_pressed);
  document.getElementById('Sound_4').addEventListener('click', Sound_4_pressed);
    document.getElementById('Sound_5').addEventListener('click', Sound_5_pressed);
  document.getElementById('Sound_6').addEventListener('click', Sound_6_pressed);
    document.getElementById('Sound_7').addEventListener('click', Sound_7_pressed);
  document.getElementById('Sound_8').addEventListener('click', Sound_8_pressed);
    document.getElementById('Sound_9').addEventListener('click', Sound_9_pressed);
  document.getElementById('Sound_10').addEventListener('click', Sound_10_pressed);

  document.getElementById('mybrightness').addEventListener('click', Brightness_changed);
  document.getElementById('tetspeed').addEventListener('click', tetspeed_changed);
  document.getElementById('topspeed').addEventListener('click', topspeed_changed);
  document.getElementById('botspeed').addEventListener('click', botspeed_changed);  
  document.getElementById('zonebutton').addEventListener('click', zonebutton_clicked);
  document.getElementById('settime').addEventListener('click', timebutton_clicked);
  document.getElementById('texttopbutton').addEventListener('click', texttopbutton_clicked);
  document.getElementById('textbotbutton').addEventListener('click', textbotbutton_clicked);
  document.getElementById('Network').addEventListener('click', Networkbutton_clicked);
  document.getElementById('volume').addEventListener('click', Volume_changed);
  document.getElementById('displaybutton').addEventListener('click', Displaybutton_clicked);
   
  function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event) {
      processCommand(event);
    };
  }
  function processCommand(event) {
    var obj = JSON.parse(event.data);
    document.getElementById('mybrightness').value = obj.Brightness;
    document.getElementById('tetspeed').value = obj.Animationspeed;
    document.getElementById('topspeed').value = obj.Speedtop;
    document.getElementById('botspeed').value = obj.Speedbot;
    document.getElementById('currentzone').value = obj.Tzone;
    document.getElementById('TopTxt').value = obj.TopTXT;
    document.getElementById('BotTxt').value = obj.BotTXT;
    document.getElementById('SIDOLD').value = obj.NetSSID;
    document.getElementById('volume').value = obj.Volume;
    document.getElementById('Displayold').value = obj.Display;
    document.getElementById('Chipold').value =obj.Chipset;
    
    console.log(obj.Brightness);
    console.log(obj.Animationspeed);
    console.log(obj.Speedtop);  
    console.log(obj.Speedbot);    
  }
  function Sound_1_pressed() {
    Socket.send('H' + "Sound1");
    
  }
  function Sound_2_pressed() {
    Socket.send('H' + "Sound2");
  }
  function Sound_3_pressed() {
    Socket.send('H' + "Sound3");
  }
  function Sound_4_pressed() {
    Socket.send('H' + "Sound4");
  }
  function Sound_5_pressed() {
    Socket.send('H' + "Sound5");
  }
  function Sound_6_pressed() {
    Socket.send('H' + "Sound6");
  }
  function Sound_7_pressed() {
    Socket.send('H' + "Sound7");
  }
  function Sound_8_pressed() {
    Socket.send('H' + "Sound8");
  }
  function Sound_9_pressed() {
    Socket.send('H' + "Sound9");
  }
    function Sound_10_pressed() {
    Socket.send('H' + "Sound10");
  }









  
  window.onload = function(event) {
    init();
  }
  function Brightness_changed() {

    Socket.send('B' + document.getElementById('mybrightness').value);
  }
  function tetspeed_changed() {

    Socket.send('T' + document.getElementById('tetspeed').value);
  }
function topspeed_changed() {

    Socket.send('U' + document.getElementById('topspeed').value);
  }

function botspeed_changed() {

    Socket.send('L' + document.getElementById('botspeed').value);
  }
 function zonebutton_clicked() {

    Socket.send('Z' + document.getElementById('Country').value);
  } 

function timebutton_clicked () {

    Socket.send('X' + today.getHours() + ":" + today.getMinutes()+ ":" );

  } 
function texttopbutton_clicked() {
  
  Socket.send('Q' + document.getElementById('TopTxtNew').value);
  document.getElementById('TopTxtNew').value = '';
}

function textbotbutton_clicked() {
  
  Socket.send('R' + document.getElementById('BotTxtNew').value);
  document.getElementById('BotTxtNew').value = "-";
}

function Networkbutton_clicked(){  
  Socket.send('C' + document.getElementById('SID').value + ":" + document.getElementById('PW').value + ":");
}

function Volume_changed() {

    Socket.send('D' + document.getElementById('volume').value);
  }

function Displaybutton_clicked(){
  Socket.send('Y' + document.getElementById('DisplayType').value + ":" + document.getElementById('ChipsetType').value + ":");  
  document.getElementById('displaybutton').value = "Refresh browser!"
  document.getElementById('zonebutton').value = "Refresh browser!"
  document.getElementById('texttopbutton').value = "Refresh browser!"
  document.getElementById('textbotbutton').value = "Refresh browser!"
  document.getElementById('Network').value = "Refresh browser!"

}


</script>
</html>


)=====";
