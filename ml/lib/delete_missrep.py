import pandas as pd

import pandas as pd

def _delete_missrep(sp, ratio):
    misscid = []

    # Rockではflexorに一定以上の動きがない試技を探索する
    sp_rock = sp[sp["label"]=="rock"]
    rock_diff = { 
        cid: sp_cid.flexor_sp.max() - sp_cid.flexor_sp.min()
        for cid, sp_cid in sp_rock.groupby("count_idx")}

    if len(rock_diff) >= 1:
        threshold = max(rock_diff.values()) * ratio
        misscid += [cid for cid, diff in rock_diff.items() if diff <= threshold]

    # Paperではextensorに一定以上の動きがない試技を探索する
    sp_paper = sp[sp["label"]=="paper"]
    paper_diff = { 
        cid: sp_cid.extensor_sp.max() - sp_cid.extensor_sp.min()
        for cid, sp_cid in sp_paper.groupby("count_idx")}

    if len(paper_diff) >= 1:
        threshold = max(paper_diff.values()) * ratio
        misscid += [cid for cid, diff in paper_diff.items() if diff <= threshold]

    # 削除する
    for cid in misscid:
        sp = sp[sp.count_idx!=cid]

    return sp
    
def delete_missrep(sp, ratio=0.2):
    """
    各試技においてRMSの動き（Max-Min)が小さいものはデータから除外する。
    Args:
        sp: make_dataset.pyで作成したRMSDataFrame。
        ratio: 除外試技を決める比率。各課題において、動き（Max-Min)が最大の試技を1として、ratio以下の試技を除外する。
    Returns:
        sp: 除外後のRMSDataFrame
    """
    return pd.concat([
        _delete_missrep(_sp, ratio) for _, _sp in sp.groupby(["task_name", "task_num"])
    ])