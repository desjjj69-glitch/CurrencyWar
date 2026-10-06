import json
from pathlib import Path


def main():
    # 获取当前 Python 文件所在目录
    base_dir = Path(__file__).resolve().parent

    # 只遍历当前目录，不递归；筛选 .png 文件，忽略大小写
    png_files = sorted(
        (
            p for p in base_dir.iterdir()
            if p.is_file() and p.suffix.lower() == ".png"
        ),
        key=lambda p: p.name.lower()
    )

    # 生成 { "文件名不带扩展名": "文件名.png" }
    data = {p.stem: p.name for p in png_files}

    # 输出到当前目录下的 png_files.json
    output_path = base_dir / "png_files.json"
    with output_path.open("w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)

    print(f"已生成: {output_path}")
    print(f"共找到 {len(data)} 个 PNG 文件")


if __name__ == "__main__":
    main()