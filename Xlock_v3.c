#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <Imlib2.h>

#define PASSWORD "123"
#define FONT_NAME "DejaVu Sans:size=32:antialias=true"
#define DOT_RADIUS 8
#define DOT_SPACING 20
#define BACKGROUND_COLOR "#000000"  // Черный фон
#define TEXT_COLOR "#FFFFFF"        // Белый текст

// Названия для групп раскладок (настройте под свою систему)
static const char* group_names[] = {
    "EN",  // Группа 0
    "RU"  // Группа 1
    };

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
    Imlib_Image background_image;
} LockState;

// Функция для получения текущей раскладки
const char* get_current_layout(Display* dpy) {
    XkbStateRec state;
    XkbGetState(dpy, XkbUseCoreKbd, &state);
    
    if (state.group < sizeof(group_names) / sizeof(group_names[0])) {
        return group_names[state.group];
    }
    return "??";
}

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

    // Загрузка фонового изображения
    s->background_image = imlib_load_image("/tmp/screen1.png"); // Загружаем изображение
    if (s->background_image) {
        imlib_context_set_image(s->background_image);
        imlib_context_set_display(s->dpy);
        imlib_context_set_visual(s->visual);
        imlib_context_set_colormap(s->colormap);
        imlib_context_set_drawable(s->win);
    }
}

void draw_centered_text(LockState* s, const char* text, int k) {
    XGlyphInfo extents;
    XftTextExtentsUtf8(s->dpy, s->font, (FcChar8*)text, strlen(text), &extents);

    int x = (s->width - extents.width) / 2;
    int y = (s->height - extents.height) / 2;

    XftDrawStringUtf8(s->draw, &s->text_color, s->font, x, y + k, 
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

void draw_time(LockState* s) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

    draw_centered_text(s, time_str, -100);
}

void draw_keyboard_layout(LockState* s) {
    const char* layout = get_current_layout(s->dpy);
    XGlyphInfo extents;
    
    XftTextExtentsUtf8(s->dpy, s->font, (FcChar8*)layout, strlen(layout), &extents);
    
    int x = 20;//s->width- extents.width - 20;  // Отступ 20 пикселей справа
    int y = extents.height + 20;            // Отступ 20 пикселей сверху
    
    XftDrawStringUtf8(s->draw, &s->text_color, s->font, x, y, 
                     (FcChar8*)layout, strlen(layout));
}

void draw_background(LockState* s) {
    if (s->background_image) {
        imlib_context_set_image(s->background_image);
        imlib_render_image_on_drawable(0, 0);
    }
}

void redraw_screen(LockState* s, int pass_len, int attempts) {
    // Очистка экрана
    //XClearWindow(s->dpy, s->win);

    // Отрисовка фонового изображения
    draw_background(s);

    // Отрисовка времени
    draw_time(s);

    // Отрисовка раскладки клавиатуры
    draw_keyboard_layout(s);

    // Отрисовка текста
    draw_centered_text(s, "СИСТЕМА ЗАБЛОКИРОВАНА", -30);
    draw_password_dots(s, pass_len);

    // Отрисовка сообщения о неверном пароле
    if (attempts > 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Неверный пароль (%d попытка)", attempts);
        draw_centered_text(s, buf, 20);
    }

    XFlush(s->dpy);
}

void lock_loop(LockState* s) {
    XEvent ev;
    char pass[64] = {0};
    int pass_len = 0;
    int attempts = 0;
    time_t last_time = 0;

    while (1) {
        // Проверяем, прошла ли секунда с последнего обновления времени
        time_t current_time = time(NULL);
        if (current_time != last_time) {
            last_time = current_time;
            redraw_screen(s, pass_len, attempts);
        }

        // Обработка событий с таймаутом (100 мс)
        if (XPending(s->dpy) > 0) {
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
        } else {
            // Задержка для уменьшения нагрузки на CPU
            usleep(100000); // 100 мс
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

    // Освобождение ресурсов Imlib2
    if (state.background_image) {
        imlib_context_set_image(state.background_image);
        imlib_free_image();
    }

    return 0;
}
