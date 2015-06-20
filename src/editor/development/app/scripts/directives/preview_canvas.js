'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:previewCanvas
 * @description
 * # previewCanvas
 */
angular.module('lauEditor')
.directive('previewCanvas', function () {
  function handleNaClMessage(msg) {
    console.log('got nacl message:');
    console.log(msg);
  }

  return {
    template: '<embed class="main-canvas" src="lau_canvas.nmf" type="application/x-pnacl" />',
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      element.find('embed')[0].addEventListener('message', handleNaClMessage, true);
    }
  };
});
