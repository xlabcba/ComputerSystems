# mini-DMTCP

## Introdunction

This project aims at building a mini-DMTCP to achieve checkpoint-restart of an arbitrary application. The checkpoint-restart function means that any running process can be save as checkpoint image file on disk and can be restarted from disk by using the saved checkpoint image file. The checkpoint image can uniquely identify a process by keeping track of its factors, e.g. process identifier, process state, program counter and context data. 

## Run Instruction

1. Open the terminal
2. go to the directory containing the program
3. type command “make”
4. get result