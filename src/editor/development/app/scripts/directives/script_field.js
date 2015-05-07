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
      break;
      case 'float':
        return 'views/directives/component_editors/script_fields/float.html';
      break;
    }
  }
  return {
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      // The parent scope is defined in initializeComponent()
      $templateRequest(getTemplateName(scope.$parent.component.metadata.fieldTypes[scope.field.name])).then(function(template){
        element.html(template);
        $compile(element.contents())(scope);
      });
    }
  };
});