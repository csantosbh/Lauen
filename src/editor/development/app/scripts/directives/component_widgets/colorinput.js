'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:colorinput
 * @description
 * # colorinput
 */
angular.module('lauEditor')
  .directive('colorinput', function () {
    return {
      template: '<div></div>',
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        element.text('this is the colorinput directive');
      }
    };
  });
