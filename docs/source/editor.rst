Editor
*****

The editor is written in JavaScript/HTML/CSS, and runs on a Webkit-based
window. The plethora of javascript libraries available on the internet allowed
the editor to be written extremely quickly, with a large amount of highly
interactible GUI elements. By combining AngularJS and ThreeJS, it was possible
to separate entities from their data, while still keeping them consistent in a
very manageable way. All of this would've had required a very large effort to
be accomplished with other languages, such as C++.

=======
Folder structure
=======
Any source code related to the editor is located inside ``$REPO_ROOT$/src/editor``. The ``development/`` folder contains the relevant source code, which was scaffolded with the `Yeoman tool <http://www.yeoman.io>`_; ``dist/`` will contain the compiled editor in the form of minified files, and is generated with the command ``grunt build -f``.

The relevant source code for the editor module lies within the ``development/app`` folder. It can be previewed in development mode by running ``grunt serve``.

====
Layout
====
The layout structure is defined in the file ``views/main.html``. It is composed of five major elements, namely:

* **Menu bar** Header element. Contains the major menu bar (File, Edit, etc), and the preview button.
* **Hierarchy** List of all game objects in the current scene.
* **Canvas** Edit/preview render canvas.
* **Game Object Editor** Panel for adding and editing game object components.
* **Project Panel** List of all assets in the current project.

.. figure:: imgs/editor_layout.svg

Each layout element is implemented as an AngularJS `directive <https://docs.angularjs.org/guide/directive>`_. They are implemented in the following files:

* **Menu bar** ``views/directives/menu_bar.html`` (view) and ``scripts/directives/menu_bar.js`` (logic).
* **Hierarchy** ``views/directives/game_object_hierarchy.html`` (view) and ``scripts/directives/game_object_hierarchy.js`` (logic).
* **Canvas** ``scripts/directives/edit_canvas.js`` (logic).
* **Game Object Editor** ``views/directives/game_object_editor.html`` (container view), ``scripts/directives/game_object_editor.js`` (high leven and panel initialization logic) and ``scripts/lau/component_prototypes.js`` (component instantiation logic).

  * **Component Editor** Each component type (like Transform and Script) is implemented as a ``component-editor`` directive (which is defined in ``scripts/directives/component_editor.js``), with their views dynamically determined based on the type of their flyweight component. Their views are located inside ``views/directives/component_editors/``.
  * **Script component Editor** Each script field widget (number editor, color picker, etc) is a directive implemented in ``scsripts/directives/script_field.js``, and whose view dynamically determined and loaded from ``views/directives/component_editors/script_fields``.
* **Project Panel** ``views/directives/project_panel.html`` (view) and ``scripts/directives/project_panel.js`` (logic).

==================
Game Object Editor
==================

All game objects are stored in the array ``$scope.gameObjects``, defined in ``scripts/controllers/main.js``. The boolean ``$scope.currentGameObjectId`` contains the index of the currently selected game object (the one highlighted in the hierarchy).

Each game object is an instance of a prototype defined in ``scripts/lau/game_object.js``. It has an array of components, whose prototypes are defined in ``scripts/lau/component_prototypes.js``.

-----------------------
Script Field directives
-----------------------

.. class:: numberInput

   This directive creates a number input whose value can be changed by dragging the mouse. A label can be specified via the ``label`` attribute.

   :param lblClass: CSS class for the input label. Default: ``number-input``.
   :param lblId: ID for the label wrapper.
   :param label: Label text.
   :param inpClass: CSS class for the input tag.
   :param inpId:  ID for the input tag.
   :param sensitivity: Defines how much will the input value change for each pixel that the mouse moves. Default: ``0.109375``.


====
Events
====
`Events <http://gameprogrammingpatterns.com/event-queue.html>`_ are the way different modules of the interface communicate with each other.

----
Listening and broadcasting events
----

----
Available events
----
list of available events...

====
RPCs
====
RPCs are... they are implemented as...
To see what RPCs are available, check out the ref <server rpcs>

====
Creating component types
====

====
Creating component widgets
====

