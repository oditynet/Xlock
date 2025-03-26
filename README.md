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
Добавил вывод времени, раскладки клавиатуры и фотографии на фоне.

```
gcc Xlock_v3.c -o Xlock -lX11 -lXft -lImlib2 -lxkbfile -ldbus-1 -I /usr/include/dbus-1.0/ -I /usr/lib32/dbus-1.0/include/  -I /usr/include/freetype2/

```
Take a screen  /home/user/.i3/lock.sh:
```bash
#!/bin/bash
# Make screenshot
scrot /tmp/screen.png
# Scale image
convert -scale 10% -scale 1000% /tmp/screen.png /tmp/screen1.png
./Xlock
```
bindsym Mod1+ctrl+l exec /home/user/.i3/lock.sh


<img src="https://github.com/oditynet/Xlock/blob/main/pic2.jpg" title="example" width="500" />


# Mega idea

Вы отошли от компьютера и к вам пришли коллеги, но не знаюют где вы. У вас горит монитор и qrcode на нем. Сотрудник его сканирует и он перебрасывает его на набор вашего номера или в ТГ или другие сети.

61.py - очень умный: он анализирует куда положит свой пиксель от qrcode и если фон темный, то цвет точки инвертирует.

<img src="https://github.com/oditynet/Xlock/blob/main/screen4.png" title="example" width="500" />
