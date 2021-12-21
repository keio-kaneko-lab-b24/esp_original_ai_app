import pandas as pd
import os
import shutil

import plotly.graph_objects as go


def get_timing(sp):
    # グーのタイミング
    # 例：[(123373, 126370),...]
    sp_rock = sp[sp["label"] == "rock"]
    sp_rock_t = sp_rock.groupby(["count_idx"])["time"]
    rock_times = list(zip(sp_rock_t.first().values, sp_rock_t.last().values))

    # パーのタイミング
    # 例：[(123373, 126370),...]
    sp_paper = sp[sp["label"] == "paper"]
    sp_paper_t = sp_paper.groupby(["count_idx"])["time"]
    paper_times = list(
        zip(sp_paper_t.first().values, sp_paper_t.last().values))

    return rock_times, paper_times


def show_plot(sp):
    task_name = sp["task_name"].iloc[0]
    task_num = sp["task_num"].iloc[0]

    fig = go.Figure()

    fig.add_trace(go.Scatter(x=sp["time"], y=sp["extensor_sp"],
                             mode='lines',
                             name='extensor'))

    fig.add_trace(go.Scatter(x=sp["time"], y=sp["flexor_sp"],
                             mode='lines',
                             name='flexor'))

    #　グーとパーのタイミングを可視化
    rock_times, paper_times = get_timing(sp)

    for rock_t in rock_times:
        fig.add_vrect(
            x0=rock_t[0], x1=rock_t[1],
            fillcolor="LightSalmon", opacity=0.5,
            layer="below", line_width=0,
        )

    for paper_t in paper_times:
        fig.add_vrect(
            x0=paper_t[0], x1=paper_t[1],
            fillcolor="lightBlue", opacity=0.6,
            layer="below", line_width=0,
        )

    fig.update_layout(
        title=task_name,
        xaxis_title="time[s] (端末経過時間)",
        yaxis_title="RMS値[mV]",
        legend_title="筋電",
        font=dict(
            size=12
        ),
        width=1000,
        height=500,
    )

    os.makedirs(f"./ml/dataset/images", exist_ok=True)
    fig.write_image(f"./ml/dataset/images/{task_name}_{task_num}.png")


dirpath = os.path.join('ml', 'dataset', 'images')
if os.path.exists(dirpath) and os.path.isdir(dirpath):
    shutil.rmtree('./ml/dataset/images')

sp_all = pd.read_json(f"./ml/dataset/sp.json")
for _, sp in sp_all.groupby(["task_name", "task_num"]):
    show_plot(sp)
