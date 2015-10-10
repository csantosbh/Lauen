Preprocessor directives
***********************

-----
Compilation directives
-----

==================  ================================================
Variable name       Variable meaning
==================  ================================================
``DEBUG``           Defined when building the executable in debug
                    mode. No optimizations are performed in this mode, and
                    debugging symbols are kept (-g).
``RELEASE``         Defined when building the executable in release
                    mode. In this mode, debugging symbols are not kept, and
                    compilation is performed with level 3 optimization
                    (``-O3``).
==================  ================================================

----
Platform directives
----

==================  ================================================
Variable name       Variable meaning
==================  ================================================
``NACL``            Defined when the target platform is Google's Native Client.
``DESKTOP``         Defined when the executable is compiled for a Desktop
                    (Linux and Windows native binaries).
``LINUX``           Defined when the executable is built for the Linux
                    platform. The preprocessor ``DESKTOP`` is also defined in
                    this case.
``WIN32``           Defined when the executable is compiled for the Windows
                    platform. The preprocessor ``DESKTOP`` is also defined
                    in this case.
==================  ================================================

----------------
Other directives
----------------

==================  ================================================
Variable name       Variable meaning
==================  ================================================
``PREVIEWMODE``     Defined when the executable is built for being run in
                    preview (debug) mode inside the engine editor.
``GL_ES``           Defined when the target platform uses OpenGL ES.
==================  ================================================

