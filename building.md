# Building #

We currently provide no build system for the released maya plugins since the Make system we currently use at Anima relies too much on the way our file system is laid out.

We are looking into providing something a bit more flexible (and maybe more modern too) for this project.

However the plugins should be very easy to build using the instructions in Maya docs or by customising whatever system you already use. The plugins are structured in a very standard fashion. The name of the plugin .so or .mll should be derived from the folder name.