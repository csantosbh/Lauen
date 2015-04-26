'use strict';

/**
 * @ngdoc function
 * @name lauEditor.controller:MainCtrl
 * @description
 * # MainCtrl
 * Controller of the lauEditor
 */
angular.module('lauEditor')
.controller('MainCtrl', function ($scope) {
  $scope.awesomeThings = [
  'HTML5 Boilerplate',
  'AngularJS',
  'Karma'
  ];
  $scope.lau = [2.0, {lauzito: 1.5}];

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
});
