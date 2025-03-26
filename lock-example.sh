#!/bin/bash

# Make screenshot
#scrot /tmp/screen.png

# Scale image
convert -scale 10% -scale 1000% /home/odity/Pictures/adfsdfg.png  /tmp/screen1.png
#convert -scale 10% -scale 1000% -gravity center -annotate +0+160 "Enter password:" lock.png /tmp/screen.png /tmp/screen1.png


# Clean
#rm -f /tmp/screen.png

# Lock screen
python /home/odity/.i3/61.py /tmp/screen1.png  /tmp/screen2.png 'tel:+79771234567'
#python /home/odity/.i3/61.py /tmp/screen1.png  /tmp/screen2.png 'tg://resolve?domain=oditynet&text=ты где?'
#python /home/odity/.i3/61.py /tmp/screen1.png  /tmp/screen2.png 'tg://user?id=262480141&text=ты где?'

convert  /tmp/screen2.png  /home/odity/.i3/right_out.png +append /tmp/screen1.png 
i3lock -k -i /tmp/screen1.png
