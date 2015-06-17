'use strict';

// Handle IO events
function setupIOEvents($scope, $timeout) {
  function handleIOEvents(sceneData) {
    $timeout(function() {
      // Setup game objects
      for(var i = 0; i < sceneData.length; ++i) {
        var goComps = [];
        // TODO create gameObject class
        var comps = sceneData[i].components;
        for(var c = 0; c < comps.length; ++c) {
          var comp = LAU.Components.componentFactory(comps[c].type, comps[c], $scope, $timeout);
          if(comp == null)
            continue;
          goComps.push(comp);
        }

        $scope.gameObjects.push(new LAU.GameObject(sceneData[i].name, goComps));

        $event.broadcast('gameObjectCreated', i);
      }

      // TODO remove line below when the hierarchy panel is correctly created (with blur events to un-select game objects)
      $scope.currentGameObjectId = $scope.gameObjects.length-1;
    });
  }

  $rpc.call('loadCurrentScene', null, handleIOEvents);
}

// Display errors/warnings/etc
function setupConsole($scope, $timeout) {
  $event.listen('compilationStatus', function(eMsg) {
    //console.clear();
    console.log(eMsg.message);
  });
  $event.listen('executionMessage', function(eMsg) {
    console.log(eMsg);
  });
  $event.listen('executionStatus', function(eMsg) {
    //console.clear();
    console.log(eMsg.message);
  });
}

/**
 * @ngdoc function
 * @name lauEditor.controller:MainCtrl
 * @description
 * # MainCtrl
 * Controller of the lauEditor
 */
angular.module('lauEditor').controller('MainCtrl', function ($scope, $timeout, $window, ngDialog) {
  $socket.connect();

  // Setup main layout
  $("#main-wnd-container").layout({
    resizeWhileDragging: true,
    north__spacing_open: 0,
    north__size: 70,
    east__size: 300,
  });
  $('#center-container').layout({
    resizeWhileDragging: true,
    south__size: 200,
  });

  // Inject the LAU namespace into the global scope
  $scope.LAU = LAU;
  // Initialize editor
  $scope.gameObjects = [];
  $scope.currentGameObjectId = -1;
  setupIOEvents($scope, $timeout);
  setupConsole($scope, $timeout);
  $event.listen('reloadProject', function() {
    $window.location.reload();
  });
});
var lau;
