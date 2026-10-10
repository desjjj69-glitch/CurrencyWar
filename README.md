# CurrencyWar (货币战争)

一个基于 C++ 和 SDL3 开发的战斗/游戏项目，包含角色选择、技能释放、战斗结算等核心逻辑。

## 📖 项目简介
CurrencyWar 是一个使用 C++ 编写的项目，开发环境为 Visual Studio 2026。项目使用 SDL3 进行图形渲染、窗口管理和多媒体处理。

## ✨ 主要功能
- **战斗系统**：角色对战逻辑与结算（Fight/FightResult）。
- **技能系统**：可配置的技能释放与效果。
- **数据驱动**：通过 `config` 和 `data` 文件夹中的 JSON 文件配置角色与技能。

## 🛠️ 环境依赖
在编译和运行本项目之前，请确保你的开发环境满足以下条件：
- **操作系统**：Windows 10 / 11
- **IDE**：Visual Studio 2026
- **第三方库**：
  - SDL3
  - SDL3_image
  - SDL3_ttf

## 🚀 编译与运行指南

### 1. 配置 Visual Studio 环境
本项目通过配置 Visual Studio 的包含目录和库目录来链接 SDL3 系列库：
1. 打开项目属性（右键项目 -> 属性）。
2. 在 **VC++ 目录** -> **包含目录** 中，添加 SDL3、SDL3_image、SDL3_ttf 的头文件路径。
3. 在 **VC++ 目录** -> **库目录** 中，添加 SDL3、SDL3_image、SDL3_ttf 的 `.lib` 文件路径。
4. 在 **链接器** -> **输入** -> **附加依赖项** 中，填入对应的 `.lib` 文件（如 `SDL3.lib`、`SDL3_image.lib`、`SDL3_ttf.lib`）。

### 2. 编译项目
- 使用 Visual Studio 打开 `CurrencyWar.sln`（或者项目文件）。
- 选择合适的平台（推荐 `x64`，请确保与你的 SDL3 库版本一致），点击**生成解决方案**。

### 3. 运行项目
⚠️ **重要提醒**：由于项目使用动态链接库（DLL），编译成功后，必须将 `lib/dll` 文件夹中的 SDL3 相关 DLL 文件（如 `SDL3.dll`, `SDL3_image.dll`, `SDL3_ttf.dll`）复制到生成的 `.exe` 文件所在的目录（例如 `x64/Debug/` 或 `x64/Release/`），否则程序将无法启动。

## 📂 目录结构说明
```text
CurrencyWar/
├── assets/       # 图片、音效等资源文件
├── config/       # 角色、技能等 JSON 配置文件
├── data/         # 游戏数据文件
├── lib/
│   └── dll/      # 存放 SDL3, SDL3_image, SDL3_ttf 等运行所需的 DLL 文件
├── Skill/        # 技能相关的逻辑代码
├── BasicActor.h  # 角色基础类定义
└── ...           # 其他源码文件

## 📄 开源许可证

本项目采用 [MIT License](LICENSE) 许可协议。