#include <stdio.h>
#include "../include/index.h"

int main()
{
    add_index_entry("hello.txt", "1234567890abcdef1234567890abcdef12345678");

    print_index();

    return 0;
}