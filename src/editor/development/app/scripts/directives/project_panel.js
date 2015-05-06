'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor')
  .directive('projectPanel', function () {
    return {
      templateUrl: 'views/directives/project_panel.html',
      restrict: 'E',
      transclude: true,
    };
  });
