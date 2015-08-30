'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:numberInput
 * @description
 * # numberInput
 */
angular.module('lauEditor').directive('numberInput', ['historyManager', function ($hm) {
  var defaultAttrs = {
    'lblClass': 'number-input',
    'lblId': '',
    'label': '',
    'inpClass': '',
    'inpId': '',
    'sensitivity': 1.0/8.0-1.0/64.0,
  };

  return {
    template: function(elem, userAttrs) {
      var attrs = {};
      $.extend(attrs, defaultAttrs, userAttrs);

      return `<label ng-class="lblClass" for="{{inpId}}" ng-id="lblId">{{label}}</label>
      <input ng-model="bind" type="number" ng-id="inpId" ng-class="inpClass"/>`;
    },
    scope: {
      bind: '=',
      commitCallback: '&',
      lblClass: '@',
      lblId: '@',
      inpClass: '@',
      inpId: '@',
      sensitivity: '@',
      label: '@',
    },
    link: function(scope, element, userAttrs) {
      var attrs = {};
      $.extend(attrs, defaultAttrs, userAttrs);
      var inputElement = element.find('input');
      var labelElement = element.find('label');
      var preCommitValue = null;

      // Handle focus
      inputElement.bind('focus', function() {
        preCommitValue = scope.bind;
      })
      // Handle blur
      .bind('blur', function() {
        publishHistoryCommand();
      })
      // Handle <enter>
      .bind('keydown', function(downE) {
        if(downE.keyCode === 13) {
          inputElement.blur();
        }
      });
      // Handle mouse-based value changing
      labelElement.bind('mousedown', function(event) {
        preCommitValue = scope.bind;
        $canvas.requestPointerLock();

        // Catch mouse move event
        function mouseMoveDocument(moveE) {
          var xDiff = moveE.movementX * attrs.sensitivity;

          inputElement.val(parseFloat(scope.bind)+xDiff);
          inputElement.change();
        }
        document.addEventListener('mousemove', mouseMoveDocument);

        // Catch mouse up event
        document.addEventListener('mouseup', function mouseUpDocument() {
          // Don't need to keep these events bound
          document.removeEventListener('mouseup', mouseUpDocument);
          document.removeEventListener('mousemove', mouseMoveDocument);
          publishHistoryCommand();
          document.exitPointerLock();
        });

        event.preventDefault();
        event.stopPropagation();
      });

      // Handle history commands
      function publishHistoryCommand() {
        var afterValue = scope.bind;

        // Do not push commands that dont change anything
        if(afterValue == preCommitValue) return;

        scope.commitCallback()(preCommitValue, afterValue);

        preCommitValue = afterValue;
      }
    },
    restrict: 'E',
  };
}]);
