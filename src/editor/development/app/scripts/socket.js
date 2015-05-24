'use strict';

var $socket = (function() {
  var socket = null;
  function broadcast(event, message) {
    socket.send(JSON.stringify({
      event: event,
      msg: message
    }));
  }
  function connect() {
    socket = new WebSocket('ws://localhost:9001');
    socket.onmessage = function(e) {
      var evData = JSON.parse(e.data);
      $event.broadcast(evData.event, evData.msg);
    };
  }

  return {
    broadcast: broadcast,
    connect: connect
  };
})();

