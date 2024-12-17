#include "input.h"
bool identifier = 0;
void process_input(const char *input)
{
    if (strcmp(input, "cls", 4) == 0)
    {
        clear_screen();
    }
    else
    {
        print("\n");
        print(input);
        print(" is an unknown command\n");
    }
}
