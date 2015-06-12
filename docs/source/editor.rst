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

* **Menu bar** ``views/directives/menu_bar.html`` (view) and ``scripts/directives/menu_bar.html`` (logic). *Its initialization logic is not yet refactored*
* **Hierarchy** *Not yet refactored*
* **Canvas** ``scripts/directives/edit_canvas.js`` (logic). *Initialization logic not yet refactored*.
* **Game Object Editor** ``views/directives/game_object_editor.html`` (container view) and ``scripts/directives/game_object_editor.js`` (logic). Each component type (like Transform and Script) is implemented as its own directive, with their views located inside ``views/directives/component_editors/``, and one single controller in ``scripts/directives/component_editor.js``. Finally, each script field widget (number editor, color picker, etc) is a directive whose view is located inside ``views/directives/component_editors/script_fields``. *We still have no specific folder for storing their logic files*.
* **Project Panel** ``views/directives/project_panel.html`` (view) and ``scripts/directives/project_panel.js`` (logic). *Initialization logic not yet refactored*.

====
Events
====
events are... (link to gpp)

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

