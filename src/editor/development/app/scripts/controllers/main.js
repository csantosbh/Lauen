'use strict';

/*
 * Project panel
 */
function setupProjectPanel(interact, $scope, $timeout) {
  $scope.projectFiles = [];
  $event.listen('assetlist', function(e) {
    // Instead of scope.$apply, which may not work if the
    // $digest is already running, use $timeout
    $timeout(function() {
      $scope.projectFiles = e.files;
    });
  });

  interact.maxInteractions(Infinity);

  // setup draggable elements.
  interact('.js-drag')
  .draggable({ max: Infinity })
  .on('dragstart', function (event) {
    event.interaction.x = parseInt(event.target.getAttribute('data-x'), 10) || 0;
    event.interaction.y = parseInt(event.target.getAttribute('data-y'), 10) || 0;
  })
  .on('move', function (event) {
    var interaction = event.interaction;
    var ay = interaction.y;

    // if the pointer was moved while being held down
    // and an interaction hasn't started yet
    if (interaction.pointerIsDown && !interaction.interacting()) {
      var original = event.currentTarget,
        // create a clone of the currentTarget element
        clone = event.currentTarget.cloneNode(true);

      // insert the clone to the page
      clone.style.position='absolute';
      clone.style.zIndex=800;
      document.body.appendChild(clone);

      // start a drag interaction targeting the clone
      interaction.start({ name: 'drag' },
                        event.interactable,
                        clone);
                        var originalBoundingRect = original.getBoundingClientRect();
                        event.interaction.x = -(-clone.offsetLeft-originalBoundingRect.left);
                        event.interaction.y = -(-originalBoundingRect.top);
    }
  })
  .on('dragmove', function (event) {
    event.interaction.x += event.dx;
    event.interaction.y += event.dy;

    event.target.style.left = event.interaction.x + 'px';
    event.target.style.top  = event.interaction.y + 'px';
  })
  .on('dragend', function (event) {
    event.target.setAttribute('data-x', event.interaction.x);
    event.target.setAttribute('data-y', event.interaction.y);
    interact(event.target).unset();
    event.target.parentNode.removeChild(event.target);
  });

  // setup drop areas.
  // dropzone #1 accepts draggable #1
  setupDropzone('#drop1', '#drag1');

  /**
   * Setup a given element as a dropzone.
   *
   * @param {HTMLElement|String} el
   * @param {String} accept
   */
  function setupDropzone(el, accept) {
    interact(el).dropzone({
      accept: accept,
      ondropactivate: function (event) {
        addClass(event.relatedTarget, '-drop-possible');
      },
      ondropdeactivate: function (event) {
        removeClass(event.relatedTarget, '-drop-possible');
      }
    })
    .on('dropactivate', function (event) {
      var active = event.target.getAttribute('active')|0;

      // change style if it was previously not active
      if (active === 0) {
        addClass(event.target, '-drop-possible');
        event.target.textContent = 'Drop me here!';
      }

      event.target.setAttribute('active', active + 1);
    })
    .on('dropdeactivate', function (event) {
      var active = event.target.getAttribute('active')|0;

      // change style if it was previously active
      // but will no longer be active
      if (active === 1) {
        removeClass(event.target, '-drop-possible');
        event.target.textContent = 'Dropzone';
      }

      event.target.setAttribute('active', active - 1);
    })
    .on('dragenter', function (event) {
      addClass(event.target, '-drop-over');
    })
    .on('dragleave', function (event) {
      removeClass(event.target, '-drop-over');
    })
    .on('drop', function (event) {
      removeClass(event.target, '-drop-over');
    });
  }

  function addClass (element, className) {
    if (element.classList) {
      return element.classList.add(className);
    }
    else {
      element.className += ' ' + className;
    }
  }

  function removeClass (element, className) {
    if (element.classList) {
      return element.classList.remove(className);
    }
    else {
      element.className = element.className.replace(new RegExp(className + ' *', 'g'), '');
    }
  }
}

/*
 * Component menu
 */
function setupComponentMenu($scope, $timeout) {
  $scope.componentTypes = {
    'Basic': [{label:'Transform', flyweight: null, type: 'transform' }],
    'Scripts':[]
  };

  $scope.menuPickup = function(item){
    // The item array contains the menu item selected
    $event.broadcast('addComponent', $scope.componentTypes[item[0]][item[1]]);
  };
  $event.listen('assetlist', function(fileListEvent) {
    $timeout(function() {
      for(var i=0; i < fileListEvent.files.length; ++i) {
        $scope.componentTypes['Scripts'].push({
          label: LAU.IO.getFileNameFromPath(fileListEvent.files[i].path),
          flyweight: fileListEvent.files[i],
          type: 'script'
        });
      }
    });
  });
}

/*
 * Game Object editor menu
 */
function setupGameObjectEditorMenu($scope, $timeout) {
  $scope.currentGameObjectId = 0;
  setupComponentMenu($scope, $timeout);
  $event.listen('addComponent', function(eventData) {
    if($scope.currentGameObjectId < 0) return;

    var componentData = LAU.Components.componentFactory(eventData.type, eventData.flyweight);
    $scope.gameObjects[$scope.currentGameObjectId].components.push(componentData);
  });
}

/*
 * Hierarchy panel
 */
function setupHierarchyPanel($scope, $timeout) {
  $scope.gameObjects = [];
  $scope.createGameObject = function() {
    $scope.gameObjects.push(new LAU.GameObject());
  }
  $scope.selectGameObject = function(i) {
    $scope.currentGameObjectId = i;
  }
}

// Menu bar (File, Edit, Help, etc..)
function setupMenuBar($scope, $timeout) {
  $scope.requestBuild = function() {
    $socket.broadcast('build', null);
  }
  $scope.requestSave = function() {
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
    $socket.broadcast('save', exported);
  }

  var isRequestingProject = false; // Multi-clock lock
  $scope.requestNewProject = function() {
    if(!isRequestingProject) {
      isRequestingProject = true;
      $rpc.call('createNewProject', null, function(folderName) {
        if(folderName.length > 0) {
          console.log('vou criar em '+folderName);
        }
        isRequestingProject = false;
      });
    }
    // TODO clear current project
  }
}

// Handle IO events
function setupIOEvents($scope, $timeout) {
  function handleIOEvents(io_event) {
    var sceneData = JSON.parse(io_event);

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

  $event.listen('loadCurrentScene', handleIOEvents);
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
angular.module('lauEditor').controller('MainCtrl', function ($scope, $timeout) {
  // Setup main layout
  $(".container").layout({
    resizeWhileDragging: true,
    north__spacing_open: 0,
    north__size: 50,
    east__size: 300,
  });
  $('#center-container').layout({
    resizeWhileDragging: true,
    south__size: 200,
  });

  // Setup project panel
  setupHierarchyPanel($scope, $timeout);
  setupProjectPanel(window.interact, $scope, $timeout);
  setupGameObjectEditorMenu($scope, $timeout);
  setupMenuBar($scope, $timeout);
  setupIOEvents($scope, $timeout);
  setupConsole($scope, $timeout);
  lau=$scope;

  // Inject the LAU namespace into the global scope
  $scope.LAU = LAU;

  // Spin the wheel! By connecting to the server, we'll
  // trigger communication between modules.
  $socket.connect();
});
var lau;
