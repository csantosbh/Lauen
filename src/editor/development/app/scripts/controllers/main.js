'use strict';

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
    onresize: function(paneName, paneElement, paneState, paneOptions, layoutName) {
      $event.broadcast('paneResized', {
        pane: paneElement[0].id,
        size: {width: paneState.innerWidth, height: paneState.innerHeight},
      });
    }
  });
  $('#center-container').layout({
    resizeWhileDragging: true,
    south__size: 200,
    onresize: function(paneName, paneElement, paneState, paneOptions, layoutName) {
      $event.broadcast('paneResized', {
        pane: paneElement[0].id,
        size: {width: paneState.innerWidth, height: paneState.innerHeight},
      });
    }
  });

  // Inject the LAU namespace into the global scope
  $scope.LAU = LAU;
  // Initialize editor
  $scope.gameObjects = [];
  $scope.currentGameObjectId = -1;
  $scope.reloadProject = function() {
    $window.location.reload();
  };

  var _realGameObjects; // Backup for the real gameobjects in the edit mode
  $event.listen('togglePreviewMode', function(isPreviewing) {
    if(isPreviewing) {
      _realGameObjects = $scope.gameObjects;
      $scope.gameObjects = [];
    } else {
      $scope.gameObjects = _realGameObjects;
    }
  });

  setupConsole($scope, $timeout);
});
var lau;
