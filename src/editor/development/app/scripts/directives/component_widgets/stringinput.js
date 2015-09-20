'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:stringinput
 * @description
 * # stringinput
 */
angular.module('lauEditor').directive('stringInput', function () {
  return {
    template: function(elem, userAttrs) {
      let tmpl = '<input ng-model="bind" ng-id="inpId" ng-class="inpClass"/>';

      if(userAttrs.label != undefined)
        tmpl = '<label ng-class="lblClass" for="{{inpId}}" ng-id="lblId">{{label}}</label>'+tmpl;

      return tmpl;
    },
    scope: {
      bind: '=',
      commitCallback: '&',
      lblClass: '@',
      lblId: '@',
      inpClass: '@',
      inpId: '@',
      label: '@',
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
    }
  };
});
