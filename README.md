# webSlave

90%以上代码由ChatGPT生成， 仅根据自己需要手动增加了部分指令。

现有设备， 8G内存的mini工控机， 5盘位硬盘柜

想写一个linux的service, 接收json指令，如执行mount, umount，sync等命令， 让web端更好的操作我的mini服务器


配置systemd，/etc/systemd/system/webSlave.service

sudo systemctl daemon-reload

sudo systemctl enable webSlave.service

sudo systemctl start webSlave.service


通过web页面或android手机操作mini工控机

1, 系统维护：关机，重启， 更新

2, 磁盘维护：挂载， 卸载， 磁盘容量， 磁盘健康

3, docker维护：停止， 启动， 重启

4, 数据维护：备份磁盘（只有两块盘， 一块平时使用， 一块手动备份）
