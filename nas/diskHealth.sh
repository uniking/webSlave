#!/bin/bash

# 获取所有磁盘设备（不包括分区）
disks=$(ls /dev/sd* | grep -E '^/dev/sd[a-z]$')

# 对每个磁盘运行 smartctl 检查并判断状态
for disk in $disks; do
    result=$(smartctl -H $disk | grep 'SMART overall-health self-assessment test result')
    if echo "$result" | grep -q "PASSED"; then
        echo "$disk is ok"
    else
        echo "$disk is bad"
    fi
done
