# minkit

����֧��ҪĿ�����޸�Bloomberg Chromium�� ɾ������Ҫ�Ĵ��룬��һ�����ٱ�������Ĵ����С��
ɾ���Ĵ��������
    * ��ӡ֧��
    * PDF ���
    * HTML5 media
    * device�� usb�� power etc��
	*WEBRTC
	*WEBP
	*PPAPI������ NPAPI��
	

## Build Instructions


* ׼�����뻷��:
    * [Python 2.7](https://www.python.org/download/releases/2.7.6/)
    * Visual Studio 2013 Update 4 (see [VS updates](https://support.microsoft.com/en-us/kb/2829760))
    * [Ninja](https://github.com/martine/ninja)
    * �޸� setenv.bat, �滻���е� python ·��
    * ���� VS2013 ����̨
    * �л�������Ŀ¼
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
