import pandas as pd
from scipy.signal import find_peaks


def _delete_spike(sp, distance):
    sp = sp.reset_index(drop=True)

    extensor_peaks, _ = find_peaks(sp.extensor_sp, distance=distance)
    for ep in extensor_peaks:
        try:
            sp.loc[ep, "extensor_sp"] = (
                sp.extensor_sp.iloc[ep-1] + sp.extensor_sp.iloc[ep+1]) / 2
        except:
            pass

    flexor_peaks, _ = find_peaks(sp.flexor_sp, distance=distance)
    for ep in flexor_peaks:
        try:
            sp.loc[ep, "flexor_sp"] = (
                sp.flexor_sp.iloc[ep-1] + sp.flexor_sp.iloc[ep+1]) / 2
        except:
            pass

    return sp


def delete_spike(sp, distance=1):
    """
    スパイクを除外する。
    Args:
        sp: make_dataset.pyで作成したRMSDataFrame。
        distance: scipy.signal.find_peaksのdistanceパラメータ
    Returns:
        sp: スパイク除外後のRMSDataFrame。
    """
    return pd.concat([
        _delete_spike(_sp, distance) for _, _sp in sp.groupby(["task_name", "task_num"])
    ])
