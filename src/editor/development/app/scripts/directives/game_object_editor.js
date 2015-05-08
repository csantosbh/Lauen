'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectEditor
 * @description
 * # gameObjectEditor
 */
angular.module('lauEditor')
  .directive('gameObjectEditor', function () {
    return {
      templateUrl: 'views/directives/game_object_editor.html',
      restrict: 'E',
      transclude: true,
    };
  });
