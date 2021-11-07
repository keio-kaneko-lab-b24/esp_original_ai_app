import subprocess
import asyncio
import time
import glob
import common

args = common.load_args()

async def view_task(task_number, task_type, flipImage):
    '''課題のステート画面を起動する'''
    print("start view_task")
    if flipImage:
        subprocess.Popen(
            f"python monitor/view_task.py --taskNumber {task_number} --taskType {task_type} --flipImage", shell=True)
    else:
        subprocess.Popen(
            f"python monitor/view_task.py --taskNumber {task_number} --taskType {task_type}", shell=True)
    print("end view_task")


async def view_monitor(task_number, task_type):
    '''筋電をモニターするための画面を起動する'''
    print("start view_monitor")
    subprocess.Popen(
        f"python monitor/view_monitor.py --taskNumber {task_number} --taskType {task_type}", shell=True)
    print("end view_monitor")


async def do_task(task_number, task_type):
    '''
    課題を実行する。
    '''
    print("start do_task")
    while True:
        try:
            subprocess.Popen(
                f"python monitor/do_task.py --taskNumber {task_number} --taskType {task_type}", shell=True)
            break
        except:
            print("wait for setting up...")
            time.sleep(1)

    print("end do_task")


async def do_stdio_task(task_number, task_type):
    '''
    標準入力での課題を実行する。
    '''
    print("start do_stdio_task")
    while True:
        try:
            subprocess.Popen(
                f"python monitor/do_stdio_task.py --taskNumber {task_number} --taskType {task_type}", shell=True)
            break
        except:
            print("wait for setting up...")
            time.sleep(1)

    print("end do_stdio_task")


async def do_monitor(task_number, task_type):
    '''
    ESPから信号処理後の筋電を継続的に取得して、monitor.txtへ書き込み続ける。
    '''
    print("start do_monitor")
    subprocess.call(
        f"python monitor/do_monitor.py --taskNumber {task_number} --taskType {task_type}", shell=True)
    print("end do_monitor")


def main():
    loop = asyncio.get_event_loop()

    task_type = args.taskType
    task_number = args.taskNumber

    # すでにファイルが存在していたらエラー
    exist_label_file = len(
        glob.glob(f"monitor/file/label/label_{task_type}_{task_number}.txt")) > 0
    exist_monitor_file = len(
        glob.glob(f"monitor/file/monitor/monitor_{task_type}_{task_number}.txt")) > 0
    if exist_label_file | exist_monitor_file:
        raise ValueError(
            f"label_{task_type}_{task_number}.txtか、monitor_{task_type}_{task_number}.txtが存在しています。")

    if task_type == "free":
        loop.run_until_complete(asyncio.gather(
            *[view_task(task_number, task_type, args.flipImage), view_monitor(task_number, task_type), do_stdio_task(task_number, task_type), do_monitor(task_number, task_type)]))
    else:
        loop.run_until_complete(asyncio.gather(
            *[view_task(task_number, task_type, args.flipImage), view_monitor(task_number, task_type), do_task(task_number, task_type), do_monitor(task_number, task_type)]))


if __name__ == '__main__':
    main()
