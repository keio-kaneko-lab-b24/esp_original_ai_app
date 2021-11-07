# =====================
# 標準入力によるタスク
# =====================

import re
from define_task import define_task
import common

args = common.load_args()
monitor_file, label_file = common.load_files(args.taskType, args.taskNumber)


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
        try:
            f.write(f"{t}: {instruction}\n")
        except:
            return False

    # コマンドラインへ表示
    print(f"指示-> {instruction}をしてください。\n")

    return True


# 標準入力を受け付ける
while True:
    str = input("入力してください[r:rock, p:paper, n:rest, q:quit]\n")
    print(f"入力: {str}")
    if str == "r":
        result = instruct("rock")
    elif str == "p":
        result = instruct("paper")
    elif str == "n":
        result = instruct("rest")
    elif str == "q":
        print("*** 入力を終了します ***")
        break

    if not result:
        print("!! 入力がありません。筋電計が接続されていることを確認してください。 !!")
