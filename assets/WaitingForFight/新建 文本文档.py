#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
扫描脚本所在文件夹（不包含子文件夹）下的所有 .png 文件，
生成 WaitingForFight.json。

结构：
{
    "Picture Name": {
        "图片名(不含扩展名)": ["图片名.png"]
    },
    "Place": {
        "图片名(不含扩展名)": ["0.0 0.0 0.0 0.0"]
    }
}
"""

import json
import os

# 输出文件名
OUTPUT_JSON = "WaitingForFight.json"

# Place 的默认值（x y w h）
DEFAULT_PLACE = "0.0 0.0 0.0 0.0"


def get_script_dir():
    """获取脚本所在目录（兼容双击运行 / 命令行运行）"""
    try:
        return os.path.dirname(os.path.abspath(__file__))
    except NameError:  # 某些交互式环境下没有 __file__
        return os.getcwd()


def find_png_files(folder):
    """返回该文件夹下（不含子文件夹）所有 png 文件名，按名称排序"""
    result = []
    for name in os.listdir(folder):
        full_path = os.path.join(folder, name)
        # 必须是文件（排除同名文件夹），且扩展名为 .png（不区分大小写）
        if os.path.isfile(full_path) and name.lower().endswith(".png"):
            result.append(name)
    result.sort(key=lambda s: s.lower())
    return result


def build_data(png_files, old_data=None):
    """构造 json 数据；若提供了旧的 json 数据，则保留其中已填写过的 Place 值"""
    picture_name = {}
    place = {}

    old_place = {}
    if isinstance(old_data, dict):
        old_place = old_data.get("Place", {}) or {}

    for filename in png_files:
        key = os.path.splitext(filename)[0]  # 去掉扩展名

        # 类型一：Picture Name
        picture_name[key] = [filename]

        # 类型二：Place，若旧文件里已经填过就沿用，否则用默认值
        old_value = old_place.get(key)
        if isinstance(old_value, list) and old_value:
            place[key] = old_value
        elif isinstance(old_value, str) and old_value:
            place[key] = [old_value]
        else:
            place[key] = [DEFAULT_PLACE]

    return {
        "Picture Name": picture_name,
        "Place": place,
    }


def load_old_json(path):
    """读取已存在的 json（用于保留已填写的 Place），失败则返回 None"""
    if not os.path.isfile(path):
        return None
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return None


def main():
    folder = get_script_dir()
    out_path = os.path.join(folder, OUTPUT_JSON)

    png_files = find_png_files(folder)
    old_data = load_old_json(out_path)

    data = build_data(png_files, old_data)

    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)

    print("脚本目录：%s" % folder)
    print("共找到 %d 个 png 文件：" % len(png_files))
    for name in png_files:
        print("  - %s" % name)
    print("已生成：%s" % out_path)


if __name__ == "__main__":
    main()