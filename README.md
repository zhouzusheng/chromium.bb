# minkit


* ����֧��ҪĿ�����޸�Bloomberg Chromium�� ɾ������Ҫ�Ĵ��룬���ٱ�������Ĵ����С��
* ɾ���Ĵ��������
    * ��ӡ֧��
    * PDF ���
    * HTML5 media, medea capture
    * DEVICE API (usb�� power�� etc.��
    * WEBRTC
    * WEBP
    * PPAPI������ NPAPI��
    * SANDBOX
    * remoting
    * safe_browsing
    * speech	
    * ����һЩС����	

## Build Instructions


* ׼�����뻷��:
    * [Python 2.7](https://www.python.org/download/releases/2.7.6/)
    * Visual Studio 2013 Update 4 (see [VS updates](https://support.microsoft.com/en-us/kb/2829760))
    * �޸� setenv.bat, �滻���е� python ·��
    * ���� VS2013 ����̨
    * �л�������Ŀ¼
    * setenv.bat    
    * src/build/runhooks

* �����б���:
            
            ���� VS2013 ����̨
            �л�������Ŀ¼
            setenv.bat
            ninja -C src/out/Debug     # for Debug builds
            ninja -C src/out/Release   # for Release builds

* �� Visual Studio ����:
            
            ���� VS2013 ����̨
            �л�������Ŀ¼
            setenv.bat
            devenv src\minkit\minkit.sln
            Build the `minkit_all` project.
    

---
###### Microsoft, Windows, Visual Studio and ClearType are registered trademarks of Microsoft Corp.
###### Firefox is a registered trademark of Mozilla Corp.
###### Chrome is a registered trademark of Google Inc.
