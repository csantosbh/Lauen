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
Any source code related to the editor is located inside
``$REPO_ROOT$/src/editor``. The ``development/`` folder contains the relevant
source code, which was scaffolded with the `Yeoman tool
<http://www.yeoman.io>`_; ``dist/`` will contain the compiled editor in the
form of minified files, and is generated with the command ``grunt build -f``.

The relevant source code for the editor module lies within the
``development/app`` folder. It can be previewed in development mode by running
``grunt serve``.

The editor module is divided into the following folders:

* **scripts** JavaScript code.
* **views** HTML templates.
* **styles** SCSS files.

====
Layout
====
The layout structure is defined in the file ``views/main.html``. It is composed
of five major elements, namely:

* **Menu bar** Header element. Contains the major menu bar (File, Edit, etc),
  and the preview buttons.
* **Hierarchy** List of all game objects in the current scene.
* **Canvas** Edit/preview render canvas.
* **Game Object Editor** Panel for adding and editing game object components.
* **Project Panel** List of all assets in the current project.

.. figure:: imgs/editor_layout.svg

Each layout element is implemented as an AngularJS `directive
<https://docs.angularjs.org/guide/directive>`_. They are implemented in the
following files:

* **Menu bar** ``views/directives/menu_bar.html`` (view) and
  ``scripts/directives/menu_bar.js`` (logic).
* **Hierarchy** ``views/directives/game_object_hierarchy.html`` (view) and
  ``scripts/directives/game_object_hierarchy.js`` (logic).
* **Canvas** ``scripts/directives/edit_canvas.js`` (canvas initialization),
  ``scripts/services/edit_canvas_manager.js`` (exposes the canvas internal to
  other directives and services) and ``scripts/directives/preview_canvas.js``
  (preview logic).
* **Game Object Editor** ``views/directives/game_object_editor.html``
  (container view), ``scripts/directives/game_object_editor.js`` (GameObject
  Panel management) and ``scripts/services/lau_components.js`` (component
  instantiation and persistency logic).

  * **Component Editor** Each component type (like Transform and Script) is
    implemented as a ``component-editor`` directive (which is defined in
    ``scripts/directives/component_editor.js``), with their views dynamically
    determined based on the type of their flyweight component. Their views are
    located inside ``views/directives/component_editors/``.
  * **Script component Editor** Each script field widget (number editor, color
    picker, etc) is a directive implemented in
    ``scsripts/directives/script_field.js``, and whose view dynamically
    determined and loaded from
    ``views/directives/component_editors/script_fields``.
* **Project Panel** ``views/directives/project_panel.html`` (view) and
  ``scripts/directives/project_panel.js`` (logic). All components shown there
  are stored and managed by ``scripts/services/component_manager.js``.

==================
Game Object Manager
==================

All game objects are stored in the array ``gameObjectManager.gameObjects``,
defined in ``scripts/services/game_object_manager.js``. The variable
``currentGameObjectId`` (defined in the same service) contains the index of the
currently selected game object (the one highlighted in the hierarchy). If no
game object is selected, then this variable is set to -1.

Each game object is an instance of a prototype defined in
``scripts/services/lau_game_object.js``. It has an array of components, whose prototypes
are defined in ``scripts/services/lau_components.js``. It has the following
fields:


.. code-block:: javascript

    /// Game Object Prototype
    {
      components: Array[], // Array of components
      name: "string", // Component name
      instanceId: <int>, // Only set during preview mode, represents the
                         // id of that game object instance, used for
                         // linking the editor game object with its NaCl
                         // equivalent.
      constructor: function(scope, name='unnamed', components=[], instanceId=undefined),
      getComponentById: function(id), // Returns the first component whose id equals the
                                      // parameter id
      getComponentByInstanceId: function(id), // Returns the first component whose instance
                                              // id equals id
      updateStates: function(currentStates), // Updates all components within the provided
                                             // state array
      destroy: function(scope) // Game Object destructor. Must be manually called whenever
                               // a game object is permanently removed from scope.
    }

The Game Object Manager service has provides the following functions:

.. function:: getGameObjects() -> array

   Returns the list of game objects currently being displayed in the editor. If
   in edit mode, the game objects are the ones created by the user; in preview
   mode, they are the ones created by the NaCl previewer, and besides not being
   necessarily the same, they are internally two different arrays.

.. function:: selectGameObject(index)

   Selects a game object for editing. This will show all of its components in
   the Game Object Editor panel.

   :param index: The position of the desired game object in the gameObjects array.

.. function:: selectedGameObject() -> index

   Returns the index of the currently selected game object, or -1 if none are
   selected.

.. function:: pushGameObject(go)

   Pushes the requested game object to the array of game objects.

   :param go: A game object instance.

.. function:: removeGameObjectByIndex(idx)

   Destroys the game object whose index in the gameObject array is ``idx``. This
   will call its destroy() method before removing it from the gameObjects array.

   :param idx: The position of the game object in the gameObjects array.

.. function:: removeGameObjectByInstanceId(id)

   Destroys the game object whose instance id field equals ``id``. This will
   call its destroy() method and remove it from the gameObjects array.

   :param id: The game object instance id.

.. function:: removeScriptFromGameObjects(scriptFlyweight)

   Remove script component from all game objects in the current scene. Internally, component comparison is made by checking if their paths are equal. TODO implement this for all scenes! VERY IMPORTANT!

   :param scriptFlyweight: The flyweight of the component being removed from all game objects.


.. function:: addComponentToSelectedGameObject(component)

   Adds the component to the currently selected game object.

   :param component: The component instance to be added.

.. function:: serializeGameObjects() -> array

   Returns an array of objects containing serializable data from each
   instantiated game object.

-----------------------
Script Field directives
-----------------------

.. function:: <number-input lbl-class lbl-id label inp-class inp-id sensitivity/>

   This directive creates a number input whose value can be changed by dragging
   the mouse after clicking on the label. A label can be specified via the
   ``label`` attribute.

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

.. function:: $event.listen(event_name, callback)

   Listen to an event. The callback function will be called whenever the event
   is raised anywhere in the editor. It is possible to have many listeners to
   each event. Since there's no way to guarantee that the first event will be
   raised after all modules are done loading (angularjs doesn't have a
   definitive "doneLoading" event), event raising must be performed with care.

   :param event_name: String defining the name of the event.
   :param event_data: Callback function that will be executed when the event is broadcast.

Sample:

.. code-block:: javascript

    $event.listen('myEvent', function(eventData) {
        console.log(eventData);
    });

.. function:: $event.broadcast(event_name, event_data)

   Broadcasts an event with name defined by the string event_name. Every
   listener will receive event_data as a parameter.

   :param event_name: String defining the name of the event.
   :param event_data: Object with the event data.

Samples:

.. code-block:: javascript

    // This listener will lose the event fired below!
    $event.listen('myEvent', function(data) {
      console.log(data);
    });
    $event.broadcast('myEvent', {id: 123});


----
Available events
----

========================= ========================================
 Event name                Description                            
========================= ========================================
 gameObjectCreated         A new gameobject was created.
                           **Parameter:** The numeric index
                           referring to the position of the new
                           gameobject in the $scope.gameObjects
                           array.
 togglePreviewMode         Fired by the Preview Canvas to indicate
                           that the preview mode has either started
                           or stopped.
                           **Parameter:** A boolean variable. If true,
                           then the preview mode has just started;
                           otherwise, the edit mode was just turned on.
 paneResized               A layout panel has been resized. Fired for
                           all panels individually when they resize.
                           **Parameter:** The following object:
                           ``{
                           pane:"pane_id", size: {width: <new_width>
                           height: <new_height>}
                           }``
========================= ========================================

=============
Socket events
=============
Socket events are events that are broadcast to the server module. To broadcast
a message to the server, use ``$socket.broadcast("event_name", eventData)``.
The server can also broadcast events to the Editor frontend (check out the list
of server events :ref:`here <server-events>`). To subscribe, simply listen to
the corresponding event with the ``$event`` object:

.. code-block:: javascript

   $event.listen("server_event", function(eventData) {
     console.log(eventData);
   });

The socket module is implemented in ``scripts/socket.js``.

====
RPCs
====
RPC stands for *Remote Procedure Call*. There are many functions implemented in
the server module (like script compilation and file monitoring) that the editor
frontend must trigger, and they do so by exchanging websocket messages.

Whenever the editor needs to trigger a server function, it does so by
broadcasting the `RPCCall` socket event to the server, with the following
parameter:

.. code-block:: javascript

   {
     procedure: "procedure_name",
     from: randomUniqueID,
     parameters: userParameters
   }

The server receives this request, processes it (to see the details on
server-side RPCs, check out :ref:`Server RPC <server-rpc>`) and returns an
object to the callee (the return value varies across functions -- refer to
:ref:`Server RPC <server-rpc>` for more info on this).

In order to call an RPC, use the ``$rpc.call(procedureName, parameters,
callback)`` function. Example:

.. code-block:: javascript

   $rpc.call("serverMethod", {data: 12}, function(returnValue) {
     console.log(returnValue);
   });


The RPC module is implemented in ``scripts/rpc.js``.

====
Creating component types
====
For each new component type (for instance, a Transform or a Script component),
all of the following items are required:

* **An editor view** An HTML template with the component's editable fields.
  Must be located inside ``views/directives/component_editors/``.
* **Make the editor view loadable** Add a ``case 'component_name'`` to make the
  new component loadable in ``scripts/directives/component_editor.js``.
* **Add the new component to the component menu** This involves editing the
  file ``scripts/directives/game_object_editor.js``, and the required changes
  will depend on the type of component you are creating. This step is explained
  in detail :ref:`down below <add-component-to-menu>`.
* **Define a unique numeric id for the new component** This requires modifying
  **server** files in order to make these ids accessible from the whole
  project. It depends on the type of component being created, and is explained
  in detail :ref:`down below <define-unique-component-id>`.
* **Implement the runtime for the new component** If you are implementing the
  runtime for this component, its path and class name must be specified in the
  :ref:`server as well <implementing-custom-components>`.
* **Make the component persistent** In order to make the component persistent,
  you must implement the code that will export it to a serializable
  format, and the code that will receive data in that format and transform it
  back into something that the editor can use. This is done in the file
  ``services/lau_components.js``, and is explained :ref:`down below
  <persistent-components>`.
* **Make the component visualizable** If the component should be displayed in
  the edit canvas (for instance, the preview is highly dependent on the
  Transform component; and the Mesh component is expected to be displayed on
  the edit canvas), there :ref:`are some steps <canvas-consistency>` to make this component always
  consistent with the edit canvas.

.. _add-component-to-menu:

----
Adding new components to the Component Menu
----
The Component Menu, whose callbacks are managed in
``scripts/directives/game_object_editor.js``, displays all objects listed in
the array ``componentMenu``, defined in
``scripts/services/component_manager.js``.

This object is an array of dictionaries. Within this array, a component type is
a dictionary in the format:

.. code-block:: javascript

   {
     menu_label: "Component Type Label",
     flyweight: {..internal data...}
   }
   
and a category has the format:

.. code-block:: javascript

   {
     menu_label: "Component Type Label",
     children: [..components or subcategories...]
   }

Notice that a category may contain both components and subcategories.


The **menu_label** field in the component object is the name that will be
displayed in the Component Menu; the **flyweight** field points to an object
with implementation specific data (for instance, the unique numeric id and the
path to the file where the component is implemented). Typically, the
*flyweight* will be provided by the server through the ``getDefaultComponents``
RPC. This is the case when the component is implemented by a single class. In
different cases (for instance, the Script type has one implementation per file,
and is given by the engine user), the flyweight has to be managed and retrieved
from the server accordingly (the Script components are managed by the Project
Panel module, and are given by the server during startup and every time a new
script is detected). Normal components, however, are only required to be added
to the ``componentTypes`` object and directly to the menu via a reference to
the ``componentTypes`` object.

.. _define-unique-component-id:

----
Defining a unique numeric id for the new Component
----
The engine requires each component type to have a unique numeric ID. Even
individual scripts have their own unique ID, so two different scripts have
different IDs. This is required by the Component instantiation code, which
looks up to the required ID in order to decide which class to instantiate.

User script IDs are determined by the server in the file
``server/project/Project.py``, by the function ``getScriptId()``. Normal
components (standard components) must be manually specified in
``server/components/DefaultComponentManager.py``, in the ``_defaultComponents``
dictionary. The format of this dictionary is:

.. code-block:: python

   '<unique_string_identifier>': {
       'type': '<unique_string_identifier>',
       'id': <unique_numeric_id>,
       'path': '<path to component file.hpp>',
       'full_class_name': 'lau::ComponentClassNameWithNamespace',
       'fields': {
           'field_name': <default_value>,
           ...
       }
   }

If you setup your component on this file (which you'll do whenever creating a
typical component), make sure to edit the
``scripts/directives/game_object_editor.js`` file accordingly, as
:ref:`explained above <add-component-to-menu>`.

.. _implementing-custom-components:

----
Implementing custom components
----

Custom components are typically within the ``lau`` namespace. Although not
obligatory, this is a good practice since it will prevent from cluttering the
global namespace.

Whenever implementing a standard component, make sure to fill the
:ref:`DefaultComponentManager.py file accordingly
<define-unique-component-id>`.

.. _persistent-components:

------
Making the new component persistent
------
In order to make your new component's public data savable and loadable by both
the editor and its own instances, you need to define which fields need to be
saved, and how these fields can be converted into instance-specific usable
information.

This is performed in ``scripts/services/lau_components.js`` file. The following
changes must be implemented:

* Adapt the function ``createComponentFromFlyWeight``. This function creates
  javascript instances of components, which are usually added to a game object
  (either by the editor, or as requested by the previewer).
* Implement the following prototype:

.. code-block:: javascript

  // The parameter "flyweight" is either the value you
  // defined as flyweight in the componentTypes variable (game_object_editor.js)
  // or a serialized data, which may have been loaded from disk or
  // received from the previewer. Either way, it will always have
  // the same format.
  function ComponentPrototype(flyweight) {
    // Initialize internal fields. Do not copy all values from the flyweight;
    // instead, keep a reference to it. Only copy values that vary across
    // instances.
  }
  ComponentPrototype.prototype = {
    export: function(), // Exports a serializable object with data
                        // from this component, which can be saved
                        // and loaded later (in which case, it will
                        // be passed as the "flyweight" parameter to
                        // the constructor)
    setValues: function(flyweight), // Set its internal data from the equivalent 
                                    // fields in the flyweight. Used both for
                                    // initialization and during preview updates.
    destroy: function() // Component destructor. Called when the component is
                        // removed from the game object, or when the gameo bject
                        // itself is destroyed.
   };

* If you need to access the component from somewhere else, then make it public
  by adding this new prototype to the object returned at the end of this file:

.. code-block:: javascript

  return {
    TransformComponent: TransformComponent,
    ...,
    NewComponent: ComponentPrototype
   };

.. _canvas-consistency:
--------
Making the new component Canvas-Consistent
--------
In order to make the new component interact with the edit canvas, follow these
steps:

* In the file ``scripts/directives/edit_canvas.js``, create a trackComponent
  function that initializes its graphical representation and watches for
  changes in the component fields. Consult ``trackPositionalComponent`` for how
  this can be done.
* A few lines below, close to the `@@ Watch for changes in the component list` comment, add all the necessary logic to deal with the following two cases:

  * Your component was just **added** to the game object
  * Your component was just **removed** from the game object

====
Creating component widgets
====
Every component field (Number, Color, String, etc) that can be potentially used
by scripts can be created by following these steps:

* In the ``development/app/`` folder, run the directive creation tool: ``yo
  angular:directive <field_name>``
* Move the created script from ``app/scripts/directives/<field_name>.js`` to
  ``app/scripts/directives/component_widgets/<field_name>.js``. Make sure to
  update the ``app/index.html`` file with the new location of the directive
  file.
* Create an alias for the type that will be handled by the new widget: In the
  server file ``parser/CppParser.py``, add an ``elif`` in the function
  ``translateFieldType(typeDeclaration)`` to convert the USR typename symbol as
  it is returned by clang into something that will be used everywhere else in
  the lib.
* Create a view under ``views/directives/component_editors/script_fields/``
  specifying how the widget will be used by scripts. The following variables
  are available:

  * ``{{fieldName}}`` Name of the field as defined by the users in their scripts.
  * ``component.fields[fieldName]`` Reference to the field data. Bind this to
    the widget input value.

* Add a reference to this view on ``scripts/directives/script_field.js``, under
  ``getTemplateName``, by using the alias name defined previously.

-----
Initialization rules
-----
Component widgets must have their initialization rules defined in the server
file ``server/components/DefaultComponentManager.py``, in the function
``DefaultFieldValue(typename)``. This function receives the unique string
identifier of that field, and returns the default value associated with it.


=======
Canvas Manager
=======

The canvas manager is a service that provides access to the internals of the
edit canvas (WebGL). This is useful when developing standard components that
have a visual representation that must be shown in the edit canvas.

The exposed fields are the following:


.. code-block:: javascript

   isEditMode: function(), // Return true if the frontend is in editmode,
                           // or false if it is in preview mode.
   scene: THREE.Scene, // The ThreeJS scene object singleton.
   getBoundingBox: function(), // Returns a new THREE.Mesh object containing a
                               // wireframe rendered box.
   disableEditMode: function(), // Disables edit mode, entering in preview mode
   enableEditMode: function() // Enables edit mode, exiting preview mode


=======
Component Manager
=======

The component manager holds the list of all user defined assets, such as
scripts and shader programs. It is implemented as a service of name
``componentManager``, and provides the following functions:

.. function:: pushComponent(comp)

   Adds the asset ``comp`` to the list of all assets.

.. function:: getComponent() -> list

   Returns a list of all assets from the project. Each element from the list
   has the format ``{menu_label: "Menu Label", flyweight: {...}}``

.. function:: getComponentMenu() -> list

   Returns the componentMenu array.

.. function:: getFlyweightById(id)

   Given the unique numeric id that identifies every component, returns the
   flyweight of that component, or ``null`` if no component matching that id
   was found.

.. function:: createComponentFromId(gameObject, id, instanceId)

   Given the unique numeric id that identifies every component, returns a new
   component instance for that flyweight.

   :param gameObject: Game object where the component will be inserted.
   :param id: Unique id identifying the component type.
   :param instanceId: Unique numeric id that can be used to identify every single component in the scene. Only used by the NaCl previewer, since only it queries components by their ids during every update iteration.

The internal function ``updateGameObjectsAfterUpdatedFlyweight(scriptFlyweight)`` iterates
over all game objects in the current scene, updating their instances of the
script component specified by ``scripyFlyweight``, when they possess it.
Updating means that fields that are no longer present are removed from the game
object's component, and new fields are added.  Its parameter
``scriptFlyweight`` is the flyweight of the component being updated. TODO
implement this for all scenes! VERY IMPORTANT!

.. _asset-types:

======
Component Flyweights
======

The flyweights of standard components are defined in the
:ref:`DefaultComponentManager.py server file <define-unique-component-id>`.
Non-standard components have different flyweights, as specified further below.

The list of all flyweights can be queried with the function ``getComponents()``
from the ``componentManager`` service.

------
Script
------
The script flyweights contains both their unique numeric ID and implementation
specific data parsed from their C++ files. They are created by the server in
the file ``server/io/IOEventHandler.py``, and are loaded in the editor by
``scripts/directives/project_panel.js`` and inserted in the components
collection managed by ``scripts/services/component_manager.js``.

.. code-block:: javascript

   {
     type: 'script', // This field is literally "script". The type field is 
                     // a standard string field in flyweights used to
                     // identify them.
     fields: {
       "fieldName0": <initialValue>,
       "fieldName1": <initialValue>,
        ...
     },
     types: {
       "fieldName0": "fieldType",
       "fieldName1": "fieldType",
        ...
     },
     pragmas: {
       "fieldName0": ["user", "defined", "pragmas"],
       "fieldName1": [...],
        ...
     },
     visibilities: {
       "fieldName0": visibilityLevel,
       "fieldName1": visibilityLevel,
        ...
     },
     path: "/full/path/to/script/File.hpp",
     namespace: "sample::inner",
     class: "CPPClassName",
     id: uniqueNumericId
   }

The ``visibilityLevel`` can be 0 (public), 1 (protected) or 2 (private). Only
public fields are given to the editor by the server.

======
LAU Utils
======

The LAU Utils namespace concentrate several utility functions used by the
javascript frontend. They are documented below:

.. function:: LAU.Utils.clone(variable) -> var

   Performs a deep clone of the input variable.

   :param variable: Any variable type (numbers, strings, objects, arrays), with the exception of functions and objects that contain functions.
