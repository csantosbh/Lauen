'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:mmenu
 * @description
 * # mmenu
 */
angular.module('lauEditor')
.directive('mmenu', function ($compile) {
  function genMenu(lvlElements, callbackArguments) {
    var newMenu = '';
    for(var i in lvlElements) {
      if(lvlElements.hasOwnProperty(i) && lvlElements[i] != null) {
        var childArguments = callbackArguments.slice(0);
        childArguments.push(i);

        if(lvlElements[i].hasOwnProperty('children')) {
          // lvlElements[i] is a subcategory
          var catArguments = childArguments.slice(0);
          catArguments.push('children');
          newMenu = newMenu+'<li><a href="#">'+lvlElements[i].menu_label+'</a><ul>'+genMenu(lvlElements[i].children, catArguments)+'</ul></li>';
        } else {
          // lvlElements[i] is a component type
          newMenu = newMenu+'<li><a href="#" ng-click=\'elementSelected('+JSON.stringify(childArguments)+')\'>'+lvlElements[i].menu_label+'</a></li>';
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
    replace: true,
    scope: {
      elements: '=elements',
      callback: '=callback',
    },
    link: function postLink(scope, element) {
      scope.elementSelected = function(i) {
        scope.callback(i);
      };

      scope.$watch('elements', function() {
        // Delete old menu
        var elementDOM=element.get()[0];
        while(elementDOM.firstChild) {
          elementDOM.removeChild(elementDOM.firstChild);
        }
        element.removeData('mmenu');

        // Create new menu
        var newMenu = '<ul>' + genMenu(scope.elements, []) + '</ul>';
        var newMenuDOM = $compile($(newMenu))(scope.$new());

        element.append(newMenuDOM).mmenu(mmenuOptions, mmenuParameters);
      }, true);
    }
  };
});
