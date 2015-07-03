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
      'transform': {menu_label:'Transform', flyweight: null },
    };

    $rpc.call('getDefaultComponents', null, function(dcs) {
      for(var i in dcs) {
        if(dcs.hasOwnProperty(i)) {
          componentTypes[i].flyweight = dcs[i];
        }
      }
    });

    $scope.gameObjectEditor = {
      /*
       * Menu indices:
       * 0 transform component
       * 1 Scripts category
       */
      componentMenu: [
        componentTypes.transform,
        {menu_label: 'Scripts', children: []}
      ],
      _menuPickup: function(item) {
        var componentType = $scope.gameObjectEditor.componentMenu;
        for(var i = 0; i < item.length; ++i) {
          componentType = componentType[item[i]];
        }
        // The item array contains the menu item selected
        $scope.gameObjectEditor.addComponent(componentType);
      },
      addComponent: function(eventData) {
        if($scope.currentGameObjectId < 0) return;

        var componentData = LAU.Components.createComponentFromFlyWeight(eventData.flyweight);
        $scope.gameObjects[$scope.currentGameObjectId].components.push(componentData);
      }
    };

    $event.listen('initialAssetList', function(fileList) {
      $scope.gameObjectEditor.componentMenu[1].children = fileList;
    });
  }

  return {
    templateUrl: 'views/directives/game_object_editor.html',
    restrict: 'E',
    transclude: true,
    link: function postLink(scope, element) {
      setupComponentMenu(scope);
    },
  };
});
