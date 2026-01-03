#!/bin/bash
LOG_FILE="app.log"
chmod +x log_generator.sh
echo "开始模拟日志生成... (写入到 $LOG_FILE)"

while true; do
    # 生成当前时间
    TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")

    # 随机生成一种日志等级
    LEVELS=("INFO" "WARNING" "ERROR")
    RANDOM_INDEX=$((RANDOM % 3))
    LEVEL=${LEVELS[$RANDOM_INDEX]}

    # 写入文件
    echo "[$TIMESTAMP] [$LEVEL] User action detected in module payment-service." >> $LOG_FILE

    # 打印到屏幕让我们看见
    echo "写入: [$TIMESTAMP] [$LEVEL] ..."

    # 随机等待 2-4 秒
    sleep $(( ( RANDOM % 3 ) + 2 ))
done