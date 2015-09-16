'use strict';
var $canvas;

/**
 * @ngdoc directive
 * @name lauEditor.directive:editCanvas
 * @description
 * # editCanvas
 */
angular.module('lauEditor').directive('editCanvas', ['$timeout', 'editCanvasManager', 'editorStateManager', function ($timeout, $editCanvas, $esm) {

  return {
    restrict: 'E',
    link: function postLink(scope, element) {
      $editCanvas.initCanvas(element);

      scope.editCanvas = {
        editMode: $esm.isEditMode
      };
    }
  };
}]);
