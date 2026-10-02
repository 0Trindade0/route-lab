import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BINARY = Path(os.environ.get("ROUTE_LAB_BINARY", ROOT / "build" / ("route_lab.exe" if os.name == "nt" else "route_lab"))).resolve()


class CliTests(unittest.TestCase):
    def run_cli(self, *args):
        return subprocess.run([str(BINARY), *map(str, args)], capture_output=True, text=True, timeout=10)

    def test_weighted_route(self):
        result = self.run_cli(ROOT / "examples/network.txt", "A", "D", "dijkstra")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout)["path"], ["A", "C", "E", "D"])

    def test_bfs_objective(self):
        data = json.loads(self.run_cli(ROOT / "examples/network.txt", "A", "D", "bfs").stdout)
        self.assertEqual((data["objective"], data["distance"]), ("hops", 2))

    def test_unreachable(self):
        data = json.loads(self.run_cli(ROOT / "examples/network.txt", "A", "isolated", "bfs").stdout)
        self.assertFalse(data["reachable"])
        self.assertIsNone(data["distance"])

    def test_missing_file(self):
        self.assertEqual(self.run_cli("does-not-exist", "A", "D", "bfs").returncode, 2)

    def test_invalid_algorithm(self):
        self.assertEqual(self.run_cli(ROOT / "examples/network.txt", "A", "D", "invalid").returncode, 2)

    def test_invalid_graph(self):
        with tempfile.TemporaryDirectory() as folder:
            file = Path(folder) / "bad.txt"
            file.write_text("node A\nedge A A -1\n", encoding="utf-8")
            result = self.run_cli(file, "A", "A", "bfs")
            self.assertEqual(result.returncode, 2)
            self.assertIn("Line 2", result.stderr)

    def test_help(self):
        self.assertEqual(self.run_cli("--help").returncode, 0)


if __name__ == "__main__":
    unittest.main()
