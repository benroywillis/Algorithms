#include <stdio.h>

/* John: how to break the tools
 * Variation: call the function pointer in a loop, where the function in the pointer is different for each iteration
 *  - different function for each iteration
 *  - different function for invokation of the loop
 * 
 * Variation: function pointer is recursive
 *  - head recursion: I immediately recurse then use the rest of the function to clean up
 *  - tail recursion: I return a call to myself (fibonacci)
 *  - middle recursion: something done before and after recursive calls (DFS)
 * Variation: pass function pointer into function pointer (indirect recursion)
 * Variation: function pointer contains a loop
 * 
 * NewTest: goto's... try messing around with these
 *  - you can put labels in variables
 *
 * Do we care about what happens before main?
 * John: there is an entire culture that do things before main for fun
 *  - people would have contests to see who could write the smallest program (bytes of executable). 
 * We are making a blanket assumption that the intent of the programmer lies within the curly brackets of main
 *  - we miss anything that is procedurally setting up static memory
 *  - if anybody moves a significant amount of computation into global static variables (these should be the only ones that happen before main) we miss it
 * From a complete program reconstruction point of view, it would be great if we can capture that
 *  - if we just jam all this stuff into a header, we are fine
 *  - we may just access base types ourselves.. ie throw away user-defined abstractions that encapsulate user-defined types
 *  - case: my datastructure is making arbitrary traversal decisions... so we would have to extract this code and run it ourselves
 */

int print1(char* stuff)
{
	printf("Print1 has been handed %s stuff\n", stuff);
	return 0;
}

int print2(char* stuff)
{
	printf("Printf2 has been handed %s stuff\n", stuff);
	return 0;
}

int helper(char* name, int (*op)(char*))
{
	printf("Running helper with %s!\n", name);
	return op(name);
}

int main()
{
	helper("Crappy", print1);
	helper("Good", print1);
	helper("Crappy", print2);
	helper("Good", print2);
	return 0;
}
