/* Simple CallBack from Context */

#include <stdio.h>
#include <stdint.h>

typedef struct event
{
    int id;
    char data[20];
}event_t;

typedef void (*event_cb_t) (const event_t *evt, void *data);

typedef void (*ptr)(); 


typedef struct cb_Data {
    event_cb_t cb;
    void *data;
}cb_Data_t;


void event_fucntion(const event_t *evt1, int *data)
{
    printf("evt1->data = %s\n", evt1->data);
    printf("evt1->id = %d\n", evt1->id);
    printf("data = %d\n", *data);
}

int event_cb_register(cb_Data_t *cbi, void *userdata)
{
    event_t e = { 5, "Test data" };
    int b = 26;
    cbi->cb(&e, &b);
    printf("test after callback\n");
}

  
void A()
{
    printf("I am function A\n");
}
  

int main()
{
    // cb_Data_t *c;
    // c->cb = (event_cb_t *)event_fucntion;

    // event_t e = { 5, "Test data" };

    // c->cb(&e, (int*)24);

    cb_Data_t my_event_cb = {event_fucntion, "TEST data"};

    int my_custom_data = 25;

    event_cb_register(&my_event_cb, &my_custom_data);

    ptr p;

    p = A;

    (p)();
}
