import shutil
import os
import time

from PIL import Image
import importlib

uploader_name = "pz"
current_path = os.path.dirname(os.path.realpath(__file__))
normal_dir_path = os.path.join(current_path, 'normal')
original_dir_path = os.path.join(current_path, 'original')

uploader = importlib.import_module(f"uploader.{uploader_name}")


def transparence2white(img):
    sp = img.size
    width = sp[0]
    height = sp[1]

    for yh in range(height):
        for xw in range(width):
            dot = (xw, yh)
            color_d = img.getpixel(dot)
            if color_d[3] == 0:
                color_d = (255, 255, 255, 255)
                img.putpixel(dot, color_d)
    return img


def try_upload(_path: str, max_time: int = 3):
    _time = 0
    while _time < max_time:
        try:
            url = uploader.upload(_path)
            return url
        except Exception as e:
            print(e)
            time.sleep(1)
            _time += 1
    assert _time < max_time


def right_replace(string: str, old: str, new: str, _max=1):
    return string[::-1].replace(old[::-1], new[::-1], _max)[::-1]


if os.path.exists(normal_dir_path):
    shutil.rmtree(normal_dir_path)
os.mkdir(normal_dir_path)

config_files = []
for root, ds, fs in os.walk(original_dir_path):
    for f in fs:
        if f.endswith(".txt"):
            config_files.append(os.path.join(root, f))
            continue

        pic_path = os.path.join(root, f)
        print(pic_path)
        im = Image.open(pic_path)
        print(im.mode)

        if im.mode == "RGBA":
            im = transparence2white(im)
            im = im.convert('RGB')

        target = (56 * 8, 84 * 8)
        if im.size != target:
            im = im.resize(target, resample=Image.LANCZOS)
            print("resized.")
        group_dir_name = os.path.basename(root)
        name, suffix = os.path.splitext(f)

        _dir = os.path.join(normal_dir_path, group_dir_name)
        if not os.path.exists(_dir):
            os.mkdir(_dir)

        im.save(os.path.join(_dir, f"{name}.jpg"), quality=95)

for config in config_files:
    original_config_file = open(config, 'r', newline='\n', encoding='utf8')
    normal_config_file_name = right_replace(config, "original", "normal")
    normal_config_file = open(normal_config_file_name, "w", newline='\n', encoding='utf8')
    group_dir_name = os.path.dirname(normal_config_file_name)

    for line in original_config_file.readlines():
        pic_name, pic_type = line.split()
        pic_name, suffix = os.path.splitext(pic_name)
        pic_path = os.path.join(group_dir_name, f"{pic_name}.jpg")

        if pic_type == "image":
            pic_type = try_upload(pic_path)
            print(pic_path)
            print(pic_type)
            print()

        normal_config_file.write(f"{pic_name}.jpg {pic_type}\n")
        normal_config_file.flush()

    original_config_file.close()
    normal_config_file.close()
