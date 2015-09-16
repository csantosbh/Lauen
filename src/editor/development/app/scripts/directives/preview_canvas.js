'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:previewCanvas
 * @description
 * # previewCanvas
 */
angular.module('lauEditor')
.directive('previewCanvas', ['$timeout', 'gameObjectManager', 'lauGameObject', 'componentManager', 'editorStateManager', function ($timeout, $gom, $lgo, $cm, $esm) {
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
              var newGameObj = new $lgo.GameObject(msg.newGameObjects[i], msg.newGameObjects[i].instanceId);
              $gom.pushGameObject(newGameObj);
              if(msg.newGameObjects[i].parentId != null) {
                $gom.moveGameObjectTo(newGameObj, $gom.getGameObject(msg.newGameObjects[i].parentId));
              }
            }
          }
          // Add new components
          if(msg.newComponents.length > 0) {
            for(var i = 0; i < msg.newComponents.length; ++i) {
              var gameObj = $gom.getGameObject(msg.newComponents[i].instanceId);
              var componentData = $cm.createComponentFromId(gameObj,
                msg.newComponents[i].component.componentId,
                msg.newComponents[i].component.instanceId);
              gameObj.addComponent(componentData);
            }
          }
          // Update states
          if(msg.currentStates.length > 0) {
            for(var i = 0; i < msg.currentStates.length; ++i) {
              var state = msg.currentStates[i];
              var gameObj = $gom.getGameObject(state.instanceId);
              // Update its components
              gameObj.updateStates(state);
            }
          }
          // Remove game object
          if(msg.deletedGameObjects.length > 0) {
            for(var i = 0; i < msg.deletedGameObjects.length; ++i) {
              var deletedObj = msg.deletedGameObjects[i];
              $gom.removeGameObject(deletedObj.instanceId);
            }
          }
          // Remove component
          if(msg.deletedComponents.length > 0) {
            for(var i = 0; i < msg.deletedComponents.length; ++i) {
              var deletedObj = msg.deletedComponents[i];
              $gom.getGameObject(deletedObj.gameObjectId).removeComponent(deletedObj.componentId);
            }
          }
        }

        // Check if the edit mode is still enabled,
        // to avoid race conditions.
        $timeout(function() {
          if(!$esm.isEditMode()) {
            processMessage();
          }
        });
      }

      element.find('embed')[0].addEventListener('message', handleNaClMessage, true);
      var _toggleRequested = false;
      scope.previewCanvas = {
        toggleEditMode: function() {
          if(!_toggleRequested) {
            if($esm.isEditMode() == true) {
              _toggleRequested = true;
              $rpc.call('previewGame', null, function(status) {
                $timeout(function() {
                  $event.broadcast('togglePreviewMode', true);
                  console.log('build status: ' + status);
                  $esm.disableEditMode();
                  _toggleRequested = false;
                });
              });
            } else {
              _toggleRequested = true;
              $timeout(function() {
                $event.broadcast('togglePreviewMode', false);
                _toggleRequested = false;
                $esm.enableEditMode();
              });
            }
          }
        }
      }
      Object.defineProperty(scope.previewCanvas, '_previewToggleMode', {
        get: function() {
          return !$esm.isEditMode();
        },
        set: function(v) {},
      });
    }
  };
}]);
