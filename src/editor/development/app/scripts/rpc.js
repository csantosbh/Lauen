'use strict';

var $rpc = (function() {
  var pendingBroadcasts = {};

  function getNewId() {
    function genId() {
      return Math.floor(Math.random()*1e9);
    }

    var id = genId();
    while(pendingBroadcasts.hasOwnProperty(id)) id = genId();
    return id;
  }

  function gotRPCAnswer(answer) {
    pendingBroadcasts[answer.to](answer.result);
    delete pendingBroadcasts[answer.to];
  }

  $event.listen('RPCAnswer', gotRPCAnswer);

  function RPCSystem() { }
  RPCSystem.prototype = {
    call: function(procedureName, parameters, callback) {
      var myId = getNewId();
      pendingBroadcasts[myId] = callback;
      $socket.broadcast('RPCCall', {
        procedure: procedureName,
        from: myId,
        parameters: parameters
      });
    }
  };
  return new RPCSystem();
})();
