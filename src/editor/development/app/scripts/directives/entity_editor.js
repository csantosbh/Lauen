'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:entityEditor
 * @description
 * # entityEditor
 */
angular.module('lauEditor')
  .directive('entityEditor', function () {
    return {
      templateUrl: 'views/directives/entity_editor.html',
      restrict: 'E',
      transclude: true,
    };
  });
