'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:componentEditor
 * @description
 * # componentEditor
 */
angular.module('lauEditor')
  .directive('componentEditor', function ($templateRequest, $compile) {
    function getTemplate(controllerType) {
      switch(controllerType) {
        case 'transform':
          return 'views/directives/component_editors/transform_editor.html';
        break;
        case 'script':
          return 'views/directives/component_editors/script_editor.html';
        break;
      }
    }
    return {
      restrict: 'E',
      link: function postLink(scope, element, attrs) {
        $templateRequest(getTemplate(scope.component.type)).then(function(template){
          element.html(template);
          $compile(element.contents())(scope);
        });
      }
    };
  });