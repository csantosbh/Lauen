'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:v2f
 * @description
 * # v2f
 */
angular.module('lauEditor')
  .directive('v2f', function () {
    return {
      templateUrl: 'views/directives/component_editors/v2f.html',
      restrict: 'E',
      scope: {parentModel: '=ngModel'},
      link: function postLink(scope, element, attrs) {
      }
    };
  });
