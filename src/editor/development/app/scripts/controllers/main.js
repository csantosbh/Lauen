'use strict';

// TODO: Move this socket service to its own file
var socket = new WebSocket("ws://localhost:9001");
socket.onopen = function(e) {
}
socket.onmessage = function(e) {
  var evData = JSON.parse(e.data);
  if(this.hasOwnProperty('callbacks_') && this.callbacks_.hasOwnProperty(evData.event)) {
    var cblist = this.callbacks_[evData.event];
    for(var i = 0; i < cblist.length; ++i) {
      cblist[i](evData);
    }
  }
  else {
    // Store message and deliver it when theres a subscriber
    if(!this.hasOwnProperty('pendingQueue_'))
      this.pendingQueue_ = {};
    if(!this.pendingQueue_.hasOwnProperty(evData.event))
      this.pendingQueue_[evData.event] = [];
    this.pendingQueue_[evData.event].push(evData);
  }
}
socket.subscribe = function(e, callback) {
  if(!this.hasOwnProperty('callbacks_'))
    this.callbacks_ = {};
  if(!this.callbacks_.hasOwnProperty(e))
    this.callbacks_[e] = [];
  this.callbacks_[e].push(callback);
  if(this.hasOwnProperty('pendingQueue_') && this.pendingQueue_.hasOwnProperty(e)) {
    var queue = this.pendingQueue_[e];
    for(var i = 0; i < queue.length; ++i) {
      callback(queue[i]);
    }
    delete this.pendingQueue_[e];
  }
}

/*
 * Project panel
 */
function setupProjectPanel(interact, $scope, $timeout) {
  // TODO the project panel could be its own view
  socket.subscribe('assetlist', function(e) {
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
  //socket.emit('evento', {doze: 12});
}

/**
 * @ngdoc function
 * @name lauEditor.controller:MainCtrl
 * @description
 * # MainCtrl
 * Controller of the lauEditor
 */
angular.module('lauEditor').controller('MainCtrl', function ($scope, $timeout) {
  $scope.awesomeThings = [
    'HTML5 Boilerplate',
    'AngularJS',
    'Karma'
  ];
  $scope.lau = [2.0, 1.5];
  $scope.projectFiles = [];

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
  $scope.componentTypes=['um', 'dois', {tres:['subum', 'subdois', 'subtres']}];
  setupProjectPanel(window.interact, $scope, $timeout);
  lau={s:$scope,t:$timeout};

});
var lau;
