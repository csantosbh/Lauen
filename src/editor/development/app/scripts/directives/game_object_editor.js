'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectEditor
 * @description
 * # gameObjectEditor
 */
angular.module('lauEditor').directive('gameObjectEditor', ['gameObjectManager', 'componentManager', 'lauComponents', function ($gom, $cm, $lauComps) {
  function setupComponentMenu($scope) {
    $scope.gameObjectEditor = {
      componentMenu: $cm.getComponentMenu(),
      gameObjects: $gom.getGameObjects,
      selectedGameObject: $gom.selectedGameObject,
      _menuPickup: function(item) {
        var componentType = $scope.gameObjectEditor.componentMenu;
        for(var i = 0; i < item.length; ++i) {
          componentType = componentType[item[i]];
        }
        // The item array contains the menu item selected
        $scope.gameObjectEditor.addComponent(componentType);
      },
      addComponent: function(eventData) {
        if($gom.selectedGameObject() < 0) return;

        var currentGameObj = $gom.selectedGameObject();
        var componentData = $lauComps.createComponentFromFlyWeight(currentGameObj, eventData.flyweight);
        $gom.addComponentToSelectedGameObject(componentData);
      }
    };
  }

  return {
    templateUrl: 'views/directives/game_object_editor.html',
    restrict: 'E',
    transclude: true,
    link: function postLink(scope, element) {
      setupComponentMenu(scope);
    },
  };
}]);
