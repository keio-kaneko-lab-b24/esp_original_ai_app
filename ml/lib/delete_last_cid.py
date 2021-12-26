import pandas as pd

def _delete_last_cid(sp):
    last_cid = sp.count_idx.to_list()[-1]
    return sp[sp.count_idx != last_cid]
    
def delete_last_cid(sp):
    return pd.concat([
        _delete_last_cid(_sp) for _, _sp in sp.groupby(["task_name", "task_num"])
    ])