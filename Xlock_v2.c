#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#define PASSWORD "123"
#define FONT_NAME "DejaVu Sans:size=24:antialias=true"
#define DOT_RADIUS 8
#define DOT_SPACING 20
#define BACKGROUND_COLOR "#000000"  // Черный фон
#define TEXT_COLOR "#FFFFFF"        // Белый текст

typedef struct {
    Display* dpy;
    Window win;
    XftFont* font;
    XftDraw* draw;
    XftColor text_color;
    XftColor bg_color;
    Visual* visual;
    Colormap colormap;
    int screen;
    int width;
    int height;
} LockState;

void init_x11(LockState* s) {
    s->dpy = XOpenDisplay(NULL);
    s->screen = DefaultScreen(s->dpy);
    s->width = DisplayWidth(s->dpy, s->screen);
    s->height = DisplayHeight(s->dpy, s->screen);

    // Получаем визуал и цветовую карту
    s->visual = DefaultVisual(s->dpy, s->screen);
    s->colormap = DefaultColormap(s->dpy, s->screen);

    // Настройка окна
    XSetWindowAttributes attrs = {
        .override_redirect = True,
        .background_pixel = BlackPixel(s->dpy, s->screen),
        .border_pixel = BlackPixel(s->dpy, s->screen)
    };

    s->win = XCreateWindow(s->dpy, RootWindow(s->dpy, s->screen),
        0, 0, s->width, s->height, 0,
        CopyFromParent, InputOutput, s->visual,
        CWOverrideRedirect | CWBackPixel | CWBorderPixel,
        &attrs);

    XMapWindow(s->dpy, s->win);
    XGrabKeyboard(s->dpy, s->win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

    // Инициализация Xft
    s->font = XftFontOpenName(s->dpy, s->screen, FONT_NAME);
    s->draw = XftDrawCreate(s->dpy, s->win, s->visual, s->colormap);

    // Цвета
    XftColorAllocName(s->dpy, s->visual, s->colormap, TEXT_COLOR, &s->text_color);
    XftColorAllocName(s->dpy, s->visual, s->colormap, BACKGROUND_COLOR, &s->bg_color);
}

void draw_centered_text(LockState* s, const char* text,int k) {
    XGlyphInfo extents;
    XftTextExtentsUtf8(s->dpy, s->font, (FcChar8*)text, strlen(text), &extents);

    int x = (s->width - extents.width) / 2;
    int y = (s->height - extents.height) / 2;

    XftDrawStringUtf8(s->draw, &s->text_color, s->font, x, y+k, 
                     (FcChar8*)text, strlen(text));
}

void draw_password_dots(LockState* s, int count) {
    int total_width = (DOT_RADIUS * 2 + DOT_SPACING) * count - DOT_SPACING;
    int start_x = (s->width - total_width) / 2;
    int y = s->height / 2 + 50;

    for (int i = 0; i < count; i++) {
        XFillArc(s->dpy, s->win, DefaultGC(s->dpy, s->screen),
                start_x + i * (DOT_RADIUS * 2 + DOT_SPACING), y - DOT_RADIUS,
                DOT_RADIUS * 2, DOT_RADIUS * 2, 0, 360 * 64);
    }
}

void lock_loop(LockState* s) {
    XEvent ev;
    char pass[64] = {0};
    int pass_len = 0;
    int attempts = 0;

    while (1) {
        // Очистка экрана
        XClearWindow(s->dpy, s->win);

        // Отрисовка текста
        draw_centered_text(s, "СИСТЕМА ЗАБЛОКИРОВАНА",-30);
        draw_password_dots(s, pass_len);

        // Отрисовка сообщения о неверном пароле
        if (attempts > 0) {
            char buf[64];
            snprintf(buf, sizeof(buf), "Неверный пароль (%d попытка)", attempts);
            draw_centered_text(s, buf,20);
        }

        XFlush(s->dpy);

        // Обработка событий
        XNextEvent(s->dpy, &ev);
        if (ev.type == KeyPress) {
            KeySym keysym;
            char buffer[8];
            int len = XLookupString(&ev.xkey, buffer, sizeof(buffer), &keysym, NULL);

            if (keysym == XK_Return) {
                if (strcmp(pass, PASSWORD) == 0) break;
                attempts++;
                pass[0] = '\0';
                pass_len = 0;
            } else if (keysym == XK_BackSpace && pass_len > 0) {
                pass[--pass_len] = '\0';
            } else if (pass_len < sizeof(pass) - 1 && len > 0) {
                pass[pass_len++] = buffer[0];
                pass[pass_len] = '\0';
            }
        }
    }
}

int main() {
    setlocale(LC_ALL, "");

    LockState state = {0};
    init_x11(&state);
    lock_loop(&state);

    XUngrabKeyboard(state.dpy, CurrentTime);
    XDestroyWindow(state.dpy, state.win);
    XCloseDisplay(state.dpy);
    return 0;
}
