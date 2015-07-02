'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:vector3
 * @description
 * # vector3
 */
angular.module('lauEditor')
  .directive('vector3', function () {
    return {
      templateUrl: 'views/directives/component_editors/vector3.html',
      restrict: 'E',
      scope: {parentModel: '=ngModel'},
      link: function postLink(scope, element, attrs) {
      }
    };
  });
