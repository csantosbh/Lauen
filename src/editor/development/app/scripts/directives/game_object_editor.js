'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectEditor
 * @description
 * # gameObjectEditor
 */
angular.module('lauEditor').directive('gameObjectEditor', ['gameObjectManager', 'componentManager', 'lauComponents', 'historyManager', 'dragdropManager', function ($gom, $cm, $lc, $hm, $dm) {
  function setupComponentMenu($scope) {
    $scope.gameObjectEditor = {
      componentMenu: $cm.getComponentMenu(),
      onDrop:function(event, draggedElement) {
        $dm.dispatchAction(draggedElement, $scope, 'dropid_game_obj_editor');
      },
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
        var componentData = $lc.createComponentFromFlyWeight(currentGameObj, eventData.flyweight);

        // Undo/redo support
        $hm.pushCommand({
          _gameObjId: currentGameObj.instanceId,
          _componentId: componentData.instanceId,
          _compFlyweight: eventData.flyweight,
          undo: function() {
            let gameObj = $gom.getGameObject(this._gameObjId);
            gameObj.removeComponent(this._componentId);
          },
          redo: function() {
            let gameObj = $gom.getGameObject(this._gameObjId);
            let remadeComp = $lc.createComponentFromFlyWeight(gameObj, this._compFlyweight, this._componentId);
            gameObj.addComponent(remadeComp);
          }
        });

        currentGameObj.addComponent(componentData);
      }
    };

    $dm.registerAction('dragid_project_panel', 'dropid_game_obj_editor', function(draggedScope, dropScope) {
      $scope.gameObjectEditor.addComponent(draggedScope.file);
    });
  }

  return {
    templateUrl: 'views/directives/game_object_editor.html',
    restrict: 'E',
    transclude: true,
    link: function postLink(scope, element) {
      setupComponentMenu(scope);
      scope.doBlur = function(event) {
        event.target.blur();
      }
    },
  };
}]);
