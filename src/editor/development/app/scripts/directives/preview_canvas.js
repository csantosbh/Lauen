'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:previewCanvas
 * @description
 * # previewCanvas
 */
angular.module('lauEditor')
.directive('previewCanvas', ['$timeout', 'gameObjectManager', 'lauGameObject', 'componentManager', 'editCanvasManager', function ($timeout, $gom, $lgo, $cm, $editCanvas) {
  return {
    template: '<embed class="inner-canvas" src="http://localhost:9002/lau_canvas.nmf" type="application/x-pnacl" />',
    restrict: 'E',
    link: function postLink(scope, element, attrs) {

      function handleNaClMessage(message) {
        function processMessage() {
          var msg = message.data;
          if(msg.messages.length > 0) {
            console.log(msg.messages);
          }
          if(msg.errors.length > 0) {
            console.log('%c[error] '+msg.errors, "color: DarkViolet; font-weight: bold;");
          }
          // Add new game objects
          if(msg.newGameObjects.length > 0) {
            for(var i = 0; i < msg.newGameObjects.length; ++i) {
              $gom.pushGameObject(new $lgo.GameObject(msg.newGameObjects[i], msg.newGameObjects[i].instanceId));
            }
          }
          // Add new components
          if(msg.newComponents.length > 0) {
            for(var i = 0; i < msg.newComponents.length; ++i) {
              var gameObj = $gom.getGameObjectByInstanceId(msg.newComponents[i].instanceId);
              var componentData = $cm.createComponentFromId(gameObj,
                msg.newComponents[i].component.componentId,
                msg.newComponents[i].component.instanceId);
                // TODO create an addComponent function to the gameObject prototype
              gameObj.components.push(componentData);
            }
          }
          // Remove game object
          if(msg.deletedGameObjects.length > 0) {
            for(var i = 0; i < msg.deletedGameObjects.length; ++i) {
              var deletedObj = msg.deletedGameObjects[i];
              $gom.removeGameObject(deletedObj.instanceId);
            }
          }
          // Update states
          if(msg.currentStates.length > 0) {
            for(var i = 0; i < msg.currentStates.length; ++i) {
              var state = msg.currentStates[i];
              var gameObj = $gom.getGameObjectByInstanceId(state.instanceId);
              // Update its components
              gameObj.updateStates(state);
            }
          }
        }

        // Check if the edit mode is still enabled,
        // to avoid race conditions.
        $timeout(function() {
          if(!$editCanvas.isEditMode()) {
            processMessage();
          }
        });
      }

      element.find('embed')[0].addEventListener('message', handleNaClMessage, true);
      var _toggleRequested = false;
      scope.previewCanvas = {
        toggleEditMode: function() {
          if(!_toggleRequested) {
            if($editCanvas.isEditMode() == true) {
              _toggleRequested = true;
              $rpc.call('previewGame', null, function(status) {
                $timeout(function() {
                  $event.broadcast('togglePreviewMode', true);
                  console.log('build status: ' + status);
                  $editCanvas.disableEditMode();
                  _toggleRequested = false;
                });
              });
            } else {
              _toggleRequested = true;
              $timeout(function() {
                $event.broadcast('togglePreviewMode', false);
                _toggleRequested = false;
                $editCanvas.enableEditMode();
              });
            }
          }
        }
      }
      Object.defineProperty(scope.previewCanvas, '_previewToggleMode', {
        get: function() {
          return !$editCanvas.isEditMode();
        },
        set: function(v) {},
      });
    }
  };
}]);
