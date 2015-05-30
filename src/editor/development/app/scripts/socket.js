'use strict';

var $socket = (function() {
  var socket = null;
  var publicSocketObj;
  var pendingBroadcasts = {};

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
    socket.onopen = function() {
      publicSocketObj.broadcast = broadcast;
      // Broadcast pending messages
      for(var i in pendingBroadcasts) {
        if(pendingBroadcasts.hasOwnProperty(i)) {
          for(var j = 0; j < pendingBroadcasts[i].length; ++j) {
            broadcast(i, pendingBroadcasts[i][j]);
          }
        }
      }
      pendingBroadcasts = null;
    }
  }
  function requestBroadcast(event, message) {
    if(!pendingBroadcasts.hasOwnProperty(event))
      pendingBroadcasts[event] = [];

    pendingBroadcasts[event].push(message);
  }

  publicSocketObj = {
    broadcast: requestBroadcast,
    connect: connect
  };

  return publicSocketObj;
})();

