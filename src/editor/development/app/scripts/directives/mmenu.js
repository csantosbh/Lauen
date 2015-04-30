'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:mmenu
 * @description
 * # mmenu
 */
angular.module('lauEditor')
.directive('mmenu', function ($interpolate) {
  function genMenu(lvlElements) {
    var newMenu = '';
    for(var i in lvlElements) {
      if(lvlElements.hasOwnProperty(i)) {
        if(lvlElements[i] instanceof Object) {
          for(var j in lvlElements[i]) {
            if(lvlElements[i].hasOwnProperty(j))
              newMenu = newMenu+'<li><a href="#">'+j+'</a><ul>'+genMenu(lvlElements[i][j])+'</ul></li>';
          }
        } else {
          newMenu = newMenu+'<li><a href="#">'+lvlElements[i]+'</a></li>';
        }
      }
    }
    return newMenu;
  }

  var mmenuOptions = {
    offCanvas: false,
    searchfield: true,
    onClick: {
      setSelected: false
    }
  };
  var mmenuParameters = {
    transitionDuration: 5000,
  };

  return {
    template: '<div><nav></nav></div>',
    restrict: 'E',
    scope: {elements: '=elements'},
    link: function postLink(scope, element, attrs) {
      element.mmenu(mmenuOptions, mmenuParameters);

      scope.$watch('elements', function() {
        // Delete old menu
        var elementDOM=element.get()[0];
        while(elementDOM.firstChild) {
          elementDOM.removeChild(elementDOM.firstChild);
        }
        element.removeData('mmenu');

        // Create new menu
        var newMenu = '<ul>' + genMenu(scope.elements) + '</ul>';
        element.append($(newMenu))
        .mmenu(mmenuOptions, mmenuParameters);
      }, true);
    }
  };
});
