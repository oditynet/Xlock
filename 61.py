from PIL import Image, ImageEnhance
import qrcode
import sys

def adjust_color(bg_pixel, invert=False, intensity=0.7):
    """Адаптирует цвет модуля на основе фона"""
    if isinstance(bg_pixel, int):
        bg_pixel = (bg_pixel, bg_pixel, bg_pixel)
    
    if invert:
        return tuple(
            (min(255, max(0, int(255 - (255 - c) * intensity))))
            for c in bg_pixel[:3]
        )
    else:
        return tuple(
            max(0, int(c * (1 - intensity)))
            for c in bg_pixel[:3]
        ) + (220,)

def generate_adaptive_qr(text, base_image, box_size):
    """Генерирует адаптивный QR-код"""
    qr = qrcode.QRCode(
        version=None,
        error_correction=qrcode.constants.ERROR_CORRECT_H,
        box_size=box_size,
        border=4
    )
    qr.add_data(text)
    qr.make(fit=True)
    
    qr_img = qr.make_image(fill_color="black", back_color="white").convert("L")
    qr_width, qr_height = qr_img.size
    x_pos = (base_image.width - qr_width) // 2
    y_pos = (base_image.height - qr_height) // 2
    
    result = Image.new("RGBA", base_image.size)
    result.paste(base_image, (0, 0))
    
    qr_layer = Image.new("RGBA", base_image.size)
    
    for y in range(qr_height):
        for x in range(qr_width):
            if qr_img.getpixel((x, y)) > 128:
                continue
                
            bg_x, bg_y = x_pos + x, y_pos + y
            if 0 <= bg_x < base_image.width and 0 <= bg_y < base_image.height:
                bg_pixel = base_image.getpixel((bg_x, bg_y))
                qr_layer.putpixel(
                    (bg_x, bg_y), 
                    adjust_color(bg_pixel, 0.299*bg_pixel[0] + 0.587*bg_pixel[1] + 0.114*bg_pixel[2] < 160)
                )
    
    return Image.alpha_composite(result, qr_layer)

def embed_qr(input_path, output_path, text):
    """Основная функция"""
    try:
        original = Image.open(input_path).convert("RGBA")
    except IOError:
        print("Ошибка загрузки изображения")
        sys.exit(1)
    
    box_size = max(4, int(min(original.size) * 0.7 / 21)) -10
    result = generate_adaptive_qr(text, original, box_size)
    
    try:
        result.save(output_path, quality=95)
        print(f"✅ QR-код сохранен: {output_path}")
    except Exception as e:
        print(f"Ошибка сохранения: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Использование: python qr_nocopy.py вход.png выход.png 'текст'")
        sys.exit(1)
    
    embed_qr(sys.argv[1], sys.argv[2], sys.argv[3])
