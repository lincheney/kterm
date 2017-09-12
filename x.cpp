#include <X11/Xlibint.h>

char* x_display(void* display)
{
    Display* d = (Display*)display;
    return d->display_name;
}
