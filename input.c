#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include "input.h"


static int js_fd;
void js_init(void)
{
    js_fd = open("/dev/input/js0", O_RDONLY);
}

int js_state(int b)
{
    struct JS_DATA_TYPE js;

    read(js_fd, &js, JS_RETURN);
    //printf("%#x %d %d\n", js.buttons, js.x, js.y);
    switch (b)
    {
        case 0: // On / Off
            return 1;
        case 1: // A
            return !!(js.buttons & (1<<1));
        case 2: // B
            return !!(js.buttons & (1<<2));
        case 3: // SELECT
            return !!(js.buttons & (1<<8));
        case 4: // START
            return !!(js.buttons & (1<<9));
        case 5: // UP
            return (js.y==1);
        case 6: // DOWN
            return (js.y==255);
        case 7: // LEFT
            return (js.x==1);
        case 8: // RIGHT
            return (js.x==255);
        default:
            return 1;
    }
 
   return 0;
}

