import subprocess
import common

args = common.load_args()
monitor_file, label_file = common.load_files(args.taskType, args.taskNumber)


subprocess.call(
    f"pio device monitor > {monitor_file}", shell=True)
