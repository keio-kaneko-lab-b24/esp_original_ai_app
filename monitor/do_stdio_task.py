# =====================
# 標準入力によるタスク
# =====================

import os
import re
import argparse
import time
import pygame.mixer
import json
from define_task import define_task

# do_monitor, do_task, do_stdio_task, view_monitor, view_stateで共通
parser = argparse.ArgumentParser()
parser.add_argument('--taskNumber', type=int, default=0)
parser.add_argument('--taskType', type=str)
args = parser.parse_args()

monitor_file = f"monitor/file/monitor/monitor_{args.taskType}_{args.taskNumber}.txt"
label_file = f"monitor/file/label/label_{args.taskType}_{args.taskNumber}.txt"
# ここまで


def instruct(instruction):
    '''1つの指示（グー/パー/レスト）を教師ラベルとしてlabel.txtに追記する'''

    # monitor読み込み
    # 10ms以下で完了できる

    with open(monitor_file, "r") as f:
        for line in f.readlines()[::-1]:
            if "time: " in line:
                t = int(re.sub("[^0-9]", "", line))
                break

    # 教師として書き込み
    with open(label_file, "a") as f:
        f.write(f"{t}: {instruction}\n")

    # コマンドラインへ表示
    print(f"指示-> {instruction}をしてください。\n")

    return True


# 標準入力を受け付ける
while True:
    str = input("入力してください[r:rock, p:paper, n:rest, q:quit]\n")
    print(f"入力: {str}")
    if str == "r":
        instruct("rock")
    elif str == "p":
        instruct("paper")
    elif str == "n":
        instruct("rest")
    elif str == "q":
        print("*** 入力を終了します ***")
        break
