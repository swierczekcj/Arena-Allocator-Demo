# Arena-Allocator-Demo
Personal arena allocator demo for use in other projects

Includes a toy calculator that supports *, /, +, -, and parenthesis with integers only (any division will be truncated). Whitespace is not supported. Any operation applied to an expression inside a parenthesis must be explicitly defined:

:white_check_mark: 2*(1+5)

🚫: 2(1+5)


## Running the Demo
Included in this repo are shell scripts for building and running the demo program on linux systems. If executing on WSL, use the 'dos2unix' command first on the .sh files first to convert the windows line endings into unix line endings (git on windows automatically converts unix file endings when checking out files). 
