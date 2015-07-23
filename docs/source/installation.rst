Lau Engine Installation
***********************

To install LauEngine, follow these steps:

#. Get and install all the :ref:`dependencies <installing-dependencies>`
#. Get the latest stable build from github:

   .. code-block:: bash

      $ git clone https://github.com/csantosbh/Lauen.git

#. Build and install the engine:

   .. code-block:: bash

      $ cd Lauen
      $ mkdir build
      $ cmake ..
      $ make -j4
      $ sudo make install

#. :ref:`Configure <user-config-fields>` your local instalation

.. _installing-dependencies:

===========
Installing the engine dependencies
===========
TODO

.. _user-config-fields:

==========
Configuration
==========

All configuration is saved in the file ``~/.laurc``. It is a JSON file with the
following fields:

.. code-block:: javascript

    {
      // Project section
      "project": {
        // Number of projects to be displayed in the "recent projects"
        // menu (Project -> recent projects)
        "recent_project_history": <number>
      },
      // Runtime section
      "runtime": {
        // Recently opened projects
        "recent_projects": [
        "/path/to/project.json"
        ]
      },
      // Export section
      "export": {
        // Compilers for windows cross compilation
        "win_compilers": {
          "g++": "/path/to/x86_64-w64-mingw32-g++.exe"
        },
        // Native Client (NaCl) specific configuration
        "nacl": {
          // Absolute path to the pepper API folder
          "pepper_folder": "/path/to/nacl_sdk/pepper_41",
          // Relative path, from the pepper folder, to the NaCl C++ compiler
          "compiler": "relative/path/to/toolchain/linux_pnacl/bin/pnacl-clang++"
        },
        // Path to third_party folder, where the engine keeps dependencies
        "third_party_folder": "/path/to/LauEngine/third_party"
      },
      // Server section
      "server": {
        // Port where the server will run the HTTP server
        "http_port": "9002",
        // Port where the server will run the websocket server
        "ws_port": "9001"
      }
    }

==================
Third Party libraries
==================
The engine requires a ``third_party`` folder containing all of its external
dependencies, like Eigen and GLFW. The location of this folder is specified in the engine configuration file.


--------
Speeding up parsing and compilation
--------

Since the engine uses and exposes the Eigen library, the time required for
parsing header files can be several seconds long, which is unreasonable. To
speed it up, the engine relies on pre-compiled header files for heavy headers.

In order to generate the pre-compiled header files used by clang and GCC, go to
``$INSTALL_ROOT$/third_party/Eigen`` and run:


.. code-block:: bash

   # Generate clang pre-compiled headers
   $ clang++ -x c++-header -std=c++11 Eigen -o Eigen.pch
   # Generate GCC pre-compiled headers
   $ g++ -x c++-header -std=c++11 Eigen -o Eigen.gch

The engine will take care of the rest.
