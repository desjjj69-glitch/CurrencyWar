import os
import json

# 必须字段（Name 和 Path 动态获取，Type 默认空）
REQUIRED_FIELDS = ['Name', 'Path', 'Type']
DEFAULT_VALUES = {
    'Type': ''
}

def get_png_files(root_dir):
    """获取 root_dir 下所有 PNG 文件的相对路径（使用正斜杠）"""
    png_files = []
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith('.png'):
                full_path = os.path.join(dirpath, filename)
                rel_path = os.path.relpath(full_path, root_dir)
                rel_path = rel_path.replace('\\', '/')
                png_files.append(rel_path)
    return png_files

def ensure_complete_item(item, name, path):
    """确保条目包含所有必须字段，缺失则补齐（Name 和 Path 使用传入值，Type 使用默认值）"""
    item['Name'] = name
    item['Path'] = path
    for field in REQUIRED_FIELDS:
        if field not in item:
            if field in DEFAULT_VALUES:
                item[field] = DEFAULT_VALUES[field]
    return item

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(script_dir, 'BasicEquipmentSettings.json')

    # 读取现有 JSON（若存在）
    if os.path.exists(json_path):
        with open(json_path, 'r', encoding='utf-8') as f:
            try:
                data = json.load(f)
            except json.JSONDecodeError:
                data = {}
        equipment = data.get('Equipment', [])
    else:
        equipment = []
        data = {}

    # 以 Path 为键建立索引
    existing = {item['Path']: item for item in equipment if 'Path' in item}

    # 获取当前所有 PNG 文件
    png_paths = get_png_files(script_dir)

    # 生成新条目列表
    new_equipment = []
    for rel_path in png_paths:
        name = os.path.splitext(os.path.basename(rel_path))[0]
        if rel_path in existing:
            item = existing[rel_path]
            item = ensure_complete_item(item, name, rel_path)
        else:
            item = {
                'Name': name,
                'Path': rel_path,
                'Type': DEFAULT_VALUES['Type']
            }
        new_equipment.append(item)

    # 更新 JSON（原 JSON 中已删除文件对应的条目会自动移除）
    data['Equipment'] = new_equipment
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    print(f"已更新 {json_path}，共 {len(new_equipment)} 个条目。")

if __name__ == '__main__':
    main()