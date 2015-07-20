Default Assets
**************

The default_assets folder contain all the internal engine code that is shipped
with every game. It performs window management, content loading, game object
instantiation, scene loading, automatic game state serialization and
deserialization, and much more.

=====
Factories
=====

Factories are :ref:`dynamically generated <cpy-templates>` functions that
deserialize game objects and components. They are implemented in the
``Factories.cpy`` template file, which is parsed into ``Factories.cpp`` by the
engine server.

The ``Factories`` class contains factory methods for Components and GameObjects:

.. cpp:function:: shared_ptr<Component> Factories::componentFactory(const rapidjson::Value& serializedComponent)

    Given a serialized version of the component, creates an instance of the
    corresponding component class and initializes its public fields with the
    serialized data.

    :param serializedComponent: A serialized object with values for all public fields of the component.

.. cpp:function:: vector<shared_ptr<GameObject>> Factories::gameObjectFactory(const rapidjson::Document& objects)

    Given a serialized version of an array containing game objects and their
    components, creates a vector of game objects and instantiates each one of
    them, populating them with their components (which are instantiated with
    Factories::componentFactory).

    :param objects: Serialized array of gameobjects, containing their components as well.

The sequence diagram below indicates where and when game objects are
instantiated under normal conditions:

.. uml::

   Window -> Game: init()
   Game -> Factories: gameObjectFactory()
   Factories -> Factories: componentFactory()
   Factories -> GameObject: addComponent()
   Game <- Factories: gameObjects


====
Peekers
====

Peekers are helper classes created with easy component serialization in mind.
Component Peekers are essentially children of the ``Component`` class, with an
internal pointer to an actual component class.

They are implemented as :ref:`CPY files <cpy-templates>`, so that all script
fields can be retrieved in compilation time. Every Component Peeker implement the
following methods:

.. cpp:function:: ComponentPeeker(shared_ptr<T> actualComponent)

   The only available ComponentPeeker constructor. Takes a shared pointer to
   the actual component as a parameter.

   :param actualComponent: Shared pointer to the actual component being wrapped.

.. cpp:function:: const pp::VarDictionary& getCurrentState()

   Returns the current state of the component wrapped by the peeker (that is,
   the current values of all its public fields).

.. cpp:function:: int getComponentId()

   Returns the :ref:`unique numeric id <define-unique-component-id>` associated
   with the component type. Every component of that particular type will return
   the same id whenever this function is called.

.. cpp:function:: int getInstanceId()

   Returns a unique numeric instance id that's computed sequentially during
   runtime. Every component instance will have a different instance id.


-------
Game Object Peeker
-------
The Game Object peeker is responsible for using these methods in order to
update the Editor state during a preview run. It is implemented as a
specialization of the GameObject class, and is only used in preview mode.

If the game is compiled in preview mode, game object instantiation will occur
as the sequence diagram below illustrates:

.. uml::

   NaClWindow -> Game: init()
   Game -> Factories: gameObjectFactory()
   Factories -> Factories: componentFactory()
   Factories -> GameObjectPeeker: addComponent()
   NaClWindow <- GameObjectPeeker: addComponent()
   NaClWindow -> Editor: broadcasts "component added" event
   Game <- Factories: gameObjects

====
Windows
====

The window manager is responsible for creating windows, listening to resize
events, capturing input data and managing the OpenGL context. There are
currently two types of window classes: Desktop (implemented with `GLFW
<http://www.glfw.org>`_ and compatible with Linux, Windows and Mac) and NaCl
(native client window).

All window management files are located in the ``window`` folder. Internally,
every Window has a pointer to a Game object. In the Desktop window, the game
loop is implemented with `fixed time step
<http://gameprogrammingpatterns.com/game-loop.html>`_ calls to the
``Game::update`` function, and is followed by a call to ``draw(alpha)`` with a
floating point parameter :math:`\alpha \in [0,1)` that can be used to perform
interpolation in the time dimension, avoiding stuttery associated with time
aliasing.

=====
Game class
=====

The Game class is responsible for managing the collection of game objects from
the current scene.

====
Utils
====

The ``utils`` folder contains a collection of utility functions detailed below.

----
IO
----

This module is responsible for performing asynchronous file loading. In order
to use it, you must get the IO singleton with ``IO::getInstance()``, and then
call ``OI::requestFiles()`` with a list of filenames and a callback function:

.. code-block:: c++

   auto& instance = lau::utils::IO::getInstance();
   instance.requestFiles({
     "file1.txt",
     "file2.png",
     "path/to/file3.bin"
   }, [] (std::deque<std::pair<bool, std::vector<uint8_t>>>& input) {
       for(auto& fileStatus: input) {
          if(fileStatus.first) { // .first: true if the file was loaded; false otherwise
              lau << "Buffer size: " << fileStatus.second.size() << endl;
          } else {
              lau << "Could not load this file!" << endl;
          }
       }
   });


The path given to ``getInstance()`` is relative to the folder where the game
executable is located.

.. warning::
   
   When loading text files, make sure to add a ``'\0'`` to the end of the loaded
   buffer if you are going to pass that buffer to some function that assumes
   its existence.

-----
Time
-----

The function ``double lau::utils::time::now()`` returns the number of seconds,
down to microseconds precision, since the Epoch, and is used by the ``Game``
class in the game loop.

====
Default Components
====

----
Transform
----

