'use strict';

/**
 * @ngdoc service
 * @name lauEditor.historyManager
 * @description
 * # historyManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('historyManager', function () {
  var history_length = 40;

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

  var commands = [];
  var currentCommand = -1;

  function performUndo() {
    if(currentCommand >= 0) {
      commands[currentCommand--].undo();
    }
  }
  function performRedo() {
    if(currentCommand<commands.length-1) {
      commands[++currentCommand].redo();
    }
  }
  function pushCommand(cmd) {
    if(currentCommand >= 0 && currentCommand<commands.length-2) {
      commands.splice(currentCommand+1, commands.length-1-currentCommand);
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
});
