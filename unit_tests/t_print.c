#include "print.h"

int main (void)
{
    USART_init(USART_BAUD_RATE);
    print("HelloWorld ");
    print("Hello\n");
    print("Expected  | Actual");
    print("44 45      %d %d", 44, 45);
    print("255 300    %c %u", 255, 300);
    print("3000000    %lu", 3000000);
    print("ffffffff   %lx", 0xffffffff);
    print("300000     %lu", 300000);
    print("65000      %u", 65000);
    print("ff         %x", 255);
    print("beef       %x", 0xbeef);
    print("b0b        %x", 0xb0b);
    print("0          %x", 0);
}
