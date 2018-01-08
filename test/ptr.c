#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>


typedef struct BTNode
{
    int data;
    struct BTNode *lchild;
    struct BTNode *rchild;
}BTNode,*btnode;

void createNode(BTNode *top)
{
    BTNode *BT;
    BT = (BTNode *)malloc(sizeof(BTNode));
    BT->data = 0x12345678;
    top->lchild = BT;
}

void f1(int x)
{
    x++;
    printf("f1 x = %d, p = 0x%x\n", x,&x);
}

void f2(int *x)
{
    (*x)++;
    printf("f2 x = %d, p = 0x%x\n", (*x), x);
}

void f3(int &x)
{
    x++;
    printf("f2 x = %d, p = 0x%x\n", x, &x);
}


int main(void)
{
    int b = 0x87654321;
    int a = 0x12345678;
    int data;
    //char x1;
    char *x1 = (char *)&a;
    short *x2 = (short *)&a;
    int   *x4 = (int *)&a;

    /*
    BTNode top;
    createNode(&top);
    data = top.lchild->data;
    printf("data1 = 0x%x\n", data);
    data = top.lchild->data;
    printf("data2 = 0x%x\n", data);

    if(top.lchild)
        free(top.lchild);
    */    

    a = 2;
    printf("a = %d, &a = 0x%x\n", a, &a);
    f1(a);    
    printf("a = %d\n", a);
    f2(&a); 
    printf("a = %d\n", a);
    f3(a); 
    printf("a = %d\n", a);
    
    return 0;

/*
    printf("a= 0x%x\n", a);                     //0x1245678 
    printf("&a= 0x%x\n", &a); 
    printf("b= 0x%x\n", b);                     //0x1245678 
    printf("&b= 0x%x\n", &b);  
    printf("\n");
    printf("sizeof(x1) = %d\n", sizeof(x1));        //        
    printf("sizeof(&x1) = %d\n", sizeof(&x1));
    printf("sizeof(*x1) = %d\n", sizeof(*x1));

    printf("\n");
    printf("sizeof(x2) = %d\n", sizeof(x2));
    printf("sizeof(&x2) = %d\n", sizeof(&x2));
    printf("sizeof(*x2) = %d\n", sizeof(*x2));

    printf("\n");
    printf("sizeof(x4) = %d\n", sizeof(x4));
    printf("sizeof(&x4) = %d\n", sizeof(&x4));
    printf("sizeof(*x4) = %d\n", sizeof(*x4));

    printf("\n");
    printf("sizeof(b) = %d\n", sizeof(b));
    printf("sizeof(&b) = %d\n", sizeof(&b));

    printf("\n");
    printf("x1 = 0x%x\n", x1);
    printf("x2 = 0x%x\n", x2);
    printf("x4 = 0x%x\n", x4);

    printf("\n");
    printf("*x1 = 0x%x\n", *x1);
    printf("(*(int*)x1) = 0x%x\n", (*(int*)x1));
    printf("*x2 = 0x%x\n", *x2);
    printf("*x4 = 0x%x\n", *x4);

    x1 = &b;
    x2 = &b;
    x4 = &b;
    printf("\n");
    printf("set value\n");
    printf("&b= 0x%x\n", &b); 
     printf("sizeof(x1) = %d\n", sizeof(x1));        //        
    printf("sizeof(&x1) = %d\n", sizeof(&x1));
    printf("sizeof(*x1) = %d\n", sizeof(*x1));

    printf("\n");
    printf("sizeof(x2) = %d\n", sizeof(x2));
    printf("sizeof(&x2) = %d\n", sizeof(&x2));
    printf("sizeof(*x2) = %d\n", sizeof(*x2));

    printf("\n");
    printf("sizeof(x4) = %d\n", sizeof(x4));
    printf("sizeof(&x4) = %d\n", sizeof(&x4));
    printf("sizeof(*x4) = %d\n", sizeof(*x4));

    printf("\n");
    printf("sizeof(b) = %d\n", sizeof(b));
    printf("sizeof(&b) = %d\n", sizeof(&b));

    printf("\n");
    printf("x1 = 0x%x\n", x1);
    printf("x2 = 0x%x\n", x2);
    printf("x4 = 0x%x\n", x4);

    printf("\n");
    printf("*x1 = 0x%x\n", *x1);
    printf("*x2 = 0x%x\n", *x2);
    printf("*x4 = 0x%x\n", *x4);

    x4 = &a;
    printf("\n");
    printf("x1 = 0x%x\n", x1);
    printf("x2 = 0x%x\n", x2);
    printf("x4 = 0x%x\n", x4);
    */
    return 0;
}
