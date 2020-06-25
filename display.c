#include <stdio.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "display.h"
#include "ppu.h"


struct rgb888{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct rgb888 pal_rgb888[64] = { 
 {  84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, {48,    0, 136}, { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, {  60,  24,   0},  
 {  32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, { 0,   60,   0}, {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {   0,   0,   0}, 

 { 152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, {92,   30, 228}, {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, { 120,  60,   0}, 
 {  84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40}, {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {   0,   0,   0}, 

 { 236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236}, {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, { 212, 136,  32},
 { 160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108}, { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {   0,   0,   0}, 

 { 236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, { 228, 196, 144},
 { 204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {   0,   0,   0}  
};


unsigned short palette_rgb565[64];


static int fb_fd;
static unsigned char *fb_mem;
static int px_width;
static int line_width;
static int screen_width;
static struct fb_var_screeninfo var;

static int lcd_fb_display_px(unsigned short color, int x, int y)
{
    unsigned char  *pen8;
    unsigned short *pen16;

    unsigned char r,g,b;

    pen16 = (unsigned short *)(fb_mem + y*line_width + x*px_width);

    *pen16 = color;

    return 0;
}



static int lcd_fb_init()
{
    fb_fd = open("/dev/fb0", O_RDWR);
    if(fb_fd < 0){
        printf("cat't open /dev/fb0 \n");
        return -1;
    }
    if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0){
        close(fb_fd);
        printf("cat't ioctl /dev/fb0 \n");
        return -1;
    }
    px_width = var.bits_per_pixel / 8;
    line_width = var.xres * px_width;
    screen_width = var.yres * line_width;

    fb_mem = (unsigned char *)mmap(NULL, screen_width, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if(!fb_mem){
        close(fb_fd);
        printf("cat't mmap /dev/fb0 \n");
        return -1;
    }
    memset(fb_mem, 0 , screen_width);
    return 0;
}


/* Flush the pixel buffer */
void nes_flush_buf(void)
{
    int x,y,color,idx;
    for (x = 0; x < WIDTH; x++){
        for (y = 0; y < HEIGHT; y++){
            //printf("x:%d y:%d idx:%d ",x, y, x+y*WIDTH);
            idx = screen_color_idx[x+y*WIDTH];
            //printf("screen_clr_idx: %d\n",idx);
            color = palette_rgb565[idx];
            lcd_fb_display_px(color, x, y);
        }
    }
}

static void palette_init(void)
{
    int i = 0;
    unsigned int r,g,b;
    for(i=0;i<64;i++){
        r = pal_rgb888[i].r>>3;
        g = pal_rgb888[i].g>>2;
        b = pal_rgb888[i].b>>3;
        palette_rgb565[i] = (r<<11)|(g<<5)|b;
    }

}

void display_init(void)
{
    palette_init();
    if(lcd_fb_init()){
        printf("lcd fb init error \n");
        return ;
    }else{
        printf("fb init ok\n");
    }
    //nes_flush_buf();
}


