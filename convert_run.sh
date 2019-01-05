python image_to_text.py zar.jpg zar.txt
python make_noise.py zar.txt 0.15 zar_noisy.txt
python text_to_image.py zar_noisy.txt output.jpg