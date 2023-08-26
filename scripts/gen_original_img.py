import os
import shutil
from PIL import Image, ImageDraw, ImageFont

ASCII_TTF_FILE = "BebasNeue-Regular.ttf"
CN_TTF_FILE = 'SourceHanSansCN-Bold.ttf'

current_path = os.path.dirname(os.path.realpath(__file__))

original_dir_path = os.path.join(current_path, 'original')
pic_path = os.path.join(current_path, 'pic')

group1 = "我你躺起真爆执没气湿看打定猫开说本马出"
group2 = "超妈平床的了行有温度V个于雷始意上你猛"
group3 = "O马可死了怕假爆好歪没看脚一干困对是户"
group4 = "P了爱友戏的有先尊下温好坏你籍硬V！？。"

groups = [group1, group2, group3, group4]

if os.path.exists(original_dir_path):
    shutil.rmtree(original_dir_path)
os.mkdir(original_dir_path)
for i in range(len(groups)):
    os.mkdir(os.path.join(original_dir_path, f"group{i + 1}"))


def gen_imgs(cnt: int, text: str, group_n: int):
    width, height = 56 * 8, 84 * 8
    background_color = (255, 255, 255)  # 白色
    image = Image.new("RGB", (width, height), background_color)
    # 在图像上创建一个绘图对象
    draw = ImageDraw.Draw(image)

    assert len(text) <= 2

    font_size = 0
    font = None
    x = 0
    y = 0

    if text.isascii():
        if len(text) == 1:
            font_size = 640
            font = ImageFont.truetype(os.path.join(current_path, ASCII_TTF_FILE), font_size)

            bbox = draw.textbbox((0, 0), text, font=font)
            text_width = bbox[2] - bbox[0]
            text_height = bbox[3] - bbox[1]
            x = (width - text_width) // 2
            y = 0

        elif len(text) == 2:
            font_size = 448
            font = ImageFont.truetype(os.path.join(current_path, ASCII_TTF_FILE), font_size)

            bbox = draw.textbbox((0, 0), text, font=font)
            text_width = bbox[2] - bbox[0]
            text_height = bbox[3] - bbox[1]
            x = (width - text_width) // 2
            if text.startswith('1'):
                x -= 15
            y = 100
    else:
        assert len(text) == 1
        font_size = 348
        font = ImageFont.truetype(os.path.join(current_path, CN_TTF_FILE), font_size)

        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        x = (width - text_width) // 2
        y = 65

    draw.text((x, y), text, font=font, fill=(0, 0, 0))
    if text == " ":
        text = "blank"
    image.save(os.path.join(current_path, 'original', f"group{group_n}", f"{cnt}_{text}.jpg"))


_end = 10
group_cnt = []

for index, v in enumerate(groups):
    group_cnt.append(_end + len(v) - 1)
    f = open(os.path.join(current_path, 'original', f"group{index + 1}", f"group{index + 1}.txt"), 'w', newline='\n',
             encoding='utf8')

    for i in range(0, _end):
        gen_imgs(i, str(i), index + 1)
        f.write(f"{i}_{i}.jpg {i}\n")

    for i in range(_end, _end + len(v)):
        gen_imgs(i, v[i - _end], index + 1)
        f.write(f"{i}_{v[i - _end]}.jpg {v[i - _end]}\n")

    f.flush()
    f.close()

for root, ds, fs in os.walk(pic_path):
    for f in fs:
        print(root, f)
        base_dir_name = os.path.basename(root)
        group_num, suffix = os.path.splitext(f)
        if not group_num.isdigit():
            continue
        group_num = int(group_num)
        src = os.path.join(root, f)
        group_cnt[group_num - 1] += 1
        dst = os.path.join(original_dir_path, f"group{group_num}",
                           f"{group_cnt[group_num - 1]}_{base_dir_name}{suffix}")
        shutil.copy(src, dst)

        config = open(os.path.join(current_path, 'original', f"group{group_num}", f"group{group_num}.txt"), 'a',
                      newline='\n', encoding='utf8')
        config.write(f"{group_cnt[group_num - 1]}_{base_dir_name}{suffix} image\n")
        config.flush()
        config.close()

for index, v in enumerate(groups):
    while group_cnt[index] < 39:
        group_cnt[index] += 1

        config = open(os.path.join(current_path, 'original', f"group{index + 1}", f"group{index + 1}.txt"), 'a',
                      newline='\n', encoding='utf8')
        gen_imgs(group_cnt[index], " ", index + 1)
        config.write(f"{group_cnt[index]}_blank.jpg blank\n")
        config.flush()
        config.close()
