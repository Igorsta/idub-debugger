# There are 4 namespaces in the project
- _N_PARSE : responsible for parsing lines of the code and the entitre files
- _N_PRNT : responsible for printing the instructions (used only for debugging)
- _N_EXEC : responsible for printing the actual execution of the instructions
- _N_ARGS : responsible for handling the arguments of each type of opcode

Each of those namespces has a nested namespace _UTILS, which allows for hiding the details of implementation.
Every type of opcodes introduces a corresponding function in some of those namespaces - most of them (also called as *SIMPLE*) in all of those namespaces.

# Organizing of the code
Code is executed by a functions which hold a set of instructions to execute and a small amount of private static memory, which is used as a registers.
When calling the function, arguments and results are always passed/returned via memory stack. Each function explicitly defines the number of arguments it will take from the stack, and how many values it will put on it.
There is not guarantee about state of the registers after a calling another function.

# Eror handling/verification
There is no static analysis of the bytecode. There are dynamic checks to ensure the proper behavious and inform the user if an error were to occur. It introduces delay, but makes handling errors much simpler.

# Implementation details
Project was compiled with C++23, CMake 3.15 and clang-format 18.1.3. on WSL.
The project was developed with intent to be built by running 
```
./utils/build.sh
```

# Debugger
Code can be run with both __run__ and __debug__ mode. The debug mode was inspired by the GDB, though it is far less rich in terms of functionality.

A quick demo can be made by running
```
./build/bin/my_vm debug ./example/cool/factorial_rec
```

When running debugger CLI, call "help" or "h" to view the set of supported commands 
