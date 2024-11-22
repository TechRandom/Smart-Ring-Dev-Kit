/*****************************************************************    
 *    htmlCode[]
 * 
 *    Store the webpage in Program Memory during upload
 *****************************************************************/
const char htmlCode[] PROGMEM =
R"=====(
<!DOCTYPE HTML>
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: white;
  }
  .topnav {
    overflow: hidden;
    background: rgb(0,185,255);
    background: linear-gradient(90deg, rgba(0,185,255,1) 0%, rgba(244,0,255,1) 50%, rgba(0,185,255,1) 100%);
  }
  body {
    margin: 0;
    background-color: #322940;
  }
  .content {
    padding: 30px;
    max-width: 1200px;
    margin: 0 auto;
    display: flex;
    justify-content: space-around;
    flex-wrap: wrap;
  }
  .card {
    background-color: #1F1A24;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding: 20px;
    margin: 10px;
    flex: 1;
    min-width: 300px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>

<body>
  <div class="topnav">
    <h1>Smart Ring Demo</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Heart Rate</h2>
      <p class="state">Current Heart Rate: <span id="heartRate">60</span> BPM</p>
      <canvas id="heartRateChart" width="400" height="200"></canvas>
    </div>
    <div class="card">
      <h2>Motion Data</h2>
      <p class="state">Current Step Count: <span id="stepCount">0</span></p>
      <canvas id="motionChart" width="400" height="200"></canvas>
    </div>
  </div>
  <script>
    const heartRateData = Array(60).fill(60);
    const motionData = Array.from({length: 6}, () => Math.floor(Math.random() * 5000));
    motionData.push(0); // Initialize the "Current" bar with 0

    const ctxHeartRate = document.getElementById('heartRateChart').getContext('2d');
    const heartRateChart = new Chart(ctxHeartRate, {
      type: 'line',
      data: {
        labels: Array.from({length: 60}, (_, i) => i + 1),
        datasets: [{
          label: 'Heart Rate (BPM)',
          data: heartRateData,
          borderColor: 'rgba(75, 192, 192, 1)',
          borderWidth: 1,
          fill: false
        }]
      },
      options: {
        scales: {
          y: {
            min: 30,
            max: 210
          },
          x: {
            title: {
              display: true,
              text: 'Time (seconds)'
            }
          }
        }
      }
    });

    const ctxMotion = document.getElementById('motionChart').getContext('2d');
    const motionChart = new Chart(ctxMotion, {
      type: 'bar',
      data: {
        labels: ['6 hours ago', '5 hours ago', '4 hours ago', '3 hours ago', '2 hours ago', '1 hour ago', 'Current'],
        datasets: [{
          label: 'Steps Taken',
          data: motionData,
          backgroundColor: 'rgba(153, 102, 255, 0.2)',
          borderColor: 'rgba(153, 102, 255, 1)',
          borderWidth: 1
        }]
      },
      options: {
        scales: {
          y: {
            min: 0,
            max: 5000
          }
        }
      }
    });

    socket = new WebSocket('ws://'+window.location.hostname+':81/'); 
    socket.onmessage = receiveMessage;

    function receiveMessage(event) {
      let data = JSON.parse(event.data);
      if (data.type === "heartRate") {
        document.getElementById('heartRate').innerHTML = data.value;
        updateHeartRateChart(data.value);
      } else if (data.type === "motionData") {
        document.getElementById('stepCount').innerHTML = data.value;
        updateMotionChart(data.value);
      }
    };

    function updateHeartRateChart(value) {
      heartRateData.push(value);
      heartRateData.shift();
      heartRateChart.update();
    }

    function updateMotionChart(value) {
      motionData[motionData.length - 1] = value; // Update the "Current" bar
      motionChart.update();
    }
  </script>
</body>

</html>

)=====";
