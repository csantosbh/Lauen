'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectHierarchy
 * @description
 * # gameObjectHierarchy
 */
angular.module('lauEditor')
  .directive('gameObjectHierarchy', function () {
    return {
      templateUrl: 'views/directives/game_object_hierarchy.html',
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        scope.gameObjectHierarchy = {
          createGameObject: function() {
            scope.gameObjects.push(new LAU.GameObject(scope));
          },
          selectGameObject: function(i) {
            scope.currentGameObjectId = i;
          }
        };
      }
    };
  });
