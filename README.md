# alexShell

Linux C shell done for a class. It's a bit rough around the edges but was pretty satisfied with the outcome.

&nbsp;

**Interactive Mode**

Standard shell, program waits for user input then parses the line to send it for sorting to determine what function needs to be executed.

&nbsp;

**Script Mode**

Alternatively, the program can be run by passing the name of a text file in the working directory. The shell will automatically read line by line and execute the instructions.

&nbsp;

**Some built in commands**

Can store environment variables that utilize a linked list for dynamic allocation. This can be done with:

```$variable=value``` e.g. $var1=33

```vprints``` prints all these environment variables.

```log``` Has a log command that prints out all the commands previously used, as well as their time and the return value of the function.

```theme <colour>``` Changes the colour of the theme (default is green).

&nbsp;

**Non-built in commands**

Additionally, the shell forks a child process to execute programs in ```/bin/``` if the command is not initially recognized.

&nbsp;

![C Shell Demo](cshelldemo.gif)
