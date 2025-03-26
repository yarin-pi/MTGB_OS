#include <stdio.h>

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
    
    for(int i = 2; i <= n; i++)
    {
        if(n % i == 0 && is_prime(i))
        {
            printf("%d \n",i);
        }
    }
    
}
int main()
{
    print_divisors(2*3*7*11*17*23*41*97);
}