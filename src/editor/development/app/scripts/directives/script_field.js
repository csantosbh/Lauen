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
    }
  }
  return {
    restrict: 'E',
    link: function postLink(scope, element) {
      $templateRequest(getTemplateName(scope.fieldContent.type)).then(function(template){
        element.html(template);
        $compile(element.contents())(scope);
      });
    }
  };
});
