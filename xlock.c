// gcc 4.c -o 4 -lX11 -ldbus-1 -I /usr/include/dbus-1.0/ -I /usr/lib32/dbus-1.0/include/
#include <X11/Xlib.h>
#include <ctype.h>
#include <locale.h>
#include <X11/keysym.h>
#include <dbus/dbus.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PASSWORD "123"
#define LOCK_MSG "Display is LOCKED. Enter your password:"

// Глобальные переменные для X11
Display *display;
Window win;
GC gc;

// Проверка пароля
int check_password(const char *input) {
    return strcmp(input, PASSWORD) == 0;
}

// Отрисовка интерфейса блокировки
void draw_lock_screen(const char *input) {
    XFontStruct* font;
    char *name="-adobe-courier-medium-o-normal--34-240-100-100-m-200-iso8859-9";
    //char* name = "-*-dejavu sans-bold-r-*-*-*-220-100-100-*-*-iso8859-1";
    font = XLoadQueryFont(display, name);
    XSetFont(display, gc, font->fid);

   // XClearWindow(display, win);
    XDrawString(display, win, gc, 200, 200, LOCK_MSG, strlen(LOCK_MSG));
    //XDrawString(display, win, gc, 200, 240, input, strlen(input));
    XFlush(display);
}

// Получение активных сессий через D-Bus
void get_active_sessions() {
    DBusConnection *conn;
    DBusError err;
    DBusMessage *msg, *reply;
    
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    
    msg = dbus_message_new_method_call(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "ListSessions"
    );

    reply = dbus_connection_send_with_reply_and_block(conn, msg, 1000, &err);
    
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus error: %s\n", err.message);
        dbus_error_free(&err);
    }
    
    dbus_message_unref(msg);
    dbus_connection_unref(conn);
}

// Основная функция блокировки
void lock_screen() {
    XEvent ev;
    char pass[64] = {0};
    int pass_len = 0;

    // Настройка X11
    display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);
    
    XSetWindowAttributes attrs = {
        .override_redirect = True,
        .background_pixel = BlackPixel(display, DefaultScreen(display))
    };
    
    win = XCreateWindow(display, root, 0, 0, 
                      DisplayWidth(display, 0),
                      DisplayHeight(display, 0), 0,
                      CopyFromParent, InputOutput,
                      CopyFromParent, CWOverrideRedirect | CWBackPixel, &attrs);
    
    XMapWindow(display, win);
    XGrabKeyboard(display, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    
    gc = XCreateGC(display, win, 0, NULL);
    XSetForeground(display, gc, WhitePixel(display, 0));

    // Главный цикл
    while (1) {
        XNextEvent(display, &ev);
        
        if (ev.type == KeyPress) {
            KeySym keysym;
            char buffer[8];
            XLookupString(&ev.xkey, buffer, sizeof(buffer), &keysym, NULL);

            if (keysym == XK_Return) {
                if (check_password(pass)) break;
                pass[0] = '\0';
                pass_len = 0;
            } 
            else if (keysym == XK_BackSpace && pass_len > 0) {
                pass[--pass_len] = '\0';
            }
            else if (pass_len < sizeof(pass)-1 && isprint(buffer[0])) {
                pass[pass_len++] = buffer[0];
                pass[pass_len] = '\0';
            }
            
            draw_lock_screen(pass);
        }
    }

    XUngrabKeyboard(display, CurrentTime);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
}

int main() {
    //setlocale(LC_ALL, "RU");
    setlocale(LC_ALL,NULL);
    // Проверка активных сессий
    get_active_sessions();
    
    // Блокировка экрана
    lock_screen();
    
    // Сигнал разблокировки для xss-lock
    system("loginctl unlock-session");
    return 0;
}
