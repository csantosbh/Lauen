'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:numberInput
 * @description
 * # numberInput
 */
angular.module('lauEditor').directive('numberInput', ['$timeout', 'historyManager', function ($timeout, $hm) {
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
      var ngModel = 'ng-model="'+attrs.ngModel+'"';

      return '<label class="'+attrs.lblClass +
             '" for="'+attrs.inpId +
             '" id="'+ attrs.lblId +
             '">'+ attrs.label +
             '</label><input '+ngModel +
             '" type="number" id="'+
             attrs.inpId + '" class="'+
             attrs.inpClass+'">';
    },
    link: function(scope, element, userAttrs) {
      var attrs = {};
      $.extend(attrs, defaultAttrs, userAttrs);
      var inputElement = element.find('input');
      var labelElement = element.find('label');
      var preCommitValue = null;

      // Handle focus
      inputElement.bind('focus', function() {
        preCommitValue = inputElement.val();
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
      labelElement.bind('mousedown', function() {
        preCommitValue = inputElement.val();
        $canvas.requestPointerLock();

        // Catch mouse move event
        function mouseMoveDocument(moveE) {
          var xDiff = moveE.movementX * attrs.sensitivity;

          inputElement.val(parseFloat(inputElement.val())+xDiff);
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
      });

      // Handle history commands
      function publishHistoryCommand() {
        var afterValue = inputElement.val();

        // Do not push commands that dont change anything
        if(afterValue == preCommitValue) return;

        $hm.pushCommand({
          _before: preCommitValue,
          _after: afterValue,
          undo: function() {
            inputElement.val(this._before);
            inputElement.change();
          },
          redo: function() {
            inputElement.val(this._after);
            inputElement.change();
          }
        });
        preCommitValue = afterValue;
      }
    },
    restrict: 'E',
  };
}]);
