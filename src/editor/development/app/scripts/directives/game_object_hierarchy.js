'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectHierarchy
 * @description
 * # gameObjectHierarchy
 */
angular.module('lauEditor')
  .directive('gameObjectHierarchy', ['gameObjectManager', 'lauGameObject', 'historyManager', function ($gom, $lgo, $hm) {
    return {
      templateUrl: 'views/directives/game_object_hierarchy.html',
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        scope.gameObjectHierarchy = {
          createGameObject: function() {
            var newGameObj = new $lgo.GameObject({name: 'Unnamed'});
            $gom.pushGameObject(newGameObj);

            // Handle undo/redo
            $hm.pushCommand({
              _instanceId: newGameObj.instanceId,
              undo: function() {
                $gom.removeGameObject(this._instanceId);
              },
              redo: function() {
                $gom.pushGameObject(new $lgo.GameObject({name: 'Unnamed'}, this._instanceId));
              }
            });
          },
          gameObjects: $gom.getGameObjects,
          selectedGameObject: $gom.selectedGameObject,
          droppableOptions: {
            accept: function(draggedElement) {
              var draggedGameObj = draggedElement.draggable().scope().gameObject;
              return draggedGameObj != undefined;
            }
          },
          onDrop: function(event, draggedElement) {
            var draggedGameObj = draggedElement.draggable.scope().gameObject;
            $gom.moveGameObjectTo(draggedGameObj, null);
          }
        };
      }
    };
  }]);
