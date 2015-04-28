# NginxLoader

让 `nginx` 以 `Windows` 的服务形式运行.


将本程序存放至 `nginx.exe` 同一目录内, 使用方法请参考下方实例:


```text
T:\nginx-1.8.0>curl http://127.0.0.1
curl: (7) couldn't connect to host

T:\nginx-1.8.0>sc query nginx
[SC] EnumQueryServicesStatus:OpenService 失败 1060:

指定的服务未安装。


T:\nginx-1.8.0>NginxLoader.exe /install

T:\nginx-1.8.0>net start nginx
Nginx 服务正在启动 .
Nginx 服务已经启动成功。


T:\nginx-1.8.0>sc query nginx

SERVICE_NAME: nginx
        TYPE               : 10  WIN32_OWN_PROCESS
        STATE              : 4  RUNNING
                                (STOPPABLE, NOT_PAUSABLE, IGNORES_SHUTDOWN)
        WIN32_EXIT_CODE    : 0  (0x0)
        SERVICE_EXIT_CODE  : 0  (0x0)
        CHECKPOINT         : 0x0
        WAIT_HINT          : 0x0

T:\nginx-1.8.0>curl http://127.0.0.1
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
    body {
        width: 35em;
        margin: 0 auto;
        font-family: Tahoma, Verdana, Arial, sans-serif;
    }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>

T:\nginx-1.8.0>net stop nginx
Nginx 服务正在停止.
Nginx 服务已成功停止。


T:\nginx-1.8.0>curl http://127.0.0.1
curl: (7) couldn't connect to host

T:\nginx-1.8.0>sc query nginx

SERVICE_NAME: nginx
        TYPE               : 10  WIN32_OWN_PROCESS
        STATE              : 1  STOPPED
        WIN32_EXIT_CODE    : 0  (0x0)
        SERVICE_EXIT_CODE  : 0  (0x0)
        CHECKPOINT         : 0x0
        WAIT_HINT          : 0x0

T:\nginx-1.8.0>NginxLoader.exe /uninstall

T:\nginx-1.8.0>sc query nginx
[SC] EnumQueryServicesStatus:OpenService 失败 1060:

指定的服务未安装。

```


代码基于 [VC知识库: 用 VC++建立 Windows 服务程序](http://www.vckbase.com/index.php/wv/1391) 中的源代码, 感谢 [李佳颖](http://www.vckbase.com/index.php/p/110018) 的无私分享精神.

