import os
import common

args = common.load_args()
monitor_file, label_file = common.load_files(args.taskType, args.taskNumber)

try:
    os.remove(monitor_file)
    print(f"deleted monitor: {args.taskType} {args.taskNumber}")
except:
    pass

try:
    os.remove(label_file)
    print(f"deleted label: {args.taskType} {args.taskNumber}")
except:
    pass
