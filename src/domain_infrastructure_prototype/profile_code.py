import subprocess
import psutil
import time
import numpy as np
import pandas as pd
from datetime import datetime
import re

def find_index(line_split,target):
    found_index = -1
    for i, string in enumerate(line_split):
        if target in string:
            found_index = i
            break
    return found_index


def profile_code(name,profile_results_name,is_python,image_size_list,n_time_chunks_list,n_chan_chunks_list,sampling_interval,set_grid='false'):

    if is_python:
        process_name = "python " + name
    else:
        process_name = name

    memory_profiling_pd = pd.DataFrame()
    profiling_list = []
    
    memory_column_labels = ['date_time','total_compute_time','total_gridding_time','total_load_data_time','n_time_chunks','n_chan_chunks','image_size','rss','vms']
    column_labels = ['date_time','n_time_chunks','n_chan_chunks','image_size','total_compute_time','total_gridding_time','total_load_data_time','max_rss','max_vms']
    
    for n_time_chunks in n_time_chunks_list:
        for n_chan_chunks in n_chan_chunks_list:
            for image_size in image_size_list:
            
                if is_python:
                    if name=="main_pybind11.py":
                        print(' '.join(["python", name, "--image_size", str(image_size),"--n_time_chunks",str(n_time_chunks),"--n_chan_chunks",str(n_chan_chunks),"--set_grid",set_grid]))
                        sub_process = subprocess.Popen(["python", name, "--image_size", str(image_size),"--n_time_chunks",str(n_time_chunks),"--n_chan_chunks",str(n_chan_chunks),"--set_grid",set_grid], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    else:
                        print(' '.join(["python", name, "--image_size", str(image_size),"--n_time_chunks",str(n_time_chunks),"--n_chan_chunks",str(n_chan_chunks)]))
                        sub_process = subprocess.Popen(["python", name, "--image_size", str(image_size),"--n_time_chunks",str(n_time_chunks),"--n_chan_chunks",str(n_chan_chunks)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                else:
                #./bin/cpp_gridder 500 1 2
                    print(' '.join(["./bin/"+name, " ", str(image_size)," ",str(n_time_chunks)," ",str(n_chan_chunks)]))
                    sub_process = subprocess.Popen(["./bin/"+name,str(image_size),str(n_time_chunks),str(n_chan_chunks)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    
                pgrep_command = f"pgrep -f '{process_name}'"
                pid = subprocess.getoutput(pgrep_command)
                sub_process_psutil = psutil.Process(int(pid))

                current_datetime = datetime.now()
                rss = []
                vms = []
                pfaults = []
                pageins = []
                mem = []
                while sub_process.poll() is None:

                    try:
                        memory_info = sub_process_psutil.memory_info()
                        time.sleep(sampling_interval)
                        rss.append(memory_info.rss / (1024 * 1024 * 1024)) # GB
                        vms.append(memory_info.vms / (1024 * 1024 * 1024))
                    except:
                        process_done = True
            
                    
                    
                stdout, stderr = sub_process.communicate()
                #print(stdout)
                
                compute_time_str = str(stdout).split('\\n')[-2]

                print('*****',compute_time_str, compute_time_str[0], compute_time_str[2:])

                if compute_time_str[0] == 'b':
                    compute_time_str = compute_time_str[2:].split(' ')
                else:
                    compute_time_str = compute_time_str.split(' ')

                #print(compute_time_str)
                total_compute_time =  float(compute_time_str[0])
                total_gridding_time =  float(compute_time_str[1])
                total_data_load_time =  float(compute_time_str[2])


                max_rss = max(rss)
                max_vms = max(vms)


                profiling_list.append([current_datetime,n_time_chunks,n_chan_chunks,image_size,total_compute_time,total_gridding_time, total_data_load_time, max_rss,max_vms])
                #print([current_datetime,n_time_chunks,n_chan_chunks,image_size,total_compute_time, total_gridding_time, total_data_load_time, max_rss,max_vms])
                #print(len(rss),len(vms),len(pfaults),len(pageins))

                sub_memory_profiling_pd = pd.DataFrame({label: array for label, array in zip(memory_column_labels, [[current_datetime]*len(rss),[total_compute_time]*len(rss),[total_gridding_time]*len(rss), [total_data_load_time]*len(rss), [n_time_chunks]*len(rss),[n_chan_chunks]*len(rss),[image_size]*len(rss),rss,vms])})
                memory_profiling_pd = pd.concat([memory_profiling_pd, sub_memory_profiling_pd], ignore_index=True)


    profiling_pd = pd.DataFrame()
    #print(memory_profiling_pd)
    memory_profiling_pd.to_csv(profile_results_name+'_memory_profiling.xlsx',index=False)
    profiling_pd = pd.DataFrame(profiling_list, columns=column_labels)
    profiling_pd.to_excel(profile_results_name+'_profiling.xlsx',index=False)
    print(profiling_pd)
    print('*'*100)
  

if __name__ == "__main__":
    #[200,500,1000,1500,3000,5000]
    #profile_code(name="main_numba.py",profile_results_name="mac_numba",is_python=True,image_size_list=[200,500],n_time_chunks_list=[1,2],n_chan_chunks_list=[1,2],sampling_interval=1)

    #profile_code(name="main_pybind11.py",profile_results_name="mac_pybind11",is_python=True,image_size_list=[200,500],n_time_chunks_list=[1,2],n_chan_chunks_list=[1,2],sampling_interval=0.5)


#    profile_code(name="main_numba.py",profile_results_name="mac_numba",is_python=True,image_size_list=[200,500],n_time_chunks_list=[3],n_chan_chunks_list=[3],sampling_interval=0.5)
#
#    profile_code(name="main_pybind11.py",profile_results_name="mac_pybind11",is_python=True,image_size_list=[200,500],n_time_chunks_list=[3],n_chan_chunks_list=[3],sampling_interval=0.5)
    
    #image_size = [500, 1000, 1500, 2000, 2500, 5000]
#    image_size = [500, 1000, 1500, 2000, 2500, 5000]
#    n_chan_chunks_list = [1, 3, 5, 8, 10]
#    n_time_chunks_list = [1, 3, 5, 8, 10]
#
#
##    image_size = [500]
##    n_chan_chunks_list = [2]
##    n_time_chunks_list = [2]
#
#    profile_code(name="cpp_gridder",profile_results_name="linux_cpp",is_python=False,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
#    profile_code(name="main_numba.py",profile_results_name="linux_numba",is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
#    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11_cpp_grid",is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1,set_grid='false')
#    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11",is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1,set_grid='true')
#    profile_code(name="main_pybind11_cpp_only.py",profile_results_name="linux_pybind11_cpp_only",is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)


    image_size = [500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000]
    n_chan_chunks_list = [10]
    n_time_chunks_list = [10]
    
    type = "mem"
    
    profile_code(name="cpp_gridder",profile_results_name="linux_cpp_" + type,is_python=False,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
    profile_code(name="main_numba.py",profile_results_name="linux_numba_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11_cpp_grid_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1,set_grid='false')
    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1,set_grid='true')
    profile_code(name="main_pybind11_cpp_only.py",profile_results_name="linux_pybind11_cpp_only_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)


    image_size = [5000]
    n_chan_chunks_list = [10]
    n_time_chunks_list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    
    type = "compute"
    
    profile_code(name="cpp_gridder",profile_results_name="linux_cpp_" + type,is_python=False,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
    profile_code(name="main_numba.py",profile_results_name="linux_numba_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11_cpp_grid_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1,set_grid='false')
    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1,set_grid='true')
    profile_code(name="main_pybind11_cpp_only.py",profile_results_name="linux_pybind11_cpp_only_"+ type,is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)


    '''
    #htop -p $(pgrep -f "python main_numba.py") -d 1 -u --sort-key PERCENT_MEM --no-color > monitoring.txt
    #htop -p $(pgrep -f "python main_pybind11.py") -d 1 -u --sort-key PERCENT_MEM --no-color > monitoring.txt
    #htop -p $(pgrep -f "python main_numba.py") -d 1 -u --sort-key PERCENT_MEM --no-color > monitoring.txt
    #top | grep python
    #top -n 1 -b | grep "$(pgrep -f 'python main_numba.py')"
    time.sleep(100)
    '''




## Function to monitor memory usage
#def monitor_memory():
#    process = psutil.Process()
#    print(f"Memory Usage: {process.memory_info().rss} bytes")
#
#
#
#def display_top(snapshot, key_type='lineno', limit=3):
#    snapshot = snapshot.filter_traces((
#        tracemalloc.Filter(False, "<frozen importlib._bootstrap>"),
#        tracemalloc.Filter(False, "<unknown>"),
#    ))
#    top_stats = snapshot.statistics(key_type)
#
#    print("Top %s lines" % limit)
#    for index, stat in enumerate(top_stats[:limit], 1):
#        frame = stat.traceback[0]
#        # replace "/path/to/module/file.py" with "module/file.py"
#        filename = os.sep.join(frame.filename.split(os.sep)[-2:])
#        print("#%s: %s:%s: %.1f KiB"
#              % (index, filename, frame.lineno, stat.size / 1024))
#        line = linecache.getline(frame.filename, frame.lineno).strip()
#        if line:
#            print('    %s' % line)
#
#    other = top_stats[limit:]
#    if other:
#        size = sum(stat.size for stat in other)
#        print("%s other: %.1f KiB" % (len(other), size / 1024))
#    total = sum(stat.size for stat in top_stats)
#    print("Total allocated size: %.1f KiB" % (total / 1024))
#



#valgrind --leak-check=full --show-leak-kinds=all --without-pymalloc python main_pybind11.py
#valgrind python main_pybind11.py




'''
htop -p $(pgrep -f "python main_pybind11.py") -d 1 -u --sort-key PERCENT_MEM --no-color > monitoring.txt
'''
