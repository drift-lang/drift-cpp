#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct Foo
{
    int x;
};

int main()
{
    int s = sizeof(struct Foo);

    struct Foo *f = malloc(s);
    f->x = 54;
    
    printf("x = %d\n", f->x);

    struct Foo *n = malloc(s);
    memcmp(n, f, s);

    printf("x = %d\n", n->x);
    return 0;
}