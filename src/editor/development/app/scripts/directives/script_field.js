'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:scriptField
 * @description
 * # scriptField
 */
angular.module('lauEditor')
.directive('scriptField', function ($templateRequest, $compile) {
  function getTemplateName(fieldType) {
    switch(fieldType) {
      case 'int':
        return 'views/directives/component_editors/script_fields/int.html';
      case 'float':
        return 'views/directives/component_editors/script_fields/float.html';
      case 'v3f':
        return 'views/directives/component_editors/script_fields/v3f.html';
      case 'v2f':
        return 'views/directives/component_editors/script_fields/v2f.html';
    }
  }
  return {
    restrict: 'E',
    link: function postLink(scope, element) {
      $templateRequest(getTemplateName(scope.component.flyweight.types[scope.fieldName])).then(function(template){
        element.html(template);
        $compile(element.contents())(scope);
      });
    }
  };
});
