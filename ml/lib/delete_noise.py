import pandas as pd
from dataclasses import dataclass

@dataclass(frozen=True)
class RestingStats:
    extensor_std: float
    extensor_mean: float
    flexor_mean: float
    flexor_std: float


def _get_resting_stats(sp):
    resting_state = sp[sp.count_idx==sp.count_idx.min()]
    label_unique = resting_state.label.unique()
    if label_unique[0]=="rest" and len(label_unique)==1:
        return {
            "extensor_std": resting_state["extensor_sp"].std(),
            "flexor_std": resting_state["flexor_sp"].std(),
            "extensor_mean": resting_state["extensor_sp"].mean(),
            "flexor_mean": resting_state["flexor_sp"].mean()
        }
    else:
        return {}

def get_resting_stats(sp):
    """
    RMSDataFrameからRestingStatsを作成する。
    - 各課題における最初のRestデータを使用してRestingStatsを計算する
    - 複数の課題がある場合は、最もstdが低いものをRestingStatsとして使用
    Args:
        sp: make_dataset.pyで作成したRMSDataFrame。
    Returns:
        resting_stats: RestingStats
    """
    resting_stats = pd.DataFrame([
        _get_resting_stats(_sp) for _, _sp in sp.groupby(["task_name", "task_num"])
    ])
    extesor_rs = resting_stats.loc[resting_stats.extensor_std.argmin()]
    flexor_rs = resting_stats.loc[resting_stats.flexor_std.argmin()]
    extensor_rs_std, extensor_rs_mean = extesor_rs["extensor_std"], extesor_rs["extensor_mean"]
    flexor_rs_std, flexor_rs_mean = flexor_rs["flexor_std"], flexor_rs["flexor_mean"]

    return RestingStats(
        extensor_std = extensor_rs_std,
        flexor_std = extensor_rs_mean,
        extensor_mean =  flexor_rs_std,
        flexor_mean =  flexor_rs_mean
    )

def denoise(sp, resting_stats: RestingStats, std_weight = 0.5):
    """
    RestingStatsを使用して安静時状態のノイズを除去する。
    Args:
        sp: make_dataset.pyで作成したRMSDataFrame。
        resting_stats: ノイズ除去に使用するRestingStats。
        std_weight: ノイズ除去の係数。「平均値 ± std_weight*SD」にある筋電をノイズとして除去する。
    Returns:
        sp: ノイズ除去後のRMSDataFrame
        resting_stats: 使用した
    """
    extensor_std = resting_stats.extensor_std
    extensor_mean = resting_stats.extensor_mean
    flexor_std = resting_stats.flexor_std
    flexor_mean = resting_stats.flexor_mean
    sp.extensor_sp = sp.extensor_sp.apply(lambda x: extensor_mean if abs(x - extensor_mean) < extensor_std * std_weight else x)
    sp.flexor_sp = sp.flexor_sp.apply(lambda x: flexor_mean if abs(x - flexor_mean) < flexor_std * std_weight else x)    
    return sp