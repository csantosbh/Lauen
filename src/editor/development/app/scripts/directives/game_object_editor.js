'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectEditor
 * @description
 * # gameObjectEditor
 */
angular.module('lauEditor').directive('gameObjectEditor', function () {
  function setupComponentMenu($scope) {
    var componentTypes = {
      'transform': {label:'Transform', flyweight: null }
    };

    $rpc.call('getDefaultComponents', null, function(dcs) {
      for(var i in dcs) {
        if(dcs.hasOwnProperty(i)) {
          componentTypes[i].flyweight = dcs[i];
          componentTypes[i].type = i;
        }
      }
    });

    $scope.componentMenu = {
      'Basic': [componentTypes.transform],
      'Scripts':[]
    };

    $scope.menuPickup = function(item){
      // The item array contains the menu item selected
      $event.broadcast('addComponent', $scope.componentMenu[item[0]][item[1]]);
    };
    $event.listen('initialAssetList', function(fileList) {
      $scope.componentMenu['Scripts'] = fileList;
    });
  }

  return {
    templateUrl: 'views/directives/game_object_editor.html',
    restrict: 'E',
    transclude: true,
    link: function postLink(scope, element) {
      setupComponentMenu(scope);
      $event.listen('addComponent', function(eventData) {
        if(scope.currentGameObjectId < 0) return;

        var componentData = LAU.Components.componentFactory(eventData.type, eventData.flyweight);
        scope.gameObjects[scope.currentGameObjectId].components.push(componentData);
      });
    },
  };
});
