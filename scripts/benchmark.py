"""Reproducible process-level benchmark, including startup, input and JSON output."""
import argparse
import csv
import json
import platform
import random
import subprocess
import tempfile
import time
from pathlib import Path


def generate(size, seed):
    if size < 2:
        raise ValueError("size must be at least 2")
    rng = random.Random(seed)
    lines = [f"node n{i}" for i in range(size)]
    lines += [f"edge n{i} n{i + 1} {rng.randint(1, 20)}" for i in range(size - 1)]
    # A chain guarantees reachability; extra directed edges may include loops.
    lines += [f"edge n{rng.randrange(size)} n{rng.randrange(size)} {rng.randint(0, 20)}"
              for _ in range(size * 2)]
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("benchmark.csv"))
    parser.add_argument("--sizes", type=int, nargs="+", default=[50, 200, 1000])
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()
    if args.repeats < 1 or any(n < 2 for n in args.sizes):
        parser.error("repeats must be positive and sizes must be at least 2")
    binary = args.binary.resolve(strict=True)
    rows = []
    with tempfile.TemporaryDirectory() as directory:
        graph = Path(directory) / "graph.txt"
        for size in args.sizes:
            graph.write_text(generate(size, args.seed), encoding="utf-8")
            for algorithm in ("bfs", "dijkstra"):
                command = [str(binary), str(graph), "n0", f"n{size - 1}", algorithm]
                subprocess.run(command, check=True, capture_output=True, timeout=30)  # warm-up
                for repeat in range(args.repeats):
                    start = time.perf_counter_ns()
                    result = subprocess.run(command, check=True, capture_output=True, text=True, timeout=30)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                    data = json.loads(result.stdout)
                    if not data["reachable"]:
                        raise RuntimeError("Generated graph must be reachable")
                    rows.append(dict(nodes=size, edges=size * 3 - 1, seed=args.seed,
                                     algorithm=algorithm, repeat=repeat + 1,
                                     process_ms=round(elapsed, 4), objective=data["objective"],
                                     distance=data["distance"], system=platform.system(),
                                     machine=platform.machine(), python=platform.python_version()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(f"Wrote {len(rows)} measurements to {args.output}")


if __name__ == "__main__":
    main()
