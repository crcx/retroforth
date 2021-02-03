# Benchmark Support
#
# This is intended to help support benchmarking by
# providing high resolution timing and instruction
# usage data.
#

import time


class BenchmarkDevice:
    def __init__(self):
        pass

    def execute(self, retro_instance, pointer):
        return 0

    def timer_resolution(self):
        try:
            time.time_ns()
            return 1
        except:
            return 0
