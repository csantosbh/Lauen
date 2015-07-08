'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:previewCanvas
 * @description
 * # previewCanvas
 */
angular.module('lauEditor')
.directive('previewCanvas', ['$timeout', function ($timeout) {
  return {
    template: '<embed class="inner-canvas" src="lau_canvas.nmf" type="application/x-pnacl" />',
    restrict: 'E',
    link: function postLink(scope, element, attrs) {

      function getGameObjectByInstanceId(id) {
        for(var i = 0; i < scope.gameObjects.length; ++i) {
          if(scope.gameObjects[i].instanceId == id)
            return scope.gameObjects[i];
        }
        // TODO assert that this line will never be achieved
        return null;
      }

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
              // TODO refactor this to somewhere else (a game object creation center)
              var newGameObject = new LAU.GameObject(scope, null, null, msg.newGameObjects[i].instanceId);
              scope.gameObjects.push(newGameObject);
            }
          }
          // Add new components
          if(msg.newComponents.length > 0) {
            for(var i = 0; i < msg.newComponents.length; ++i) {
              var gameObj = getGameObjectByInstanceId(msg.newComponents[i].instanceId);
              var componentData = LAU.Components.createComponentFromId(msg.newComponents[i].component.componentId, scope);
              // TODO refactor this below: it is a terrible thing to edit this guy like this
              componentData.instanceId = msg.newComponents[i].component.instanceId;
              gameObj.components.push(componentData);
            }
          }
          // Remove game object
          if(msg.deletedGameObjects.length > 0) {
            for(var i = 0; i < msg.deletedGameObjects.length; ++i) {
              // Look for deleted game object
              var gameObjs = scope.gameObjects;
              var deletedObj = msg.deletedGameObjects[i];
              for(var j = 0; j < gameObjs.length; ++j) {
                if(gameObjs[j].instanceId == deletedObj.instanceId) {
                  scope.gameObjects.splice(j, 1);
                  break;
                }
              }
            }
          }
          // Update states
          if(msg.currentStates.length > 0) {
            for(var i = 0; i < msg.currentStates.length; ++i) {
              var state = msg.currentStates[i];
              var gameObj = getGameObjectByInstanceId(state.instanceId);
              // Update its components
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
