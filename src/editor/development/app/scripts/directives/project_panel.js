'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', ['$timeout', function ($timeout) {
  return {
    templateUrl: 'views/directives/project_panel.html',
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      scope.projectPanel = {
        projectFiles: [],
        onDrop: function() {
          console.log('hue!');
          console.log(scope.projectPanel.dropBucket);
        }
      };

      $rpc.call('getAssetList', null, function(fileList) {
        $timeout(function() {
          scope.projectPanel.projectFiles = fileList;
          $event.broadcast('initialAssetList', fileList);
        });
      });
    }
  };
}]);
