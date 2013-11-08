#include "win32.h"

PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),   // size
    1,                     // version number
    PFD_DRAW_TO_WINDOW |   // support window
    PFD_SUPPORT_OPENGL |   // support OpenGL
    PFD_DOUBLEBUFFER,      // double buffered
    PFD_TYPE_RGBA,         // RGBA type
    24,                    // 24-bit color depth
    0, 0, 0, 0, 0, 0,      // color bits ignored
    0,                     // no alpha buffer
    0,                     // shift bit ignored
    0,                     // no accumulation buffer
    0, 0, 0, 0,            // accum bits ignored
    32,                    // 32-bit z-buffer
    0,                     // no stencil buffer
    0,                     // no auxiliary buffer
    PFD_MAIN_PLANE,        // main layer
    0,                     // reserved
    0, 0, 0                // layer masks ignored
};

BOOL InitPixelFormat(SCREENSAVER *ss)
{
    INT iPixelFormat = ChoosePixelFormat(ss->hdc, &pfd);
    SetPixelFormat(ss->hdc, iPixelFormat, &pfd);
    ss->hglrc = wglCreateContext(ss->hdc);
    return (ss->hglrc != NULL);
}

VOID MakeCurrent(SCREENSAVER *ss)
{
    wglMakeCurrent(ss->hdc, ss->hglrc);
}

void ss_term(void)
{
    XFreeGC(ss.modeinfo.dpy, ss.modeinfo.gc);
    hack_free(&ss.modeinfo);
    wglMakeCurrent(NULL, NULL);
    ReleaseDC(ss.hwnd, ss.hdc);
    wglDeleteContext(ss.hglrc);
    DeleteObject(ss.hbmScreenShot);
    ss.hbmScreenShot = NULL;
}

void ss_clear(Display *d)
{
    glClear(GL_COLOR_BUFFER_BIT);
}

XImage *GetScreenShotXImage(void)
{
    XImage *image;
    INT cx, cy, size;
    BITMAP bm;
    HBITMAP hbm = ss.hbmScreenShot;

    GetObject(hbm, sizeof(bm), &bm);
    cx = bm.bmWidth;
    cy = bm.bmHeight;

    image = XCreateImage(NULL, NULL, 32, RGBAPixmap, 0, NULL, cx, cy, 32, 0);
    if (image)
    {
        size = image->bytes_per_line * image->height;
        image->data = (char *)calloc(size, 1);
        if (image->data != NULL)
        {
            memcpy(image->data, bm.bmBits, size);
            return image;
        }
        free(image->data);
        free(image);
        image = NULL;
    }
    return image;
}

void CreateTextureFromImage(XImage *ximage, GLuint texid)
{
    assert(ximage->format == RGBAPixmap);
    glBindTexture(GL_TEXTURE_2D, texid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, ximage->bitmap_pad / 8);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, ximage->width, ximage->height,
        GL_RGBA, GL_UNSIGNED_BYTE, ximage->data);
}

void
load_texture_async(Screen *screen, Window window,
                   GLXContext glx_context,
                   int desired_width, int desired_height,
                   Bool mipmap_p,
                   GLuint texid,
                   void (*callback) (const char *filename,
                                     XRectangle *geometry,
                                     int image_width,
                                     int image_height,
                                     int texture_width,
                                     int texture_height,
                                     void *closure),
                   void *closure)
{
    XImage *ximage;
    XRectangle geometry;

    ximage = GetScreenShotXImage();
    CreateTextureFromImage(ximage, texid);
    geometry.x = geometry.y = 0;
    geometry.width = ximage->width;
    geometry.height = ximage->height;

    (*callback)(NULL, &geometry, ximage->width, ximage->height,
        geometry.width, geometry.height, closure);
    XDestroyImage(ximage);
}

HGLRC *init_GL(ModeInfo *mi)
{
    return &ss.hglrc;
}

void glXMakeCurrent(Display *d, Window w, GLXContext context)
{
    wglMakeCurrent((HDC)d, context);
}

void glXSwapBuffers(Display *d, Window w)
{
    SwapBuffers(d);
}

void check_gl_error(const char *name)
{
}

void clear_gl_error(void)
{
}