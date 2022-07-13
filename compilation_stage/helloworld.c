

/* Pre-processor Step:

These preprocessors directives are used to add  Header files to the program.
The main thing is pre-processor Directives are used to include files to our programs.
Commonly used Preprocessor Directives are #include and #define.
#define is used to Define Symbolic Constants and Macros.

Removing Comments 
Macro Substitution 
File Inclusion (Including Header files) 
Conditional Compilation

$ gcc -E prog.c -o prog.i

-------------------------------------------------

Translator:

Translator translates the High-level language into Assembly level language. 
After this stage, we will get Assembly level code something like ADD, SUM, SUB, SJMP,..etc mnemonics.

$ gcc -S prog.i -o prog.s

------------------------------------------------------
Assembler:
Assembler is also on the type of translator, 
but it is used to translate assembly level language into Machine language 
(machine language is computer understandable language contains zeros and ones only ). 
After this stage, we will get pure binary code. This code also called as object code.

$ gcc -c prog.S -o prog.o

------------------------------------------------


Linker:
Linker used to link the called function with calling function. 
Normal C program contains many library functions (predefined functions) and user-defined functions. 
Library functions present in library for example we are using printf daily, 
but we are not defined printf  we are just calling printf and printing data, f
unctions like printf are needed in our program so those printf predefined code is present 
in library that code is added to our object code in this stage. 
So linker links called function with calling function.
After this stage, our Executable is ready and stored in your computer secondary storage space.

$ gcc helloworld.o -o helloworld

Loader:

Loader brings executable file into the primary memory from the secondary memory
Any application needs to bring into primary memory in order execute, 
but our all executable files are stored in secondary memory loader is used to bring that files from secondary memory to primary memory. 
Typically  Hard disk to RAM
If you’re using Unix or Linux the normal gcc prog.c is equal to first five stages 
and we will get a.out by compiling then by running that a.out we will get our output.
Now we will discuss one example with each and every stage output files..

$ ./helloworld

*/



#include<stdio.h>
int main()
{
    printf("Hello world");
}