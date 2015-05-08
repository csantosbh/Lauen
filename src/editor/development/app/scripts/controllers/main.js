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

  // Start websocket connection with server and listen to file change events
  //$socket.emit('evento', {doze: 12});
}

/*
 * Component menu
 */
function setupComponentMenu($scope, $timeout) {
  $scope.componentTypes = {'Scripts':[]};
  $scope.menuPickup=function(item){
    // The item array contains the menu item selected
    $event.broadcast('addComponent', {
      menuItem: item,
      prefab: $scope.componentTypes[item[0]][item[1]]
    });
  };
  $event.listen('assetlist', function(fileListEvent) {
    $timeout(function() {
      for(var i=0; i < fileListEvent.files.length; ++i) {
        $scope.componentTypes['Scripts'].push({
          label: fileListEvent.files[i].name,
          data: fileListEvent.files[i]
        });
      }
    });
  });
}

/*
 * Initialize script fields
 */
function getDefaultScriptFieldValue(type) {
  // TODO: Document initialization rules for ALL types of controllers
  switch(type) {
    case 'int':
    case 'float':
      return 0;
    break;
  }
}

/*
 * Initialize new components
 */
function initializeComponent(componentMenuItem, component) {
  // The switch rules match the component menu label
  switch(componentMenuItem[0]) {
    case 'Scripts':
      var componentData = {
        type: 'script',
        label: component.label,
        persistentData: {
          fields: []
        },
        metadata: {
          fieldTypes: {}
        }
      };
      // Initialize script fields
      var fields = component.data.classes.fields;
      for(var f = 0; f < fields.length; ++f) {
        // TODO: Check for visibility constraints
        componentData.persistentData.fields.push({
          value: getDefaultScriptFieldValue(fields[f].type),
          name: fields[f].name,
        });
        componentData.metadata.fieldTypes[fields[f].name] = fields[f].type;
      }
      return componentData;
      break;
  }
}

/*
 * Game Object editor menu
 */
function setupGameObjectEditorMenu($scope, $timeout) {
  $scope.currentGameObject = 0;
  setupComponentMenu($scope, $timeout);
  $event.listen('addComponent', function(componentData) {
    var componentData = initializeComponent(componentData.menuItem, componentData.prefab);
    $scope.gameObjects[$scope.currentGameObject].components.push(componentData);
  });
}

/*
 * Hierarchy panel
 */
function setupHierarchyPanel($scope, $timeout) {
  $scope.gameObjects = [{
    name: 'Static Object',
    components: [ {
        type: 'transform',
        position: null, // Will be set by the threejs canvas
        rotation: null, // Will be set by the threejs canvas
        scale: null, // Will be set by the threejs canvas
    } ]
  }];

  // TODO broadcast this whenever a new game object is created
  $event.broadcast('gameObjectCreated', 0);
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
  lau=$scope;
});
var lau;
