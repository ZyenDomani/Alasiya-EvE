### liveupdategen

Tool to generate liveupdates for evemu

### Flags

`-host` The host for your MySQL server "127.0.0.0.1:3306"
`-u` The MySQL username "root"
`-p` The MySQL password ""
`-db` The MySQL database to write to "evemu"
`-dump` Dumps all liveupdate patches in patches/ to the database "false"
`-dev` Generates devtools.raw file from devtools.py "false"
`-debug` Outputs more debug text.  Additional dumps all code objects to codedump/ "false"
`-server` Run forever constantly reading fs events and auto updates liveupdates "false"

### Commands

`list` Lists all enabled and disabled patches
`enable` Copies a patch from patches/disabled to patches
`disable` Copies a patch from patches to patches/disabled

### patches

Patches are python files with a special comment that tells liveupdategen what to patch.

The special comment looks like a python decorator.
`#@liveupdate("globalClassMethod", "uicls.CharacterCreationLayer::CharacterCreationLayer", "AskForPortraitConfirmation")`

The first argument is the codeType.  This tells liveUpdateSvc the patch method.  There are three patch types.

- globalObjectMethod "{object name}" I don't recommend using this
- globalFunction "{object name}.{function name}"
- globalClassMethod "{__guid__}::{class}"

The last argument is the method/function name to patch.
NOTE:  no newline/space between the descriptor line and def to patch

### devtools.raw

devtools.raw can be created from a devtools.py file with the `-dev` flag.

The file is sent to the client if they have ROLEMASK_ELEVATEDPLAYER.

It expects a Bootstrap function that takes two arguments.  The first is a an instance of DevToolsClient and the second is an instance of your own Bootstrap function.

The biggest thing to know is you must set DevToolsClient.Loader to a function.

One feature that we have is the ability to replace a string constant as a hex encoded code object from a file.

`insiderClass = "hexex::insider.py"`  This allows injection of code to get around
the inability of having Bootstrap with "free arguments"(References to things outside of the code object)