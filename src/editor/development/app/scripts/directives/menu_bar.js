'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:menuBar
 * @description
 * # menuBar
 */
angular.module('lauEditor').directive('menuBar', ['$timeout', 'ngDialog', 'gameObjectManager', function ($timeout, $dialog, $gom) {
  return {
    templateUrl: 'views/directives/menu_bar.html',
    restrict: 'E',
    transclude: true,
    link: function postLink(scope, element) {
      // Load list of recent projects
      $rpc.call('getRecentProjects', null, function(recentProjects) {
        $timeout(function() {
          scope.menuBar.recentProjects = recentProjects;
        });
      });

      // TODO load list of recent projects
      var isRequestingProject = false; // Multi-click lock
      scope.menuBar = {
        recentProjects: [],
        requestBuild: function() {
          $rpc.call('buildGame', null, function(stat){});
        },
        requestSave: function() {
          var scene = $gom.serializeGameObjects();
          $rpc.call('save', scene, function(saveRes) {
            console.log('Save success: ' + saveRes);
          });
        },
        requestLoadProject: function(path) {
          if(!isRequestingProject) {
            isRequestingProject = true;
            $rpc.call('loadProject', path?path:null, function(success) {
              if(success) {
                scope.reloadProject();
              }
              isRequestingProject = false;
            });
          }
        },
        requestNewProject: function() {
          if(!isRequestingProject) {
            isRequestingProject = true;
            $rpc.call('createNewProject', null, function(folderName) {
              if(folderName.length > 0) {
                scope.reloadProject();
              }
              isRequestingProject = false;
            });
          }
          // TODO clear current project
        },
        requestBuildDialog: function() {
          $dialog.open({
            template: 'views/dialogs/build.html',
            scope: scope
          });
        },
        requestExport: function(buildAndRun) {
          $rpc.call('exportGame', {
            platform: scope.menuBar._requestBuildCompPlatform,
            buildAndRun: buildAndRun,
            compilationMode: scope.menuBar._requestBuildCompMode,
          }, function(status) {
            console.log(scope.menuBar._requestBuildCompMode);
            console.log('build status: ' + status);
          });
        },
      };

      // Internal fields
      scope.menuBar._requestBuildCompPlatform='linux';
      scope.menuBar._requestBuildCompMode='RELEASE';
    }
  };
}]);
