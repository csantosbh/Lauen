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

The relevant source code for the editor module lies within the ``development/app`` folder.

====
Layout
====

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

