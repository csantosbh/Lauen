'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:colorinput
 * @description
 * # colorinput
 */
angular.module('lauEditor').directive('colorInput', function () {
  return {
    template: function(elem, userAttrs) {
      let tmpl = '<input type="text" class="colorpicker" ng-model="bind">';

      if(userAttrs.label != undefined)
        tmpl = '<label ng-class="lblClass" for="{{inpId}}" ng-id="lblId">{{label}}</label>'+tmpl;

      return tmpl;
    },
    scope: {
      bind: '=',
      commitCallback: '&',
    },
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      var inputElement = element.find('input');
      var preCommitValue = scope.bind;
      var colorPicker = inputElement.colorpicker({
        parts: ['header', 'map', 'bar', 'hex',
          'hsv', 'rgb', 'alpha', 'preview',
          'lab', 'cmyk'],
        alpha:          true,
        showOn:        'both',
        colorFormat: 'HEXA',
        altOnChange: true,
        position: {
          my: "center",
          at: "center",
          of: window
        },
        buttonColorize: true,
        stop: function() {
          var afterValue = scope.bind;

          // Do not push commands that dont change anything
          if(afterValue == preCommitValue) return;

          scope.commitCallback()(preCommitValue, afterValue);

          preCommitValue = afterValue;
        }
      });
      // TODO update the preview button color when the model is updated outside
      // of the colorpicker
    }
  };
});
