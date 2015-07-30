# LunarProbe Debugger Client #

A simple client is provided with the debugger.  It is in the "client" folder of the installation.  It can be run with the command:

**python client.py run**

Note that the program running your lua scripts must already running before the debugger can be invoked otherwise the client will not start.

The commands that can be used in the client are shown below.  It is attached to a test debug server that opens a few lua stacks.  Here is more info on the [Test Debug Server](LunarProbeTestDebugServer.md).

## Debugger Client Commands ##

Firstly, when the client is run as mentioned in the previous step, the debugger prompt is presented.  It is something like:

**LDB:**

This is a very simple mockup and once I get a feel for all the different IDEs/Editors we use, I/We could come up with something like eclipse/netbeans plugins and so on.

At the prompt, typing **"help"** will give a list of commands and their usage.  Commands with "empty" help text are not  yet implemented.

The main commands you need to know are:
  * <p /> **Breakpoint Commands**: break, clear, clearall
  * <p /> **Program Flow Commands**: next, step, continue, finish,  until
  * <p /> **Data Related Commands**: contexts, locals, local, frame, eval
  * <p /> **Miscellaneous Commands**: file, help

### Data Related Commands ###

#### contexts ####
The lua stack is also known as the context.  Each stack that is attached to becomes a context that is visible to the debugger client.  To get a list of contexts currently opened, use the contexts command without any parameters.  An example of the output is:

```

Contexts:
=========
    Name: 'a', Addr: 0x97d3198, Running: False
                At: {u'what': u'Lua', u'name': u'abctotable', u'currentline': 29, u'linedefined': 28, u'namewhat': u'global', u'lastlinedefined': 51, u'nups': 0, u'source': u'@test.lua', u'event': 0}
    Name: 'c', Addr: 0x97f4320, Running: True
    Name: 'b', Addr: 0x97e31c8, Running: True
    Name: 'debugger', Addr: 0x97ef118, Running: True

```

In the above example, we are currently debugging 4 contexts.  The first three were created by the [Test Debug Server](LunarProbeTestDebugServer.md) and the fourth is actually the debugger stack opened by the lua debugger.  Oh did I mention that a big chunk of the lua debugger, which is written in Lua, can also be debugged in the debugger!!!  Though dont go to crazy with this feature as it is hardly tested!

The huge bit of fluff ending with "At: ..." shows where the context is paused.  Only the first one is paused as it has hit a breakpoint (at abctotable).

Keep an eye on the "Addr" parameter.  This is very very important as nearly all run/flow related commands will reference this in one way or another.

#### locals ####

This commands prints out all the local variables in a specific context and stack frame.  The command is run as:

**`locals <context> <frame>`**

Where context is the "Addr" parameter found in the contexts command above, and frame (if not specified) defaults to 0.

So with the above example (locals 0x97d3198), the following output is printed:


```

Locals:
=======
    Name: '{u'index': 1, u'name': u'a'}'
    Name: '{u'index': 2, u'name': u'b'}'
    Name: '{u'index': 3, u'name': u'c'}'

```

Again note the "index" parameter above.  This index must be passed when getting information about a particular variable instead of the name.

Another thing is the variables in lua are defined the first time they are encountered.  So even though the function "abctotable" has quite a few local variables, only a, b and c are defined when the function is first entered.  As we step through the lines more variables will come into play.

So in the same function, after stepping through about 8 lines, we get this:

```

Locals:
=======
    Name: '{u'index': 1, u'name': u'a'}'
    Name: '{u'index': 2, u'name': u'b'}'
    Name: '{u'index': 3, u'name': u'c'}'
    Name: '{u'index': 4, u'name': u'tab'}'
    Name: '{u'index': 5, u'name': u'output'}'
    Name: '{u'index': 6, u'name': u'd'}'
    Name: '{u'index': 7, u'name': u'e'}'

```

#### local ####

This commands prints out the value of a particular local variable.  The usage is:

<div> <b><code>local &lt;context&gt; &lt;index&gt; &lt;nlevels&gt; &lt;frame&gt;</code></b></div>

where the parameters are:
<div> <b>context</b> - the address of the context whose local variables are to be printed - see the "contexts" command.</div>
<div> <b>index</b> - the index of the local variable to be printed (see the locals command for a list of locals) - defaults to 0</div>
<div> <b>nlevels</b> - how many levels to recurse this variable - ie if the variable is a table, we can recursively print the value to this many levels - defaults to 1.</div>
<div> <b>frame</b> - the frame in which the local variable is located - defaults to 0 </div>

For the command **local 0x97d3198 1**, the following is the output:

```

Name: a, Type:  number
Value:  1

```

For a more complex variable like a table (eg variable 4), we have the following:

```

Name: tab, Type:  table
    [a (string)]     ->      1 : (number)
    [d (string)]     ->      0x9821a18 : (table)
    [c (string)]     ->      0x9816b08 : (table)
    [b (string)]     ->      2 : (number)

```

Note that the keys "c" and "d" have values that are themselves tables.  To drill into these, print the above variable as:

<div> <b>local 0x97d3198 4 2</b> </div>

which prints the following output:

```

Name: tab, Type:  table
    [a (string)]     ->      1 : (number)
    [d (string)]     -> 
        [a (string)]     ->      1 : (number)
        [b (string)]     ->      2 : (number)
    [c (string)]     -> 
        [1 (number)]     ->      1 : (number)
        [2 (number)]     ->      2 : (number)
    [b (string)]     ->      2 : (number)

```

Note: the actual lua table value is:
```
{["a"] = 1, ["b"] = 2, ["c"] = {1, 2}, ["d"] = {["a"] = 1, ["b"] = 2}}
```

#### eval ####

This command evaluates an expression when a particular context paused.  The usage is:

<div> <b><code>eval &lt;context&gt;   &lt;quoted_expression&gt;</code></b> </div>

where

**context**:    Is the context on which the expression is to be evaluated.
**quoted\_expression**: Is the expression to be evaluated (in quotes).

Note that this expression runs at a global level and NOT in the same stack frame where the context has paused (by hitting a breakpoint).

I am still working on making this work at the stack level.  Any ideas?


### Breakpoint Commands ###

The following commands set and clear breakpoints in the source lua scripts.

#### break ####

Sets a break point at a given location.

Usages are:

  * **`break <filename> <linenumber>`**

  * **`break <function>`**

When this is done, the next time lua hits the particular line or function, the context is paused.  Also multiple breakpoints at the same location (identified by file/line or function) will only be added once.

Also note that with the function version of the above command, LP currently does not differentiate between global functions and member functions.  Again appreciate help given my beginner lua-fu!

#### breaks ####

Prints out a list of breakpoints.  Example output is:

```

Breakpoints: 
=============
     1 - Function: abctotable
     2 - File: @test.lua, Line: 38

```

#### clear ####

Clears a break point at a given location that was added with the previous command.

Usages are:

  * **`clear <filename> <linenumber>`**

  * **`clear <function>`**


Also note that with the function version of the above command, LP currently does not differentiate between global functions and member functions.


#### clearall ####

Simple.  Clears all break points.


### Program Flow Commands ###

A bit of bad news up front on flow related commands.  Most of these do work.  However, each of them require a "context" parameter which is essentially the "addr" variable you will get from the listing (in the contexts command).  This is a pain and I am working towards making the client smart in a way that it "knows" which context it has currently hit.  This is not hard from the server side as each time a breakpoint is hit, it notifies the client about this.  I am more stuck on how to enable this from a UI point of view.

Again really sorry for this convenience.  Appreciate your patience.  Alternatively, code is free so you can help me modify it by fixing this or even better writing alternative UI (plugins or standalone) for this.

#### step ####

Steps the debugger into the next instruction.

Usage:

**`step <context>`**

**context** is the address of the context as in the "addr" parameter from the context listing.

#### next ####

Steps the debugger over the current line onto the next instruction.

Usage:

**`next <context>`**

**context** is the address of the context as in the "addr" parameter from the context listing.

#### continue ####

Continues execution till the next breakpoint is hit.

Usage:

**`continue <context>`**

**context** is the address of the context as in the "addr" parameter from the context listing.

#### finish ####

Not yet implemented.

Continues execution till the current function has exited or till the next breakpoint is hit.

Usage:

**`finish <context>`**

**context** is the address of the context as in the "addr" parameter from the context listing.

#### until ####

Not yet implemented.

Continues execution till a specific line or function has been hit or till the next breakpoint is hit.

Usages:

**`until <context> <funcname>`**

**`until <context> <filename> <linenum>`**

**context** is the address of the context as in the "addr" parameter from the context listing.

### Miscellaneous Commands ###

#### file ####

This commands prints out the contents of a file between given ranges.

The syntax is:

<div> <b><code>file &lt;filename&gt; &lt;firstline&gt; &lt;lastline&gt;</code></b> </div>

**firstline** and **lastline** are optional and default to 0 and -1 (indicating the "last" line of the file).

For example, file **test.lua 5 10** would have the following output:

```

     5 > function test1()
     6 >     for i = 1, 10 do
     7 >         print("In Test1: " .. tostring(i))
     8 >     end
     9 > end
    10 > 

```