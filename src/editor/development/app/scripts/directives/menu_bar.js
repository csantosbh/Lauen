'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:menuBar
 * @description
 * # menuBar
 */
angular.module('lauEditor').directive('menuBar', function () {
  return {
    templateUrl: 'views/directives/menu_bar.html',
    restrict: 'E',
    transclude: true
  };
});
