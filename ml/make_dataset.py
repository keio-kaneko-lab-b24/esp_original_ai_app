'''学習のためのデータセットを作成する関数'''

import argparse
import os
import re
import glob
import pandas as pd

from lib.delete_noise import get_resting_stats, denoise
from lib.delete_missrep import delete_missrep
from lib.delete_spike import delete_spike
from lib.delete_last_cid import delete_last_cid

# パラメータ
parser = argparse.ArgumentParser()
parser.add_argument('--useTask', nargs='*')
parser.add_argument('--test', action='store_true')
args = parser.parse_args()
useTask = args.useTask

# ファイルパス
label_file_path = f"{os.path.dirname(__file__)}/../monitor/file/label/"
monitor_file_path = f"{os.path.dirname(__file__)}/../monitor/file/monitor/"
dataset_file_path = f"{os.path.dirname(__file__)}/dataset/"


def fix_times(times):
    # label.txtが故障している場合の修復
    # https://github.com/tmizuguc/esp_app/issues/8
    fixed_times = times.copy()
    for t_i, t in enumerate(times):
        if t_i == 0:
            continue
        if t < times[t_i - 1]:
            fixed_times[t_i] = times[t_i - 1] + 3000
    return fixed_times


def load_label_data(task_name, task_num, label_file_path):
    '''labelファイルから教示ラベルを読み込む'''
    try:
        with open(label_file_path + f"label_{task_name}_{task_num}.txt", "r") as f:
            times = []
            labels = []

            for line in f.readlines():
                gr = re.search("^([0-9]+): ([a-z]+)\n", line)
                if gr:
                    times.append(int(gr.group(1)))
                    labels.append(gr.group(2))
    except:
        raise FileNotFoundError

    # label.txtが故障している場合の修復
    # https://github.com/tmizuguc/esp_app/issues/8
    times = fix_times(times)

    df_label = pd.DataFrame({"time": times, "label": labels})

    return df_label


def load_emg_sp_data(task_name, task_num, monitor_file_path):
    '''monotorファイルからRMS筋電を読み込む'''
    try:
        with open(monitor_file_path + f"monitor_{task_name}_{task_num}.txt", "r") as f:
            lines = f.readlines()
    except:
        raise FileNotFoundError

    _time = False
    _extensor_sp = False
    _flexor_sp = False
    times = []
    extensor_sps = []
    flexor_sps = []

    for line in lines:
        if "time:" in line:
            # 通常は「time: 12505」のようになっているが，
            # 稀に「out1 = 0, out2 = 0;time: 120585」のようになっている場合があるのでre.matchで対応
            _time = int(re.match(".*time:\ ?([.0-9]+)", line)[1])

        mc = re.match("^(e_sp): ([+-]?\\d+(?:\\.\\d+)?)\n", line)
        if mc:
            if mc[1] == "e_sp":
                _extensor_sp = float(mc[2])
        mc = re.match("^(f_sp): ([+-]?\\d+(?:\\.\\d+)?)\n", line)
        if mc:
            if mc[1] == "f_sp":
                _flexor_sp = float(mc[2])

        if (_time is not False) & (_extensor_sp is not False) & (_flexor_sp is not False):
            times.append(_time)
            extensor_sps.append(_extensor_sp)
            flexor_sps.append(_flexor_sp)

            _time = False
            _extensor_sp = False
            _flexor_sp = False

    df_emg_sp = pd.DataFrame(
        {"time": times, "extensor_sp": extensor_sps, "flexor_sp": flexor_sps})

    return df_emg_sp


def load_sp_dataset(task_name, task_num, monitor_file_path, label_file_path):
    '''label, monotorファイルから，教示信号とRMS筋電のデータセットを作成する'''
    df_label = load_label_data(task_name, task_num, label_file_path)
    df_emg_sp = load_emg_sp_data(task_name, task_num, monitor_file_path)

    _df = pd.merge_asof(df_emg_sp, df_label, on="time",
                        direction="backward").dropna()

    _df["task_name"] = task_name
    _df["task_num"] = task_num

    # 整理
    _df = _df[["task_name", "task_num", "label",
               "extensor_sp", "flexor_sp", "time"]]
    return _df


def get_task_num(task_name, monitor_file_path, label_file_path):
    '''使用可能なtask_num一覧を取得する'''
    label_files = glob.glob(f"{label_file_path}/*{task_name}_[0-9]*")
    label_num = [int(re.match(
        f".+label_{task_name}_(.+).txt", label_file)[1]) for label_file in label_files]

    monitor_files = glob.glob(f"{monitor_file_path}/*{task_name}_[0-9]**")
    monitor_num = [int(re.match(
        f".+monitor_{task_name}_(.+).txt", monitor_file)[1]) for monitor_file in monitor_files]

    return list(set(label_num) & set(monitor_num))


def add_count_idx(df):
    '''データセットに教示番号（count_idx）を付与する'''
    df = df.reset_index(drop=True)

    last_label = None
    count_idx = -1
    count_idx_list = []
    for i in df.index:
        row = df.iloc[i]
        label = row["label"]
        if label == last_label:
            count_idx_list.append(count_idx)
            pass
        else:
            last_label = label
            count_idx += 1
            count_idx_list.append(count_idx)

    df["count_idx"] = count_idx_list
    return df


def make_dataset(useTask, monitor_file_path, label_file_path):
    '''データセットを作成するMain関数'''
    _sp_list = []
    for task_name in useTask:

        task_num_list = get_task_num(
            task_name, monitor_file_path, label_file_path)
        for task_num in task_num_list:
            _sp = load_sp_dataset(task_name, task_num,
                                  monitor_file_path, label_file_path)
            _sp_list.append(_sp)
            print(f"use: {task_name} {task_num}")

    sp = pd.concat(_sp_list)
    sp = add_count_idx(sp)
    return sp


# データセット作成
sp = make_dataset(useTask, monitor_file_path, label_file_path)
sp.to_json(dataset_file_path + "sp.json", orient='records')

# 前処理
# ノイズ除去
resting_stats = get_resting_stats(sp)
sp = denoise(sp, resting_stats, std_weight=0.5)
print(resting_stats)
# スパイク除去
sp = delete_spike(sp, distance=1)
# 不要な学習データ除去
sp = delete_missrep(sp, ratio=0.3)
# 最後の試技（課題後のrest）を削除
sp = delete_last_cid(sp)
sp.to_json(dataset_file_path + "sp_processed.json", orient='records')
