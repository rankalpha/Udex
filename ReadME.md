# 简要说明

### 自己实现的安卓dex dump工具，可以处理普通一代和二代壳，以及简单的二代抽取壳，无法处理 vm 化的三代壳，以及变形的二代抽取壳。
### 本工具的优点，不用修改 rom ，直接编译此项目后使用自动化脚本即可以脱壳。
### 另外因为工具还没有公开扩散，目前检测下来几乎没有壳可以检测出本工具。
### 本工具仅为自己使用。

# 参考了以下项目：
### LSPosed (https://github.com/LSPosed/LSPosed)
### androidGRPC (https://github.com/Thehepta/androidGRPC)
### Poros (https://github.com/WindySha/Poros)

# 更新说明
### 2024/06/07
### 在自己的 RedNote7 手机上测试成功，首次提交。

### 2024/06/13
### 使用静态编译dobby代替以前的动态链接

### 2024/06/20
### 添加 frida-gumjs 来方便运行 js 代码

### 2024/09/30
### 更新到 Matrixx 系统。
### 最近才发现 EvolutionX 系统在 SourceForge 的仓库已经删除，并且 EvolutionX 系统宣布不再支持 RedNote7 ，因此就有了更换一个安卓系统的想法。
### 经过测试发现 Matrixx 系统还不错，基于 crDroid 系统，自带 KernelSU ，目前最新的安卓版本为 Android 14.0.0 r62。
### Matrixx 官方主页：https://github.com/ProjectMatrixx
### Matrixx 的 ROM 可以在 SourceForge 下载：https://sourceforge.net/projects/vitor-unofficial-builds/files/Project%20Matrix/
### 以前的 EvolutionX 系统也最终找到一处下载地址：https://sourceforge.net/projects/evolution-x/files/previously_supported_devices/lavender/14/
### 经过测试发现本程序无法直接在 Matrixx 系统下使用，经过调试发现原来是 EvolutionX 系统（android 14.0.0 r25）版本使用的一个小技巧来获取 ArtMethod 类的 GetCodeItem 的方法已经在 Matrixx 系统下失效了。
### 本来以为要大费周章才能找到新的解决方案，哪知道踏破铁鞋无觅处，得来全不费工夫，Matrixx 系统（android 14.0.0 r62）中导出了 art::ArtMethod::GetCodeItem 符号
### 这样就很容易修正这个问题了。经过测试在 EvolutionX 和 Matrixx 系统下，都能成功 dump dex 了。

# 使用环境：
### 1、手机安装 Matrixx 的 android 14 系统，对应的安卓版本为 android-14.0.0r62。（因为 Matrixx 系统自带了 KernelSU ，因此不需要再单独安装 KernelSU）
### 2、在 KernelSU 中安装 ZygiskNext (https://github.com/Dr-TSNG/ZygiskNext/tree/master) 以及自己修改过的 ZygiskFrida (https://github.com/rankalpha/ZygiskFrida) ，可选择安装 Shamiko。

# 使用方法一：
### 1、修改 python 目录下的 futils.py 文件中的 package_name 为你需要 dump 的包名，并保存
### 2、在电脑上运行 python 目录下的 dump_dex.py 文件即可自动运行 app 并且提取结果保存到 python/output 目录下

# 使用方法二：
### 1、修改 python 目录下的 futils.py 文件中的 package_name 为你需要 dump 的包名，并保存
### 2、在电脑上运行 python 目录下的 create_work_env.py 程序，程序会自动复制工作需要的文件到手机上
### 3、在手机上运行 app , 如果有监控 logcat 的程序可以查看 tag 为 UDEX 或者 DEXHOOK 的信息是否出现，如果有，则通常程序在后台开始工作
### 4、在电脑上运行 python 目录下的 get_dump_result.py 程序，程序会把结果提取到 python/output 目录下
