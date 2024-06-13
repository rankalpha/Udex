自己实现的安卓dex dump工具，可以处理普通一代和二代壳，以及简单的二代抽取壳，无法处理 vm 化的三代壳，以及变形的二代抽取壳。

本工具的优点，不用修改 rom ，直接编译此项目后使用自动化脚本即可以脱壳。
另外因为工具还没有公开扩散，目前检测下来几乎没有壳可以检测出本工具。

本工具仅为自己使用。

参考了以下项目：
LSPosed
androidGRPC
Poros

2024/06/07
目前只在自己的 RedNote7 手机上测试成功。

2024/06/13
使用静态编译dobby代替以前的动态链接

使用环境：
1、手机安装 EvolutionX 的 android 14 系统，对应的安卓版本为 android-14.0r25。
2、安装 KernelSU 
3、在 KernelSU 中安装 ZygiskNext 以及自己修改过的 ZygiskFrida ，可选择安装 Shamiko。

使用方法一：
1、修改 python 目录下的 futils.py 文件中的 package_name 为你需要 dump 的包名，并保存
2、在电脑上运行 python 目录下的 dump_dex.py 文件即可自动运行 app 并且提取结果保存到 python/output 目录下

使用方法二：
1、修改 python 目录下的 futils.py 文件中的 package_name 为你需要 dump 的包名，并保存
2、在电脑上运行 python 目录下的 create_work_env.py 程序，程序会自动复制工作需要的文件到手机上
3、在手机上运行 app , 如果有监控 logcat 的程序可以查看 tag 为 UDEX 或者 DEXHOOK 的信息是否出现，如果有，则通常程序在后台开始工作
4、在电脑上运行 python 目录下的 get_dump_result.py 程序，程序会把结果提取到 python/output 目录下
