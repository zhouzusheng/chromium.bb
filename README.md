# minkit

本分支主要目的是修改Bloomberg Chromium， 删除不必要的代码，进一步减少编译出来的代码大小。
删除的代码包括：
    * 打印支持
    * PDF 组件
    * HTML5 media
    * device（ usb， power etc）
	*WEBRTC
	*WEBP
	*PPAPI（保留 NPAPI）
	

## Build Instructions


* 准备编译环境:
    * [Python 2.7](https://www.python.org/download/releases/2.7.6/)
    * Visual Studio 2013 Update 4 (see [VS updates](https://support.microsoft.com/en-us/kb/2829760))
    * [Ninja](https://github.com/martine/ninja)
    * 修改 setenv.bat, 替换其中的 python 路径
    * 启动 VS2013 控制台
    * 切换到顶层目录
    * setenv.bat    
    * src/build/runhooks

* You can build it either on the command-line or from Visual Studio.
* If building from the command line:

            ninja -C src/out/Debug     # for Debug builds
            ninja -C src/out/Release   # for Release builds

* If building from Visual Studio:
    * **Note:** Even though you are using Visual Studio, it will internally
      build using ninja.  The Visual Studio projects simply invoke ninja when
      you build them.  *Pure* MSVC builds are **not supported**.  These Visual
      Studio projects are useful only for browsing the code and debugging
      (setting breakpoints etc).
    * Open `src/minkit/minkit.sln`.  This solution file should be generated
      from the previous step.
    * Build the `minkit_all` project.
    

---
###### Microsoft, Windows, Visual Studio and ClearType are registered trademarks of Microsoft Corp.
###### Firefox is a registered trademark of Mozilla Corp.
###### Chrome is a registered trademark of Google Inc.
