#include "stdl.h"

void swap(int* ptr1, int* ptr2)
{
    int tmp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = tmp;
}
void sort(int* arr, int len)
{
    for(int i = 0; i < len; i++)
    {
        for(int j = i + 1; j < len; j++)
        {
            if(arr[j] < arr[i])
            {
                swap(&arr[j],&arr[i]);

            }
        }
    }
}
int main()
{
    int ar[10] = {4,2,7,19,24,14,35,23,11,44};
    char t[2] = {0,0};
    sort(ar,10);
    for(int i = 0; i < 10;i++)
    {
        t[0] = ar[i];
        print(t);
        print("\n");
        wait(1);

    }
}