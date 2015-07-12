'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:previewCanvas
 * @description
 * # previewCanvas
 */
angular.module('lauEditor')
.directive('previewCanvas', ['$timeout', 'gameObjectManager', function ($timeout, $gom) {
  return {
    template: '<embed class="inner-canvas" src="lau_canvas.nmf" type="application/x-pnacl" />',
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
              var newGameObject = new LAU.GameObject(scope, null, null, msg.newGameObjects[i].instanceId);
              $gom.pushGameObject(newGameObject);
            }
          }
          // Add new components
          if(msg.newComponents.length > 0) {
            for(var i = 0; i < msg.newComponents.length; ++i) {
              var gameObj = $gom.getGameObjectByInstanceId(msg.newComponents[i].instanceId);
              var componentData = LAU.Components.createComponentFromId(
                msg.newComponents[i].component.componentId,
                scope,
                msg.newComponents[i].component.instanceId);
                // TODO use the game object manager to add this component
                /*
              console.log('added comp:');
              console.log(componentData);
             */
              gameObj.components.push(componentData);
            }
          }
          // Remove game object
          if(msg.deletedGameObjects.length > 0) {
            for(var i = 0; i < msg.deletedGameObjects.length; ++i) {
              var deletedObj = msg.deletedGameObjects[i];
              $gom.removeGameObjectByInstanceId(deletedObj.instanceId);
            }
          }
          // Update states
          if(msg.currentStates.length > 0) {
            for(var i = 0; i < msg.currentStates.length; ++i) {
              var state = msg.currentStates[i];
              var gameObj = $gom.getGameObjectByInstanceId(state.instanceId);
              // Update its components
              /*
              console.log('up state');
              console.log(state);
             */
              gameObj.updateStates(state);
            }
          }
        }

        // Check if the edit mode is still enabled,
        // to avoid race conditions.
        if(!scope.canvas.editMode) {
          $timeout(processMessage);
        }
      }

      element.find('embed')[0].addEventListener('message', handleNaClMessage, true);
      lau = $gom;
      var _editRequested = false;
      scope.previewCanvas = {
        toggleEditMode: function() {
          if(!_editRequested) {
            if(scope.canvas.editMode == true) {
              _editRequested = true;
              $rpc.call('previewGame', null, function(status) {
                $timeout(function() {
                  $event.broadcast('togglePreviewMode', true);
                  console.log('build status: ' + status);
                  scope.canvas.editMode = false;
                  _editRequested = false;
                });
              });
            } else {
              _editRequested = true;
              $timeout(function() {
                $event.broadcast('togglePreviewMode', false);
                _editRequested = false;
                scope.canvas.editMode = true;
              });
            }
          }
        }
      }

    }
  };
}]);
