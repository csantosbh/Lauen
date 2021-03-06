'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:componentEditor
 * @description
 * # componentEditor
 */
angular.module('lauEditor')
  .directive('componentEditor', function ($templateRequest, $compile) {
    function getTemplateName(controllerType) {
      switch(controllerType) {
        case 'transform':
          return 'views/directives/component_editors/transform_editor.html';
        case 'camera':
          return 'views/directives/component_editors/camera_editor.html';
        case 'script':
          return 'views/directives/component_editors/script_editor.html';
        case 'mesh':
          return 'views/directives/component_editors/mesh_editor.html';
        case 'mesh_renderer':
          return 'views/directives/component_editors/mesh_renderer_editor.html';
        case 'light':
          return 'views/directives/component_editors/light_editor.html';
        case 'skinned_mesh_renderer':
          return 'views/directives/component_editors/skinned_mesh_renderer_editor.html';
      }
    }
    return {
      restrict: 'E',
      scope: {
        component: '=',
      },
      link: function postLink(scope, element) {
        $templateRequest(getTemplateName(scope.component.type)).then(function(template) {
          element.html(template);
          $compile(element.contents())(scope);
        });
      }
    };
  });
