# webSlave

现有设备， 8G内存的mini工控机， 5盘位硬盘柜

想写一个linux的service, 接收json指令，如执行mount, umount，sync等命令， 让web端更好的操作我的mini服务器


配置systemd

sudo systemctl daemon-reload

sudo systemctl enable webSlave.service

sudo systemctl start webSlave.service

