'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectHierarchy
 * @description
 * # gameObjectHierarchy
 */
angular.module('lauEditor')
  .directive('gameObjectHierarchy', ['gameObjectManager', 'lauGameObject', function ($gom, $lgo) {
    return {
      templateUrl: 'views/directives/game_object_hierarchy.html',
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        scope.gameObjectHierarchy = {
          createGameObject: function() {
            $gom.pushGameObject(new $lgo.GameObject({name: 'Unnamed'}));
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
