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
      var preCommitValue = null;
      // Handle focus
      inputElement.bind('focus', function() {
        preCommitValue = scope.bind;
      })
      // Handle blur
      .bind('blur', function() {
        var afterValue = scope.bind;

        // Do not push commands that dont change anything
        if(afterValue == preCommitValue) return;

        scope.commitCallback()(preCommitValue, afterValue);

        preCommitValue = afterValue;
      })
      // Handle <enter>
      .bind('keydown', function(downE) {
        if(downE.keyCode === 13) {
          inputElement.blur();
        }
      });
      $(inputElement).colorpicker({
        parts: ['header', 'map', 'bar', 'hex',
          'hsv', 'rgb', 'alpha', 'preview',
          'lab', 'cmyk'],
        alpha:          true,
        showOn:         'both',
        buttonColorize: true,
        position: {
          my: "center",
          at: "center",
          of: window
        },
        //buttonImage: 'images/colorpicker/ui-colorpicker.png',
        showNoneButton: true
      });
    }
  };
});
