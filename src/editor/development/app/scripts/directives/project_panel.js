'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', function () {
  return {
    templateUrl: 'views/directives/project_panel.html',
    restrict: 'E',
    // TODO: transclude includes the whole parent scope here. It would be better to just receive whatever I need to make the file panel, and isolate my own scope
    transclude: true,
  };
});
