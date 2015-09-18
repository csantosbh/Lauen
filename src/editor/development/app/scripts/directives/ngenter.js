'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:ngEnter
 * @description
 * # ngEnter
 */
angular.module('lauEditor').directive('ngEnter', function () {
  return {
    restrict: 'A',
    link: function postLink(scope, element, attrs) {
      element.bind('keydown keypress', function (event) {
        if (event.which === 13) {
          scope.$apply(function () {
            scope.$eval(attrs.ngEnter, {$event: event});
          });
          event.preventDefault();
        }
      });
    }
  };
});

