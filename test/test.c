#include <stdio.h>
#include <stdlib.h>

struct Foo
{
    int x;
    int y;
};

typedef struct Foo F;

void show(int arr[])
{
    printf("%lu\n", sizeof(*arr));
    arr[0] = 9;
}

void replace(char *x)
{
    while (*x != '\0')
    {
        printf("%c\n", *x++);
    }
}

int main(void)
{
    F *f = (F *) malloc(sizeof(F));
    f->x = 45;
    f->y = 67;
    printf("%p %p\n%d %d\n", 
                &f->x, &f->y, f->x, f->y);
    ((F *)(&f->y))->x = 12;
    void *p = &f->x;
    int *i = (int *)p;
    *i = 75;
    printf("%p %p\n%d %d\n", 
                &f->x, &f->y, f->x, f->y);
    int a[] = {1, 2, 3, 4};
    show(a);
    for (int i = 0; i < 4; i++) printf("%d\t", a[i]);
    printf("\n");
    char x[] = "hello";
    x[0] = 'p';
    replace(x);
    for (int i = 0; i < 5; i++) printf("%c\t", x[i]);
    printf("\n");
    return 0;
}