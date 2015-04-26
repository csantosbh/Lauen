'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:numberInput
 * @description
 * # numberInput
 */
angular.module('lauEditor').directive('numberInput', function () {
  // TODO: document these attributes
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

      // Handle <enter>
      inputElement.bind('keydown', function(downE) {
        if(downE.keyCode === 13) {
          inputElement.blur();
        }
      });
      // Handle mouse-based value changing
      labelElement.bind('mousedown', function(downE) {
        var $document = angular.element(document);
        var prevMouse = {x: downE.pageX, y: downE.pageY};

        // Catch mouse move event
        function mouseMoveDocument(moveE) {
          var xDiff = moveE.pageX - prevMouse.x;
          if(xDiff===0 && moveE.pageY===downE.pageY){
            xDiff = moveE.pageX===0?-1:1;
          }
          xDiff *= attrs.sensitivity;

          inputElement.val(parseFloat(inputElement.val())+xDiff);
          inputElement.change();
          prevMouse.x = moveE.pageX, prevMouse.y = moveE.pageY;
          moveE.preventDefault();
        }
        $document.bind('mousemove', mouseMoveDocument);

        // Catch mouse up event
        $document.bind('mouseup', function mouseUpDocument() {
          // Don't need to keep these events bound
          $document.unbind('mouseup', mouseUpDocument);
          $document.unbind('mousemove', mouseMoveDocument);
        });
      });
    },
    restrict: 'E',
  };
});
