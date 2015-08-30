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
      scope: {
        bind: '=',
        commitCallback: '&',
      },
      link: function postLink(scope, element, attrs) {
        scope._forwardCommitter = function(axis) {
          return function(oldVal, newVal) {
            var older = LAU.Utils.clone(scope.bind);
            var newer = LAU.Utils.clone(scope.bind);
            older[axis] = oldVal;
            scope.commitCallback()(older, newer);
          }
        };
      }
    };
  });
