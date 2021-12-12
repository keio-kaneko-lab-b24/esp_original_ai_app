import pandas as pd
import numpy as np
import itertools

param_file = "./src/param_ml.h"


def calc_(t, y, a):
    pos_num = sum([_ == a for _ in t])
    all_num = sum([_ == a for _ in y])
    ans_num = len(t)
    recall = pos_num / all_num if all_num != 0 else 0
    precision = pos_num / ans_num if ans_num != 0 else 0

    sum_num = recall+precision
    f = 2*recall*precision/sum_num if sum_num != 0 else 0
    return recall, precision, f


def calc_score_threshold(X_max, y, rock_f_lower_threshold, rock_e_upper_threshold, paper_e_lower_threshold, paper_f_upper_threshold):

    t_rock = []
    t_paper = []
    for _x, _y in zip(X_max, y):
        e = _x[0]
        f = _x[1]
        if (f > rock_f_lower_threshold) & (e < rock_e_upper_threshold) & (e > paper_e_lower_threshold) & (f < paper_f_upper_threshold):
            continue
        if (f > rock_f_lower_threshold) & (e < rock_e_upper_threshold):
            t_rock.append(_y)
        if (e > paper_e_lower_threshold) & (f < paper_f_upper_threshold):
            t_paper.append(_y)

    # 性能評価
    recall_rock, precision_rock, f_rock = calc_(t_rock, y, "rock")
    recall_paper, precision_paper, f_paper = calc_(t_paper, y, "paper")

    return {
        "rock_f_lower_threshold": rock_f_lower_threshold,
        "rock_e_upper_threshold": rock_e_upper_threshold,
        "paper_e_lower_threshold": paper_e_lower_threshold,
        "paper_f_upper_threshold": paper_f_upper_threshold,
        "recall_rock": round(recall_rock, 2),
        "precision_rock": round(precision_rock, 2),
        "f_rock": round(f_rock, 2),
        "recall_paper": round(recall_paper, 2),
        "precision_paper": round(precision_paper, 2),
        "f_paper": round(f_paper, 2)
    }


def main(df_sp, max_score):
    # =======
    # 学習用データセットの準備
    # =======
    count_idx_list = df_sp.count_idx.unique()

    X = []
    y = []
    for count_idx in count_idx_list:
        _df = df_sp[df_sp["count_idx"] == count_idx]

        if len(_df) < 2:
            continue

        _df = _df[1:]  # 最初のひとつは前の試技の値が残る
        _x = _df[["extensor_sp", "flexor_sp"]].to_numpy()
        _y = _df["label"].to_numpy()[0]
        X.append(_x)
        y.append(_y)

    X = np.array(X)
    y = np.array(y)

    # =======
    # 学習
    # TODO: CNNに切り替え
    # =======

    # 各試技内での最大値を取得
    X_max = np.array([x.transpose(1, 0).max(axis=1) for x in X])

    extensor_range = np.arange(
        np.min(X_max[:, 0])*0.99, np.max(X_max[:, 0])+0.0011, 0.001)
    flexor_range = np.arange(
        np.min(X_max[:, 1])*0.99, np.max(X_max[:, 1])+0.0011, 0.001)

    # グーを最適化
    d = [calc_score_threshold(X_max, y, rock_f_lower_threshold, rock_e_upper_threshold, 0, 0)
         for rock_f_lower_threshold, rock_e_upper_threshold
         in itertools.product(flexor_range, extensor_range)]

    df = pd.DataFrame(d)

    df = df[df["f_rock"] >= df["f_rock"].max()]
    df = df[df["rock_e_upper_threshold"] == df["rock_e_upper_threshold"].min()]
    df = df[df["rock_f_lower_threshold"] == df["rock_f_lower_threshold"].max()]
    row = df.head(1)
    rock_f_lower_threshold = row["rock_f_lower_threshold"].tolist()[0]
    rock_e_upper_threshold = row["rock_e_upper_threshold"].tolist()[0]

    # パーを最適化
    d = [calc_score_threshold(X_max, y, rock_f_lower_threshold, rock_e_upper_threshold, paper_f_lower_threshold, paper_e_upper_threshold)
         for paper_f_lower_threshold, paper_e_upper_threshold
         in itertools.product(flexor_range, extensor_range)]

    df = pd.DataFrame(d)

    df["f_rock_paper"] = df.f_rock + df.f_paper

    # 精度を更新したときのみ，パラメータを更新する
    f_rock_paper_max = df["f_rock_paper"].max()
    if max_score >= f_rock_paper_max:
        return max_score, False

    df = df[df["f_rock_paper"] >= f_rock_paper_max]
    df = df[df["paper_f_upper_threshold"] ==
            df["paper_f_upper_threshold"].min()]
    df = df[df["paper_e_lower_threshold"] ==
            df["paper_e_lower_threshold"].max()]
    row = df.head(1)
    paper_e_lower_threshold = row["paper_e_lower_threshold"].tolist()[0]
    paper_f_upper_threshold = row["paper_f_upper_threshold"].tolist()[0]

    # 結果
    recall_rock = row['recall_rock'].to_list()[0]
    precision_rock = row['precision_rock'].to_list()[0]
    recall_paper = row['recall_paper'].to_list()[0]
    precision_paper = row['precision_paper'].to_list()[0]

    print("* 更新 *")
    print(f"rock:  recall={recall_rock}, precision={precision_rock}")
    print(f"paper: recall={recall_paper}, precision={precision_paper}")

    # 更新
    with open(param_file, "w") as f:
        f.write(
            f"const float rock_flexor_lower_limit = {rock_f_lower_threshold};\n")
        f.write(
            f"const float rock_extensor_upper_limit = {rock_e_upper_threshold};\n")
        f.write(
            f"const float paper_extensor_lower_limit = {paper_e_lower_threshold};\n")
        f.write(
            f"const float paper_flexor_upper_limit = {paper_f_upper_threshold};\n")

    return f_rock_paper_max, True


# 使用するデータ読み込み
df_sp = pd.read_json("./ml/dataset/sp.json")

df_sp["task_id"] = df_sp["task_name"] + df_sp["task_num"].apply(str)
task_ids = df_sp["task_id"].unique()

max_score, _ = main(df_sp, 0)

# もっとも精度が悪いファイルを除去していく
# 3ファイルまで
for i in range(2):

    drop_flag = False
    for task_id in task_ids:
        _df_sp = df_sp[df_sp["task_id"] != task_id]
        max_score, update = main(_df_sp, max_score)
        if update:
            drop_task_id = task_id
            drop_flag = True

    if drop_flag:
        df_sp = df_sp[df_sp["task_id"] != drop_task_id]

print("\nfinish")
