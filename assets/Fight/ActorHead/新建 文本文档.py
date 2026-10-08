import uuid
from pathlib import Path


def rename_images(folder: str = "."):
    folder = Path(folder)

    # 支持的图片扩展名，可按需增删
    image_exts = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", ".tif", ".tiff"}

    # 找出所有图片文件（不递归子文件夹）
    images = [
        p for p in folder.iterdir()
        if p.is_file() and p.suffix.lower() in image_exts
    ]

    # 按文件名排序，保证编号顺序稳定
    images.sort(key=lambda p: p.name.lower())

    if not images:
        print("没有找到图片文件。")
        return

    images_set = set(images)

    # 预生成目标文件名，并检查冲突
    targets = []
    for i, p in enumerate(images, start=1):
        # 扩展名统一转为小写，例如 .JPG -> .jpg
        target = folder / f"mob_{i}_Head{p.suffix.lower()}"

        # 如果目标文件已存在，且不是本次要重命名的图片，则中止
        if target.exists() and target not in images_set:
            raise SystemExit(f"冲突：目标文件已存在且不是待重命名图片：{target}")

        targets.append(target)

    # 第一阶段：先改成唯一临时名，避免重命名过程中互相覆盖
    temp_pairs = []
    for p, target in zip(images, targets):
        temp = folder / f"__tmp_rename_{uuid.uuid4().hex}{p.suffix}"

        while temp.exists():
            temp = folder / f"__tmp_rename_{uuid.uuid4().hex}{p.suffix}"

        p.rename(temp)
        temp_pairs.append((p, temp, target))

    # 第二阶段：改成 mob_id 形式
    for orig, temp, target in temp_pairs:
        temp.rename(target)
        print(f"{orig.name} -> {target.name}")


if __name__ == "__main__":
    # 默认处理当前文件夹。
    # 如果想处理指定文件夹，可以改成：
    # rename_images(r"C:\Users\xxx\Pictures")
    rename_images(".")