'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectHierarchy
 * @description
 * # gameObjectHierarchy
 */
angular.module('lauEditor')
  .directive('gameObjectHierarchy', ['gameObjectManager', 'lauGameObject', function ($gom, $lauGameObj) {
    return {
      templateUrl: 'views/directives/game_object_hierarchy.html',
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        scope.gameObjectHierarchy = {
          createGameObject: function() {
            $gom.pushGameObject(new $lauGameObj.GameObject());
          },
          gameObjects: $gom.getGameObjects,
          selectGameObject: $gom.selectGameObject,
          selectedGameObject: $gom.selectedGameObject,
          removeGameObject: function(idx) {
            $gom.removeGameObjectByIndex(scope, idx);
          }
        };
      }
    };
  }]);
