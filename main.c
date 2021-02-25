/*
o-------------------------------------------------------------------o
| main.c - Exebição de imagem apartir da leitura de um arquivo .bmp |
o-------------------------------------------------------------------o

github: https://github.com/S1riu50x01;

Author: Renan Silva, aka Siriu50x01;

Sources: https://en.wikipedia.org/wiki/BMP_file_format;
         https://tronche.com/gui/x/xlib/utilities/manipulating-images.html;

compile: clang -lX11 -o main main.c
run: ./main <filename.bmp>
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "loadbmp.h"

int main(int argc, char *argv[])
{
    Display *dpy;
    int screen;
    Window win;
    XEvent event;
    Visual *visual;

    unsigned int width;
    unsigned int height;
    unsigned int *p;

    /* Carrega uma imagem BMP passado pelo argumento 1 do terminal */
    if (argc > 1)
    {
        p = load_image(argv[1], &width, &height);
    }

    /* Cria uma janela */
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
    screen = DefaultScreen(dpy);
    visual = DefaultVisual(dpy, screen);
    
    // tamanho do monitor atual
    int dWidth = DisplayWidth(dpy, screen);
    int dHeight = DisplayHeight(dpy, screen);

    win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, BlackPixel(dpy, screen), WhitePixel(dpy, screen));
    XMapWindow(dpy, win);

    /* Aloca a memoria necessária para a struct XImage para a exibição da nossa imagem */
    char *data = (char*)malloc(width*height*4);
    XImage *img = XCreateImage(dpy, visual, DefaultDepth(dpy, screen),ZPixmap,
        0, data, width, height, 32, 0);


    /* Substitui todos os pixels de img pelos pixels obtido do arquivo .BMP */
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t pixel[3];
            pixel[0] = p[y*width*3 + (x*3)+0]; // blue 0x00 - 0xff
            pixel[1] = p[y*width*3 + (x*3)+1]; // green 0x00 - 0xff
            pixel[2] = p[y*width*3 + (x*3)+2]; // blue 0x00 - 0xff
            XPutPixel(img, x, y, pixel[2] << 16 | pixel[1] << 8 | pixel[0]);
        }
    }

    XSelectInput(dpy, win, ExposureMask | KeyPressMask);
    
    while (1)
    {
        XNextEvent(dpy, &event);
        if (event.type == Expose)
        { 
            /* Move a janela para o centro da tela */
            XMoveWindow(dpy, win, (dWidth/2)-(width/2), (dHeight/2)-(height/2));

            /* Mostra a imagem na tela */
            XPutImage(dpy,win,DefaultGC(dpy,screen),img,0,0,0,0,width,height);           
        }

    }

    return 0;
}