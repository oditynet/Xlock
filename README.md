# Xlock


Screensaver с минимальными возможностями . Поддержка только X11. 

Реализовано:
1. Интеграция с D-Bus
2. Графическая блокировка( Создает полноэкранное окно с флагом override_redirect.Захватывает клавиатуру с помощью XGrabKeyboard.)
3. Работа с xss-lock

Запуск на i3:
```
exec --no-startup-id xss-lock -- Xlock
```

Build:
```
gcc xlock.c -o xlock -lX11 -ldbus-1 -I /usr/include/dbus-1.0/ -I /usr/lib32/dbus-1.0/include/ 
```

Font:
```
xlsfonts | grep courier
```

<img src="https://github.com/oditynet/Xlock/blob/main/pic1.jpg" title="example" width="500" />

# Версия 2:

```
gcc Xlock_v2.c -o Xlock_v2 -lX11 -ldbus-1 -lXft -I /usr/include/dbus-1.0/ -I /usr/lib32/dbus-1.0/include/ -I/usr/include/freetype2/ 
```

<img src="https://github.com/oditynet/Xlock/blob/main/out2.jpg" title="example" width="500" />

# Версия 3: 
Добавил вывод времени и фотографии на фоне.

```
gcc Xlock_v2.c -o Xlock -lX11 -lXft -lImlib2  -ldbus-1 -I /usr/include/dbus-1.0/ -I /usr/lib32/dbus-1.0/include/  -I /usr/include/freetype2/
```

<img src="https://github.com/oditynet/Xlock/blob/main/pic2.jpg" title="example" width="500" />
