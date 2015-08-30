'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:v3f
 * @description
 * # v3f
 */
angular.module('lauEditor')
  .directive('v3f', function () {
    return {
      templateUrl: 'views/directives/component_editors/v3f.html',
      restrict: 'E',
      scope: {bind: '='},
      link: function postLink(scope, element, attrs) {
      }
    };
  });
