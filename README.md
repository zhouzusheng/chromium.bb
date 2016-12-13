# minkit


* 本分支主要目的是修改Bloomberg Chromium， 删除不必要的代码，减少编译出来的代码大小。
* 删除的代码包括：
    * 打印支持
    * PDF 组件
    * HTML5 media, medea capture
    * DEVICE API (usb， power， etc.）
    * WEBRTC
    * WEBP
    * PPAPI（保留 NPAPI）
    * SANDBOX
    * remoting
    * safe_browsing
    * speech	
    * 其他一些小功能	

## Build Instructions


* 准备编译环境:
    * [Python 2.7](https://www.python.org/download/releases/2.7.6/)
    * Visual Studio 2013 Update 4 (see [VS updates](https://support.microsoft.com/en-us/kb/2829760))
    * 修改 setenv.bat, 替换其中的 python 路径
    * 启动 VS2013 控制台
    * 切换到顶层目录
    * setenv.bat    
    * src/build/runhooks

* 命令行编译:
            
            启动 VS2013 控制台
            切换到顶层目录
            setenv.bat
            ninja -C src/out/Debug     # for Debug builds
            ninja -C src/out/Release   # for Release builds

* 从 Visual Studio 编译:
            
            启动 VS2013 控制台
            切换到顶层目录
            setenv.bat
            devenv src\minkit\minkit.sln
            Build the `minkit_all` project.
    

---
###### Microsoft, Windows, Visual Studio and ClearType are registered trademarks of Microsoft Corp.
###### Firefox is a registered trademark of Mozilla Corp.
###### Chrome is a registered trademark of Google Inc.
