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
          scope.gameObjectEditor.addComponent(scope.projectPanel.dropBucket);
        }
      };

      $rpc.call('getAssetList', null, function(fileList) {
        $timeout(function() {
          for(var i=0; i < fileList.length; ++i) {
            scope.projectPanel.projectFiles.push({
              menu_label: LAU.IO.getFileNameFromPath(fileList[i].path),
              type: 'script',
              flyweight: fileList[i]
            });
          }
          $event.broadcast('initialAssetList', scope.projectPanel.projectFiles);
        });
      });
    }
  };
}]);
