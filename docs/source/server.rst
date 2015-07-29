Server
******

The server is a module created to perform editor-related tasks that would
otherwise be too complex to perform if it was written in JavaScript, such as
C++ file parsing, filesystem monitoring and shell commands (e.g. for generating
builds). It is written in Python, which have many libraries that simplify these
tasks and many others.

============
Folder structure
============
Any source code related to the server is located inside
``$REPO_ROOT$/src/server``. It is subdivided into the following folders:

* ``build`` Contains our custom :ref:`build system <build-system>`, responsible
  for compiling the project game into all supported targets.
* ``components`` Contains the flyweights of standard components.
* ``io`` The IOEventHandler submodule sets up some IO related RPCs, and the
  Utils submodule has several utility functions for managing files and folders.
* ``parser`` C++ parsing related utilities.
* ``project`` Holds the current project state (component ids, scenes, etc), and sets up some project related :ref:`RPCs <server-rpc>`.

.. _build-system:

=====
Build system
=====
The build system is responsible for generating game binaries. It was created to
automate dependency detection and perform fast, incremental building of the
games. It is used to generate both the final project binaries and the
previewable binaries.

The file ``build/BuildEventHandler.py`` exposes the following functions:

.. function:: BuildProject(platform = 'linux', runGame = True, compilationMode='DEBUG', outputFolder = None)

    Compiles all assets and standard assets and generates a game executable. It
    creates a temporary ``/build/`` folder where object files will be
    generated.

    :param platform: Target platform. Currently supported values are ``linux``, ``windows``, ``nacl`` and ``preview`` (same as nacl, but with preview specific code enabled).
    :param runGame: Runs the game after it is built. Only works with the ``linux`` target.
    :param compilationMode: ``DEBUG`` or ``RELEASE``. Affects compilation flags and linked libraries.
    :param outputFolder: Complete path where the binaries will be generated.


.. function:: ExportGame(platform, buildAndRun, compilationMode, outputFolder, cleanObjects=True)

    Builds and packages the final game, cleaning up temporary files if
    ``cleanObjects`` is ``True`` and copying assets to the output folder.

    :param platform: Target platform. Currently supported values are ``linux``, ``windows``, ``nacl`` and ``preview`` (same as nacl, but with preview specific code enabled).
    :param buildAndRun: Runs the game after it is built. Only works with the ``linux`` target.
    :param compilationMode:  ``DEBUG`` or ``RELEASE``. Affects compilation flags and linked libraries.
    :param outputFolder: Complete path to output folder.
    :param cleanObjects: If set to ``True``, the temporary object files generated in the build process will be removed after the compilation is done.

.. function:: BuildPreviewObject(inputFile) -> dict

   Compiles the input file, producing an object file for the preview target (Native Client). Returns the following dict:

   .. code-block:: javascript

        {
        output: string, // Path of the output object file
        message: string, // Compiler messages
        success: bool
        }

   :param inputFile: Path to input file. Make sure it is not a header file.

.. _cpy-templates:
-----
CPY templates
-----

CPY files are `Mako templates <http://www.makotemplates.org/>`_ of C++ files.
Theywere created to overcome the lack of compile time reflection in C++. They
are rendered by the function ``_renderTemplateSources``, which is called by the
``build/BuildEventHandler.py`` module. The currently implemented CPY files are
the following:

* **Factories.cpy** Contains factories for game objects and components.
* **Peekers.cpy** Wrapper classes that handle serialization of components and game objects.

When parsed, CPY templates have access to the following variables:

.. code-block:: javascript

   {
     components: [
     // collection of user asset flyweights
     ],
     default_components: [
     // collection of standard component flyweights
     ],
     isVecType: function(typename), // Function that returns True if typename is a vector,
                                    // and False otherwise. Useful for checking the need
                                    // for iterating through vector components.
     vecIterations: { // Maps to the number of elements in each type of vector.
       v4f: 4, v3f: 3, v2f: 2
     }
   }

-----
Adding support to new targets
-----

In order to make games compilable to new platforms, follow these steps:

* In the directory ``LauEngine/third_party/cross_compiling``, create a new
  folder with the target name and, inside it, put the object files for all the
  dependency libraries (check out :ref:`dependencies <dependencies>` for a list
  of standard, dynamically linked dependencies).
* Add the new target to the ``build/BuildEventHandler.py`` module:

  * **Create a preprocessor directive** Create a preprocessor directive by
    adding a new key to the dictionary ``platform_preprocessors``.
  * **Setup the required compiler** Cross compilation oftenly requires
    different compilers. Whether the new platform requires a special compiler
    or not, you must add an entry to the ``cxx_compiler`` dictionary specifying
    the required compiler (if it is a non-standard compiler, create a
    :ref:`Config <configpy>` entry to allow users to specify the location for
    their local installation of the required compiler).
  * **Compilation and link flags** Edit the ``_getFlags`` function and add both
    compilation and link flags for the new platform.
  * `(optional)` **Setup the post compilation actions** If the new target
    requires any post compilation steps, like copying dependency libs to the
    build folder, these steps must be specified in the ``_PostExportStep``
    function.
* **Make the new target available on the editor** In the editor, open up the
  view ``views/dialogs/build.html`` and add the new target to the export
  menu. Make sure to use the same alias you used previously to define the new
  target.

======
C++ Parser
======
The ``parser/CppParser.py`` module is responsible for parsing C++ classes and
extracting their public fields, including their preceding #pragmas.

An important function in this file is the ``translateFieldType(typeName)``.
This function converts a clang USR typename symbol into an internal identifier
that is used across the whole engine. Every class type that's supported by the
editor should be translated in this function, since that makes the typenames
more clear and reduces the cost of adapting the engine in case clang changes
its USR symbols.

Another important function is ``GetSimpleClass(filePath)``, which parses the
given file and, assuming that it defines one class, returns a dictionary
containing the following information about that class:

* Class name
* Full Namespace
* Fields, with their names, types, visibilities and preceeding pragmas
* Dependencies (a list of all files directly or indirectly included by that
  file)


The ``src/test_parser.py`` script can be used as a command line tool to parse
individual C++ files, which is useful for displaying the USR symbols of
unsupported types: ``./test_parser.py <path/to/cppFile>``.


======
HTTP and WebSocket servers
======
The HTTP server, implemented in ``HTTPServer.py`` provides the editor with both
internal engine data (like HTML and JavaScript files) and project data (like
user assets and NaCl files). The compiled NaCl files used by the preview mode
are saved in the folder ``$PROJ_ROOT$/build/nacl/``.

The websocket server, implemented in ``WSServer.py``, handles asynchronous
requests made by the editor, which are treated as events. Every websocket
packet sent to the server must be a JSON in the following format:

.. code-block:: javascript

   {
     event: "eventName",
     msg: <eventData>
   }

====
IO Utilities
====
The ``io/Utils.py`` module contains filesystem related utility functions.

.. function:: GetFileNameFromPath(path) -> str

   Given the path to a file ``path``, returns the name of the file.

    .. code-block:: javascript

        >>> Utils.GetFileNameFromPath('/var/tmp/file.cpp')
        'file.cpp'

   :param path: Complete path to a file.

.. function:: PathHasExtension(path, extensions) -> bool

   Returns ``True`` if path has one of the extensions from the array
   ``extensions``.

   :param path: Path to a file or folder.
   :param extensions: Array of extensions to be checked for.

.. function:: IsHeaderFile(assetPath) -> bool
   
   Returns true if the path ``assetPath`` refers to a C++ header file (.h,
   .hpp), and false otherwise.

   :param assetPath: Path to the asset file.

.. function:: IsImplementationFile(assetPath) -> bool
   
   Returns true if the path ``assetPath`` refers to a C++ source file (.cpp,
   .cxx), and false otherwise.

   :param assetPath: Path to the asset file.

.. function:: IsScriptFile(assetPath) -> bool

   Returns ``True`` if ``assetPath`` refers to a script asset file (which is
   determined by its extension  -- .hpp, .h, .cpp and .cxx), and ``False`` otherwise.

   :param path: Path to the asset file.

.. function:: IsTrackableAsset(assetPath) -> bool

   Returns ``True`` if ``assetPath`` refers to a trackable asset file, which is
   determined by its extension. A trackable asset file is any type of file that
   should be displayed in the editor project panel.

   :param path: Path to the asset file.

.. function:: OpenRec(path, mode) -> file_handle

   Opens the file given by ``path``, returning the file handle. If the path
   specifies a directory that doesn't exist, this function will create it
   before opening the file.

   :param path: Path to the requested file.
   :param mode: File open mode. Same as the mode passed to python's default ``open()``.

.. function:: IsSubdir(path, directory) -> bool

   Returns ``True`` if the path ``path`` lies within the path given by
   ``directory``. For instance, ``/var/tmp/sample/file.cpp`` lies within
   ``/var/`` and ``/var/tmp/``, but not ``/home`` or
   ``/var/tmp/sample/folder/``.

   :param path: String representing the queried path (can be a reference to a file or a folder).
   :param directory: String representing the reference directory.

.. function:: ListFilesFromFolder(path, extensions = None) -> list

   Lists all files from a folder (and all its subfolders) whose extensions are
   one of the extensions in the array ``extensions``. 

   :param path: Complete path to the root folder.
   :param extensions: Array of query extensions. If set to ``None``, all files in these folders will be returned.

.. function:: CopyFilesOfTypes(src, dst, types, baseSrcFolder=None)

  Copies all files of specified types to destination folder. The destination
  folder will be created, so as to keep the directory structure of each original
  file.

  :param src: Complete path to source folder.
  :param dst: Complete path to destination folder.
  :param types: Array specifying the types of files to be copied.
  :param baseSrcFolder: Complete path to the folder to be considered root of the hierarchy being copied. For instance, if ``src`` is ``/var/tmp/project/assets``, ``dst`` is ``/home/user/destination`` and ``baseSrcFolder`` is ``/var/tmp``, then a folder ``project/assets`` will be created inside ``/home/user/destination``, and all files of the requested types will be copied there.

=====
Project module
=====
The module ``project/Projecy.py`` contains a singleton of the class
``_Project``, which holds the following metadata about the project being
edited:

=================  ===================================================
``scripts``         Dictionary that maps a full path to a user script
                    into a unique numeric id.
``scenes``          List of all scenes associated with the current
                    project. Contains the path to the scene .json file,
                    relative to the ``$PROJ_ROOT$`` folder.
``currentScene``    Numeric index of the scene currently being edited
                    by the engine user.
=================  ===================================================

This module exposes the following functions:

.. function:: getAssetList() -> list

   Returns a list of dictionaries, each of them containing detailed information
   about all assets currently available inside the ``assets/`` folder of the
   project.

.. function:: getProjectFolder() -> str

   Returns the complete ``$PROJ_ROOT$`` path.

.. function:: createNewProject(path)

   Given the full path to a project file, this function creates a new project
   in that ``path``, copying the :ref:`template project <template-project>` to
   this folder. If the path contains a reference to an already existing project
   file, it will be loaded instead (and will not be overwritten).

   :params path: Full path to a target (preferrably non-existing) .json project file.

.. function:: saveCurrentScene(sceneData)

   Given a dictionary with :ref:`scene data <scene-format>`, saves this scene
   in the current scene file (which is specified by the project
   ``currentScene`` field).

   :param sceneData: :ref:`Click here for more information about the scene data format. <scene-format>`

.. function:: getScriptId(scriptPath) -> int

   Given the full path to a script asset (which must be inside the ``assets``
   project folder), returns the unique numeric identifier for the class
   contained in that script. If the script was not previously detected by the
   project, a new id will be created for the given path.

   :param scriptPath: Complete path to the script file. Must be inside the
   ``assets`` folder.

.. function:: loadCurrentScene() -> scene_data

   Returns a :ref:`scene data object <scene-format>`. The scene data will be
   the one identified by ``currentScene`` in the project singleton.

.. function:: loadProject(path) -> bool

   Given a path to a project.json file, loads it by updating the internal
   project singleton. Currently, always returns True.

   :param path: Complete path to a project.json file.

.. function:: processAsset(path, saveProject) -> <assetFlyweight>

   Loads the asset from disk, parses it and extracts its metadata (like
   modification time for any regular asset and public fields for script assets)
   and caches that metadata.

   For user scripts, this function also determines the script id when it is
   first detected.

   :param path: Complete path to the asset file.
   :param saveProject: If ``True``, the project will be saved after the asset is processed.

.. function:: isFileOlderThanDependency(filePath, assetPath) -> bool

   Given an asset path located in ``assetPath`` and any file path ``filePath``,
   this function returns True if ``filePath`` is older than any of the asset's
   dependencies. This is useful for detecting when an object file must be updated, e.g:

   .. code-block:: python

       if Project.isFileOlderThanDependency('/tmp/build/myfile.o',
                  Project.getProjectFolder()+'/assets/myfile.cpp'):
           print 'Object file is outdated!'

   This function also returns ``True`` in the following cases:

   * If ``filePath`` refers to a file that doesn't exist;
   * If ``assetPath`` itself was modified after ``filePath``.

.. function:: isCPYTemplateOutdated(cpyFilePath) -> bool

    Given a path to a CPY template, returns ``True`` if the C++ file produced
    from it is outdated, which is determined by the following rules:

    * If the C++ file doesn't exist; or
    * If the CPY file was modified after the C++ file was generated; or
    * If any of the C++ dependencies were modified after the C++ file was generated.

---------
Asset Folder Watcher
---------
This submodule is responsible for watching file changes in the project folder
$PROJ_ROOT$. Whenever a new file is created, updated or deleted, it broadcasts
an :ref:`AssetWatch socket event <server-events>`.

It exposes the function ``stopWatcher()``, which must be called prior to
shutting down the server, as it will stop the folder watcher thread.

.. _scene-format:
=====
Scenes
=====
The scene files contains a list of game objects in the serialized format, as
can be seen below:

.. code-block:: javascript

    // List of game objects
    [
      {
        "name": "gameObjectName",
        "components": [
          {
            "fields": {
              "fieldName": <fieldValue>,
              // ... other fields ...
            },
            "type": "componentStringIdentifier",
            "id": <uniqueNumericComponentId>,
            // The fields below are only present in script components
            "path": "/path/to/component/asset.hpp",
            "namespace": "full::namespace",
          },
          // ... other components ...
        ]
      }
    ]

.. _configpy:

=====
Project settings and Runtime settings
=====
The module ``Config.py`` manages two types of configuration variables:

* **User-editable configuration**, which is saved in json format in the file
  ``~/.laurc``. Such variables can be retrieved with the function
  ``Config.get(section, field)``, where ``section`` specifies the section where
  the configuration is stored, and ``field`` is the configuration field name.
  They can also be set with the function ``Config.set(section, field, value)``.
  When the server sets a variable, the settings file is automatically updated.
* **Runtime configuration**, like the engine installation folder. These
  variables can be retrieved by the ``Config.env(section)`` function, where
  ``section`` is the name of the variable to be fetched.


.. _runtime-config-fields:
-----
Runtime Configuration Fields
-----

.. code-block:: javascript

    {
      // Absolute path to the engine installation folder
      "install_location": "/path/to/LauEngine/",
      // List of asset types that must be copied to the final game folder
      // whenever a game is exported. The defaults are highlighted below.
      "exportable_asset_extensions": ['.vs', '.fs']
    }

.. _server-events:

=============
WebSocket Events
=============
The server listens to the following events (which may be broadcast by the
editor):

=================  ===================================================
RPCCall             A wrapper event that is translated into RPC calls.
                    Do not broadcast this event manually; it is used
                    by the RPC module only.
=================  ===================================================

The server broadcasts the following events to the editor:

=================  ===================================================
executionMessage    Contains the output from the executed game when it
                    is previewed in a separate window.
compilationStatus   Contains the result from a build attempt (both when the
                    user explicitly requests the game to be built, and when
                    the server automatically builds object files), including
                    warnings and errors.
AssetWatch          Broadcast everytime an asset file is created/updated
                    (``event`` = ``update``) or deleted (``event`` = ``delete``).
                    When ``event`` = ``update``, it has the format
                    ``{event="update", asset=<assetFlyweight>}``. When
                    ``event`` = ``delete``, it has the format
                    ``{event="delete", path="/path/to/deleted/asset"}``.
=================  ===================================================

.. _server-rpc:

====
RPCs
====
RPCs are Socket Events with an event of name ``RPCCall``. The server provides
the following RPC calls:

-----
Defined in `build/BuildEventHandler.py`
-----
.. function:: buildGame()

  Builds the game in debug mode, and launches the built game if compilation is
  successful.

.. function:: previewGame() -> bool

   Exports the game in `preview` mode, by compiling the NaCl executables and
   copying the required assets to the ``$PROJ_ROOT$/build/nacl`` folder. TODO make return type return false if it fails

-----
Defined in `components/DefaultComponentManager.py`
-----
.. function:: getDefaultComponents() -> dict

  Returns the ``_defaultComponents`` dictionary, which contains information
  about standard components.

-----
Defined in `io/IOEventHandler.py`
-----
.. function:: save(sceneData) -> bool

   Saves the scene specified in ``sceneData``. Returns True in case of success,
   and False otherwise.

   :param sceneData: :ref:`Click here for more information about the scene data format. <scene-format>`

.. function:: loadCurrentScene() -> sceneData

   Returns the current :ref:`scene <scene-format>` if the project was loaded
   correctly, and null otherwise.

.. function:: getAssetList() -> array

   Returns a list of assets in the current project, or null in case of a
   failure (e.g. there's no project currently loaded).

-----
Defined in `project/ProjectEventHandler.py`
-----

.. function:: createNewProject() -> string

   Creates a new, empty project. The project destination will be asked to the
   user via a file dialog, and will be returned to the caller.

.. function:: getRecentProjects() -> array

   Returns the ``recent_projects`` :ref:`runtime configuration
   <runtime-config-fields>` to the caller.

.. function:: loadProject(projectPath) -> bool

   Loads a project given the full path to its project.json file,
   ``projectPath``. Returns ``True`` in case of success, and ``False``
   otherwise.

.. function:: exportGame(params) -> bool

   Exports the game to any of the supported platforms, copying all required
   assets to the export folder. The destination folder will be asked to the
   user via a folder picker dialog.

   :param params: A dictionary in the format ``{"platform" : "platformName", "buildAndRun": bool, "compilationMode": "mode"}``. The possible values for ``platformName`` and ``compilationMode`` are explained in :ref:`Build system <build-system>`.

.. _template-project:

=====
Template Project
=====

The template project is the default set of files that correspond to an empty
project. It is located inside the folder ``$REPO_ROOT$/src/template_project``.

It contains a ``project.json`` file with a single, empty scene (called
scene0.json).
