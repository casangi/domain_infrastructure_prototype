
import tracemalloc
import os
import linecache
import numpy as np
import time


def main():

    import bin.pybind11_wrapper as py11
    gridder = py11.single_cf_gridder_pybind()
    
    tracemalloc.start()
    start = time.time()
    array = np.random.rand(100000000)
    gridder.increment_array(array)
    snapshot = tracemalloc.take_snapshot()
    display_top(snapshot)
    tracemalloc.stop()

def display_top(snapshot, key_type='lineno', limit=3):
    snapshot = snapshot.filter_traces((
        tracemalloc.Filter(False, "<frozen importlib._bootstrap>"),
        tracemalloc.Filter(False, "<unknown>"),
    ))
    top_stats = snapshot.statistics(key_type)

    print("Top %s lines" % limit)
    for index, stat in enumerate(top_stats[:limit], 1):
        frame = stat.traceback[0]
        # replace "/path/to/module/file.py" with "module/file.py"
        filename = os.sep.join(frame.filename.split(os.sep)[-2:])
        print("#%s: %s:%s: %.1f KiB"
              % (index, filename, frame.lineno, stat.size / 1024))
        line = linecache.getline(frame.filename, frame.lineno).strip()
        if line:
            print('    %s' % line)

    other = top_stats[limit:]
    if other:
        size = sum(stat.size for stat in other)
        print("%s other: %.1f KiB" % (len(other), size / 1024))
    total = sum(stat.size for stat in top_stats)
    print("Total allocated size: %.1f KiB" % (total / 1024))
    

if __name__=="__main__":
    main()
    
    
    


'''
c++ -O3 -Wall -shared -std=c++14 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/lib/python3.8/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix)

'''

