#!/usr/bin/env python3
"""
用法：python3 gen_csv.py <帧ID>
示例：python3 gen_csv.py 1021

从 Model3_CH.dbc 提取指定帧的信号，结合 data/can_frames_decoded_all_values_mcu3.json
的枚举值，生成半成品 CSV（中文注释列留空，供人工补充）。
输出文件：<帧ID>.csv
"""

import sys
import re
import json
import csv
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
DBC_FILE   = SCRIPT_DIR / "Model3_CH.dbc"
JSON_FILE  = SCRIPT_DIR / "data" / "can_frames_decoded_all_values_mcu3.json"


def parse_dbc(frame_id: int):
    """从 DBC 文件中提取指定帧的所有信号。"""
    signals = []
    inside = False

    sig_re = re.compile(
        r'^\s+SG_\s+(\S+)\s+(M|m\d+)?\s*:\s*(\d+)\|(\d+)@\d+[+-]\s+\(([^,]+),([^)]+)\)'
    )

    with open(DBC_FILE, encoding="utf-8", errors="replace") as f:
        for line in f:
            if re.match(rf'^BO_\s+{frame_id}\s+', line):
                inside = True
                continue
            if inside:
                if line.startswith("BO_"):
                    break
                m = sig_re.match(line)
                if m:
                    name, mux_raw, start, length, factor, offset = m.groups()
                    if mux_raw is None:
                        mux = "-"
                    elif mux_raw == "M":
                        mux = "M"
                    else:
                        mux = mux_raw[1:]  # strip leading 'm'
                    signals.append({
                        "name":   name,
                        "mux":    mux,
                        "start":  int(start),
                        "length": int(length),
                        "factor": factor.strip(),
                        "offset": offset.strip(),
                    })
    return signals


def load_enums(frame_id: int) -> dict:
    """从 JSON 中加载指定帧的枚举值映射。"""
    with open(JSON_FILE, encoding="utf-8") as f:
        data = json.load(f)

    for frame in data.get("frames", []):
        if frame.get("address_dec") == frame_id:
            result = {}
            for sig in frame.get("signals", []):
                name = sig.get("signal_name", "")
                vals = sig.get("possible_values", [])
                if vals:
                    parts = [
                        f"{v['value_dec']}={v['label']}"
                        for v in vals
                        if v.get("label")
                    ]
                    if parts:
                        result[name] = "; ".join(parts)
            return result
    return {}


def sort_key(sig):
    mux_str = sig["mux"]
    if mux_str in ("M", "-"):
        m = -1
    else:
        m = int(mux_str)
    return (m, sig["start"])


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    frame_id = int(sys.argv[1])
    signals  = parse_dbc(frame_id)

    if not signals:
        print(f"未在 DBC 中找到帧 {frame_id}")
        sys.exit(1)

    enums = load_enums(frame_id)
    signals.sort(key=sort_key)

    out_path = SCRIPT_DIR / f"{frame_id}.csv"
    with open(out_path, "w", newline="", encoding="utf-8-sig") as f:
        w = csv.writer(f)
        w.writerow(["名称", "mux", "起始位", "长度", "可能的枚举值", "中文注释"])
        for s in signals:
            enum_str = enums.get(s["name"], "")
            # 如果有非默认因子/偏移，附加说明
            if s["factor"] != "1" or s["offset"] != "0":
                note = f"因子={s['factor']} 偏移={s['offset']}"
                enum_str = (enum_str + f"  [{note}]").strip()
            w.writerow([s["name"], s["mux"], s["start"], s["length"], enum_str, ""])

    print(f"已生成 {out_path}，共 {len(signals)} 个信号，中文注释列待补充。")


if __name__ == "__main__":
    main()
