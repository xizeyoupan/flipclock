import shutil
import os

from PIL import Image

current_path = os.path.dirname(os.path.realpath(__file__))
output_dir_path = os.path.join(current_path, 'output')
normal_dir_path = os.path.join(current_path, 'normal')
original_dir_path = os.path.join(current_path, 'original')

if os.path.exists(output_dir_path):
    shutil.rmtree(output_dir_path)
os.mkdir(output_dir_path)


def right_replace(string: str, old: str, new: str, _max=1):
    return string[::-1].replace(old[::-1], new[::-1], _max)[::-1]


def get_top(img):
    width, height = img.size
    return img.crop((0, 0, width, height // 2))


def get_bottom(img):
    width, height = img.size
    return img.crop((0, height // 2, width, height))


def get_blank():
    width, height = 56 * 8, 84 * 8
    background_color = (255, 255, 255)  # 白色
    return Image.new("RGB", (width, height), background_color)


final_config_file = open(os.path.join(output_dir_path, 'config.txt'), 'w', encoding='utf8')
config_files = []
_cnt = 0
for root, ds, fs in os.walk(normal_dir_path):
    for f in fs:
        if f.endswith(".txt"):
            _config_path = os.path.join(root, f)
            config_files.append(_config_path)
            _config = open(_config_path, 'r', encoding='utf8')
            final_config_file.write(str(_cnt) + '\n')
            _cnt += 1
            final_config_file.writelines(_config.readlines())
            final_config_file.flush()
            _config.close()
final_config_file.write("-1\n")
final_config_file.close()

for config_path in config_files:
    normal_group_dir_path = os.path.dirname(config_path)
    output_group_dir_path = right_replace(normal_group_dir_path, "normal", "output")
    os.mkdir(output_group_dir_path)

    top_dir_path = os.path.join(output_group_dir_path, "top")
    bottom_dir_path = os.path.join(output_group_dir_path, "bottom")
    os.mkdir(top_dir_path)
    os.mkdir(bottom_dir_path)

    pic_files = open(config_path, 'r', encoding='utf8').readlines()
    pic_files = list(map(lambda x: x.strip().split(), pic_files))

    result = []
    each = []
    dir = "top"
    i = -1

    while i < 40:
        num = (i + 40) % 40
        pic_path = os.path.join(normal_group_dir_path, pic_files[num][0])
        print("Handle " + pic_path)
        im = Image.open(pic_path)

        if dir == "top":
            each.append(get_top(im))
            dir = "bottom"
            i += 1
        elif dir == "bottom":
            each.append(get_bottom(im))
            dir = "top"

        if len(each) == 4:
            result.append(each)
            each = []

    for i, v in enumerate(result):
        top_path = os.path.join(top_dir_path, f"{i}.jpg")
        bottom_path = os.path.join(bottom_dir_path, f"{i}.jpg")

        im = get_blank()
        im.paste(v[1], (0, 84 * 4))
        im.paste(v[2], (0, 0))
        im.save(top_path, quality=95)

        im = get_blank()
        im.paste(v[3], (0, 84 * 4))
        im.paste(v[0], (0, 0))
        im = im.rotate(180)
        im.save(bottom_path, quality=95)
