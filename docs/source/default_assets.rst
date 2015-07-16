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
====
Peekers
====


====
Windows
====


====
Utils
====


====
Default Components
====

----
Transform
----

