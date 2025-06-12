#include "input.h"
#include "print.h"
#include "clock.h"
#include "exe.h"
#include "vesa.h"
void process_input(const char *input)
{
    if (strcmp(input, "cls", 4) == 0)
    {
        clear_screen();
    }
    else if (strcmp(input, "echo ", 5) == 0)
    {
        vprint("\n");
        vprint(input + 5); // Print everything after "echo "
        vprint("\n");
    }
    else if (strcmp(input, "exit", 5) == 0)
    {
        vprint("\nExiting...\n");
        while (1)
            ;
    }
    else if (strcmp(input, "load ", 5) == 0)
    {
        char *name = input + 5;
        vprint("\nloading ");
        vprint(name);
        elf_load_file(name);
    }
    else if (strcmp(input, "load", 5) == 0)
    {
        vprint("\nMissing parameters error: filename is needed");
    }
    else if (strcmp(input, "bgcolor ", 8) == 0)
    {
        uint32_t c = str_to_int(input + 8);
        switch (c)
        {
        case 1:
            draw_background(0x00FF0000);
            vesa_set_cursor(0, 0);

            break;
        case 2:
            draw_background(0x0000FF00);
            vesa_set_cursor(0, 0);

            break;
        case 3:
            draw_background(0x000000FF);
            vesa_set_cursor(0, 0);

            break;
        default:
            draw_background(0);
            vesa_set_cursor(0, 0);
            break;
        }
    }
    else
    {
        vprint("\n");
        vprint(input);
        vprint(" is an unknown command\n");
    }
}
