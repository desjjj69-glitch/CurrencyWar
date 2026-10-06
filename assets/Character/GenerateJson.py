import os
import json

# 必须字段及其默认值（Name 和 Path 动态获取，Star 和 Cost 默认设为 1）
DEFAULT_VALUES = {
    'Star': 1,
    'Cost': 1,

}
REQUIRED_FIELDS = ['Name', 'Star', 'Cost', 'Path']

def get_png_files(root_dir):
    """获取 root_dir 下所有 PNG 文件的相对路径（使用正斜杠）"""
    png_files = []
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith('.png'):
                full_path = os.path.join(dirpath, filename)
                rel_path = os.path.relpath(full_path, root_dir)
                # 统一使用正斜杠，兼容 JSON
                rel_path = rel_path.replace('\\', '/')
                png_files.append(rel_path)
    return png_files

def ensure_complete_item(item, name, path):
    """确保条目包含所有必须字段，缺失则补齐（Name 和 Path 使用传入值，其他使用默认值）"""
    item['Name'] = name
    item['Path'] = path
    for field in REQUIRED_FIELDS:
        if field not in item:
            if field in DEFAULT_VALUES:
                item[field] = DEFAULT_VALUES[field]
            # Name 和 Path 已在上面赋值，不会缺失
    return item

def main():
    # 脚本所在目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(script_dir, 'BasicCharacterSettings.json')

    # 读取现有 JSON（若存在）
    if os.path.exists(json_path):
        with open(json_path, 'r', encoding='utf-8') as f:
            try:
                data = json.load(f)
            except json.JSONDecodeError:
                data = {}
        characters = data.get('Characters', [])
    else:
        characters = []
        data = {}

    # 建立现有条目索引（以 Path 为键）
    existing = {item['Path']: item for item in characters if 'Path' in item}

    # 获取当前所有 PNG 文件
    png_paths = get_png_files(script_dir)

    # 生成新的条目列表
    new_characters = []
    for rel_path in png_paths:
        # 从路径提取文件名（不含扩展名）作为 Name
        name = os.path.splitext(os.path.basename(rel_path))[0]

        if rel_path in existing:
            # 已有条目：更新 Name，并确保字段完整
            item = existing[rel_path]
            item = ensure_complete_item(item, name, rel_path)
        else:
            # 新条目：使用默认值
            item = {
                'Name': name,
                'Star': DEFAULT_VALUES['Star'],
                'Cost': DEFAULT_VALUES['Cost'],
                'Path': rel_path
            }
        new_characters.append(item)

    # 注意：原 JSON 中存在但文件已被删除的条目不会出现在 new_characters 中，即自动移除

    # 更新并写入 JSON
    data['Characters'] = new_characters
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    print(f"已更新 {json_path}，共 {len(new_characters)} 个条目。")

if __name__ == '__main__':
    main()