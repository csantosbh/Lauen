'use strict';

/*
 * Component menu
 */
function setupComponentMenu($scope, $timeout) {
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

/*
 * Game Object editor menu
 */
function setupGameObjectEditorMenu($scope, $timeout) {
  $scope.currentGameObjectId = -1;
  setupComponentMenu($scope, $timeout);
  $event.listen('addComponent', function(eventData) {
    if($scope.currentGameObjectId < 0) return;

    var componentData = LAU.Components.componentFactory(eventData.type, eventData.flyweight);
    $scope.gameObjects[$scope.currentGameObjectId].components.push(componentData);
  });
}

// Menu bar (File, Edit, Help, etc..)
function setupMenuBar($scope, $timeout, $dialog) {
  // Load list of recent projects
  $rpc.call('getRecentProjects', null, function(recentProjects) {
    $timeout(function() {
      $scope.menuBar.recentProjects = recentProjects;
    });
  });

  // TODO load list of recent projects
  var isRequestingProject = false; // Multi-click lock
  $scope.menuBar = {
    recentProjects: [],
    requestBuild: function() {
      $rpc.call('buildGame', null, function(stat){});
    },
    requestSave: function() {
      var exported = [];
      for(var g = 0; g < $scope.gameObjects.length; ++g) {
        var gameObjComps = $scope.gameObjects[g].components;
        var exportedComps = [];
        for(var c = 0; c < gameObjComps.length; ++c) {
          exportedComps.push(gameObjComps[c].export());
        }
        exported.push({
          name: $scope.gameObjects[g].name,
          components: exportedComps
        });
      }
      $rpc.call('save', exported, function(saveRes) {
        console.log('Save success: ' + saveRes);
      });
    },
    requestLoadProject: function(path) {
      if(!isRequestingProject) {
        isRequestingProject = true;
        $rpc.call('loadProject', path?path:null, function(success) {
          if(success) {
            $event.broadcast('reloadProject', null);
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
            $event.broadcast('reloadProject', null);
          }
          isRequestingProject = false;
        });
      }
      // TODO clear current project
    },
    requestBuildDialog: function() {
      $dialog.open({
        template: 'views/dialogs/build.html',
        scope: $scope
      });
    },
    requestExport: function(buildAndRun) {
      $rpc.call('exportGame', {
        platform: $scope.menuBar._requestBuildCompPlatform,
        buildAndRun: buildAndRun,
        compilationMode: $scope.menuBar._requestBuildCompMode,
      }, function(status) {
        console.log($scope.menuBar._requestBuildCompMode);
        console.log('build status: ' + status);
      });
    },
  };

  // Internal fields
  $scope.menuBar._requestBuildCompPlatform='linux';
  $scope.menuBar._requestBuildCompMode='RELEASE';
}

// Handle IO events
function setupIOEvents($scope, $timeout) {
  function handleIOEvents(sceneData) {
    $timeout(function() {
      // Setup game objects
      for(var i = 0; i < sceneData.length; ++i) {
        var goComps = [];
        // TODO create gameObject class
        var comps = sceneData[i].components;
        for(var c = 0; c < comps.length; ++c) {
          var comp = LAU.Components.componentFactory(comps[c].type, comps[c], $scope, $timeout);
          if(comp == null)
            continue;
          goComps.push(comp);
        }

        $scope.gameObjects.push(new LAU.GameObject(sceneData[i].name, goComps));

        $event.broadcast('gameObjectCreated', i);
      }

      // TODO remove line below when the hierarchy panel is correctly created
      $scope.currentGameObjectId = $scope.gameObjects.length-1;
    });
  }

  $rpc.call('loadCurrentScene', null, handleIOEvents);
}

// Display errors/warnings/etc
function setupConsole($scope, $timeout) {
  $event.listen('compilationStatus', function(eMsg) {
    console.clear();
    console.log(eMsg.message);
  });
  $event.listen('executionMessage', function(eMsg) {
    console.log(eMsg);
  });
  $event.listen('executionStatus', function(eMsg) {
    console.clear();
    console.log(eMsg.message);
  });
}

/**
 * @ngdoc function
 * @name lauEditor.controller:MainCtrl
 * @description
 * # MainCtrl
 * Controller of the lauEditor
 */
angular.module('lauEditor').controller('MainCtrl', function ($scope, $timeout, $window, ngDialog) {
  $socket.connect();

  // Setup main layout
  $("#main-wnd-container").layout({
    resizeWhileDragging: true,
    north__spacing_open: 0,
    north__size: 70,
    east__size: 300,
  });
  $('#center-container').layout({
    resizeWhileDragging: true,
    south__size: 200,
  });

  // Inject the LAU namespace into the global scope
  $scope.LAU = LAU;
  // Initialize editor
  $scope.gameObjects = [];
  setupGameObjectEditorMenu($scope, $timeout);
  setupMenuBar($scope, $timeout, ngDialog);
  setupIOEvents($scope, $timeout);
  setupConsole($scope, $timeout);
  $event.listen('reloadProject', function() {
    $window.location.reload();
  });
});
var lau;
