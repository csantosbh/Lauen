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

  // Inject the LAU namespace into the scope so views can use
  // helper functions as well
  $scope.LAU = LAU;

  // Initialize editor
  $scope.reloadProject = function() {
    $window.location.reload();
  };

  setupConsole($scope, $timeout);
});
var lau;
