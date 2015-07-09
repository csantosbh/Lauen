'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectHierarchy
 * @description
 * # gameObjectHierarchy
 */
angular.module('lauEditor')
  .directive('gameObjectHierarchy', ['gameObjectManager', function ($gom) {
    return {
      templateUrl: 'views/directives/game_object_hierarchy.html',
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        scope.gameObjectHierarchy = {
          createGameObject: function() {
            $gom.pushGameObject(new LAU.GameObject(scope));
          },
          gameObjects: $gom.gameObjects,
          selectGameObject: $gom.selectGameObject,
          selectedGameObject: $gom.selectedGameObject
        };
      }
    };
  }]);
