#include "stdl.h"

int is_prime(int n)
{

    for (int i = 2; i < n;i++)
    {
        if(!(n % i))
        {
            return 0;
        }
    }
    return 1;
}

int print_divisors(int n)
{
    char t[2] = {0, 0};
    for(int i = 2; i <= n; i++)
    {
        if(n % i == 0 && is_prime(i))
        {
            t[0] = i + 48;
            print(t);
            print("\n");
            wait(1);
        }
    }
    
}
int _start()
{
    print_divisors(2*3*7*11*17*23*41*97);
}