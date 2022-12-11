console.log("JS RUNNING");

const exampleSocket = new WebSocket("ws://10.111.1.211/log");

exampleSocket.onmessage = (event) => {
  console.log(event.data);
  logObj = JSON.parse(event.data);

  var utcSeconds = logObj.timestamp;
  var d = new Date(0); 
  d.setUTCSeconds(utcSeconds);
  //Add row
  let tbl = document.getElementById("logtable").getElementsByTagName("tbody")[0];
  let newRow = tbl.insertRow();
  newRow.classList.add(logObj.severity);
  newRow.insertCell().append(d.toLocaleDateString() + " " + d.toLocaleTimeString());
  newRow.insertCell().append(logObj.severity);
  newRow.insertCell().append(logObj.message);
  tbl.scrollTop = tbl.scrollHeight;
}