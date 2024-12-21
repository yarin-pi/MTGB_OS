#include "input.h"
bool identifier = 1;
void process_input(const char *input)
{
    if (strcmp(input, "cls", 4) == 0)
    {
        clear_screen();
    }
    else if (strcmp(input, "echo", 3) == 0)
    {
        char output[100];
        int i = 0;
        int j = 0;
        while (input[i] != ' ' && input[i] != '\0')
        {
            i++;
        }

        // Skip the space after "echo"
        if (input[i] == ' ')
        {
            i++;
        }

        // Copy the rest of the string (hello world) into output
        while (input[i] != '\0')
        {
            output[j++] = input[i++];
        }
        output[j] = '\0'; // Null-terminate the output string

        print("\n");
        print(output);
        print("\n");
    }
    else if (strcmp(input, "exit", 5) == 0)
    {
        print("\nExiting...\n");
        while (1)
            ;
    }
    else
    {
        print("\n");
        print(input);
        print(" is an unknown command\n");
    }
}

