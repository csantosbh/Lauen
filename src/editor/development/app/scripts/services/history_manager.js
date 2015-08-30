'use strict';

/**
 * @ngdoc service
 * @name lauEditor.historyManager
 * @description
 * # historyManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('historyManager', ['$timeout', '$rootScope', function ($timeout, $rootScope) {
  var history_length = 80;

  // TODO move the keystroke listeners to some keyhandler service
  window.addEventListener('keydown', function(event) {
    if(event.ctrlKey) {
      var keyVal = String.fromCharCode(event.which);
      if(keyPressToActionMap.hasOwnProperty(keyVal)) {
        keyPressToActionMap[keyVal]();
      }
      event.preventDefault();
      event.stopPropagation();
    }
  });

  var keyPressToActionMap = {
    'Z': performUndo,
    'Y': performRedo,
  };

  var defaultSettings = {
    passthrough: false
  };

  var uncommittedPassthroughs = [];

  var commands = [];
  var currentCommand = -1;

  function undoUncommittedPassthroughs() {
    if(uncommittedPassthroughs.length > 0) {
      let passThroughIdx = uncommittedPassthroughs.length-1;
      while(passThroughIdx >= 0) {
        uncommittedPassthroughs[passThroughIdx--].undo();
        $rootScope.$digest();
      }
      uncommittedPassthroughs = [];
    }
  }

  function performUndo() {
    $timeout(function() {
      function _undo() {
        if(currentCommand >= 0) {
          commands[currentCommand--].undo();
        }
      }
      undoUncommittedPassthroughs();

      // Undo committed passthroughs
      while(currentCommand >= 0 && commands[currentCommand].settings.passthrough) {
        _undo();
        $rootScope.$digest();
      }
      // Undo significant state change
      _undo();
    });
  }
  function performRedo() {
    $timeout(function() {
      function _redo() {
        if(currentCommand<commands.length-1) {
          commands[++currentCommand].redo();
        }
      }

      undoUncommittedPassthroughs();

      // Redo committed passthroughs
      while(currentCommand < commands.length-1 && commands[currentCommand+1].settings.passthrough) {
        _redo();
        $rootScope.$digest();
      }
      _redo();
    });
  }
  function pushCommand(cmd) {
    var _settings={};
    $.extend(_settings, defaultSettings, cmd.settings);
    cmd.settings = _settings;

    if(currentCommand<commands.length-1) {
      if(!cmd.settings.passthrough) {
        commands.splice(currentCommand+1, commands.length-1-currentCommand);
      } else {
        // TODO merge passthroughs
        uncommittedPassthroughs.push(cmd);
        return;
      }
    }

    if(uncommittedPassthroughs.length > 0) {
      commands = commands.concat(uncommittedPassthroughs);
      uncommittedPassthroughs = [];
    }

    commands.push(cmd);
    if(commands.length > history_length) {
      commands.splice(0,1);
    }

    currentCommand = commands.length-1;
  }

  return {
    pushCommand: pushCommand
  };
}]);
