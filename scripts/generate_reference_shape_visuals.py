#!/usr/bin/env python3
"""Render the real nought/cross reference point clouds used by ShapeSorting.

These .pcd files (binary PCD v0.7, fields x y z normal_x normal_y normal_z)
are the actual reference assets ShapeSorting/src/shape_sorting_solution
matches incoming scene point clouds against. No ROS, Gazebo, or the private
world-spawner packages are required to read and plot them -- this script
only depends on numpy and matplotlib.
"""

from __future__ import annotations

import struct
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
DATA_DIR = ROOT / "ShapeSorting" / "src" / "shape_sorting_solution" / "data"
OUT_PATH = ROOT / "docs" / "images" / "reference-shapes-pointcloud.png"

FIELD_COUNT = 6  # x, y, z, normal_x, normal_y, normal_z
POINT_STRUCT = struct.Struct("<6f")


def load_pcd_xyz(path: Path, max_points: int = 6000, seed: int = 0) -> np.ndarray:
    """Parse a binary PCD v0.7 file (x y z normal_x normal_y normal_z, float32)."""
    raw = path.read_bytes()
    marker = b"DATA binary\n"
    header_end = raw.find(marker)
    if header_end == -1:
        raise ValueError(f"{path}: expected 'DATA binary' PCD header")
    header = raw[:header_end].decode("ascii", errors="replace")

    points_line = next(line for line in header.splitlines() if line.startswith("POINTS"))
    n_points = int(points_line.split()[1])

    body = raw[header_end + len(marker) :]
    expected_bytes = n_points * POINT_STRUCT.size
    if len(body) < expected_bytes:
        raise ValueError(
            f"{path}: expected {expected_bytes} bytes of point data, got {len(body)}"
        )

    flat = np.frombuffer(body[:expected_bytes], dtype="<f4")
    points = flat.reshape(n_points, FIELD_COUNT)[:, :3]

    if n_points > max_points:
        rng = np.random.default_rng(seed)
        idx = rng.choice(n_points, size=max_points, replace=False)
        points = points[idx]

    return points, n_points


def plot_shape(ax, points: np.ndarray, title: str, n_total: int):
    # x/z form the shape's face plane; y is the ~40mm extrusion thickness,
    # so a front-on x-z scatter (colour-coded by y/depth) reveals the
    # nought/cross silhouette far more clearly than a generic 3D view.
    x, y, z = points[:, 0], points[:, 1], points[:, 2]
    scatter = ax.scatter(x, z, s=1.5, c=y, cmap="viridis")
    ax.set_title(f"{title}\n({len(points):,} of {n_total:,} points shown)", fontsize=11)
    ax.set_xlabel("x [m]")
    ax.set_ylabel("z [m]")
    ax.set_aspect("equal")
    return scatter


def main() -> None:
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)

    nought_points, nought_total = load_pcd_xyz(DATA_DIR / "nought_40mm.pcd")
    cross_points, cross_total = load_pcd_xyz(DATA_DIR / "cross_40mm.pcd")

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 5.2))

    plot_shape(ax1, nought_points, "nought_40mm.pcd reference cloud", nought_total)
    scatter2 = plot_shape(ax2, cross_points, "cross_40mm.pcd reference cloud", cross_total)
    fig.colorbar(scatter2, ax=[ax1, ax2], shrink=0.75, pad=0.02, label="y (extrusion depth) [m]")

    fig.suptitle(
        "ShapeSorting reference point clouds (real bundled data, no simulator required)",
        fontsize=12,
    )
    fig.savefig(OUT_PATH, dpi=150)
    plt.close(fig)
    print(f"wrote {OUT_PATH}")


if __name__ == "__main__":
    main()
