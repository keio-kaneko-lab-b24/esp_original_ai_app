import argparse


def load_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--taskNumber', type=int, default=0)
    parser.add_argument('--taskType', type=str)
    parser.add_argument('--flipImage', action='store_true')
    return parser.parse_args()


def load_files(task_type, task_number):
    monitor_file = f"monitor/file/monitor/monitor_{task_type}_{task_number}.txt"
    label_file = f"monitor/file/label/label_{task_type}_{task_number}.txt"
    return monitor_file, label_file
