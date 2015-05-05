var $socket;
// Wait 200ms so that everybody can subscribe to socket related events
// TODO: wait for angularjs to finish instead
setTimeout(function() {
  $socket = new WebSocket("ws://localhost:9001");
  $socket.onopen = function(e) { }
  $socket.onmessage = function(e) {
    var evData = JSON.parse(e.data);
    $event.broadcast(evData.event, evData);
  }
}, 500);

