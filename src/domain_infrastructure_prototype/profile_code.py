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


def profile_code(name,profile_results_name,is_python,image_size_list,n_time_chunks_list,n_chan_chunks_list,sampling_interval):

    if is_python:
        process_name = "python " + name
    else:
        process_name = name

    memory_profiling_pd = pd.DataFrame()
    profiling_list = []
    
    memory_column_labels = ['date_time','total_compute_time','total_gridding_time','total_load_data_time','n_time_chunks','n_chan_chunks','image_size','rss','vms','mem']
    column_labels = ['date_time','n_time_chunks','n_chan_chunks','image_size','total_compute_time','total_gridding_time','total_load_data_time','max_rss','max_vms','max_mem']
    
    for n_time_chunks in n_time_chunks_list:
        for n_chan_chunks in n_chan_chunks_list:
            for image_size in image_size_list:
                print(n_time_chunks,n_chan_chunks,image_size)
            
                if is_python:
                    print(' '.join(["python", name, "--image_size", str(image_size),"--n_time_chunks",str(n_time_chunks),"--n_chan_chunks",str(n_chan_chunks)]))
                    sub_process = subprocess.Popen(["python", name, "--image_size", str(image_size),"--n_time_chunks",str(n_time_chunks),"--n_chan_chunks",str(n_chan_chunks)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                else:
                #./bin/cpp_gridder 500 1 2
                    print(' '.join(["./bin/"+name, " ", str(image_size)," ",str(n_time_chunks)," ",str(n_chan_chunks)]))
                    sub_process = subprocess.Popen(["./bin/"+name,str(image_size),str(n_time_chunks),str(n_chan_chunks)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    
                pgrep_command = f"pgrep -f '{process_name}'"
                pid = subprocess.getoutput(pgrep_command)
                print('The ' + name +' pid',pid)
                sub_process_psutil = psutil.Process(int(pid))

                current_datetime = datetime.now()
                rss = []
                vms = []
                pfaults = []
                pageins = []
                mem = []
                while sub_process.poll() is None:

                    try:
                        #if True:
                        # Print the memory usage
                        memory_info = sub_process_psutil.memory_info()
                        #print(memory_info)
                        
                        mem_measurement = 0
                        
#                        start = time.time()
#                        sub_process_top = subprocess.Popen(["top","-l 2"], stdout=subprocess.PIPE, stderr=subprocess.PIPE) #,"-pid "+pid
                        time.sleep(sampling_interval)
#                        stdout_top, stderr_top = sub_process_top.communicate()
#                        sub_process_top.terminate()
#                        #print('1 Time ',time.time()-start)
#
#                        start = time.time()
#                        top_lines = str(stdout_top).split('\\n')
#
#
#                        #print(top_lines[0:30])
#                        for line in top_lines:
#                            if 'PID' in line:
#                                line_split = line.split(' ')
#
#                                found_index = -1
#                                for l in line_split:
#                                    if (l != '') and (l != ' '):
#                                        found_index = found_index+1
#                                    if l == 'MEM':
#                                        break
#
#                            if pid in line:
#                                #print('*'*10)
#                                #print(line)
#                                #print('*'*10)
#                                line_split = line.split(' ')
#                                index = 0
#                                for l in line_split:
#                                    if (l != '') and (l != ' ') :
#                                        if index==found_index:
#                                            mem_str = l
#                                            break
#                                        index = index+1
#
#                                #print('mem_str',mem_str)
#                                mem_units = ''.join([char for char in mem_str if char.isalpha()])
#                                #print('**',''.join([char for char in mem_str if char.isdigit()]))
#                                mem_digit = float(''.join([char for char in mem_str if char.isdigit()]))
#
#                                if 'B' in mem_units:
#                                    mem_temp = mem_digit/(1024*1024*1024)
#                                elif 'K' in mem_units:
#                                    mem_temp = mem_digit/(1024*1024)
#                                elif 'M' in mem_units:
#                                    mem_temp = mem_digit/(1024)
#                                else:
#                                    mem_temp = mem_digit
#
#                                #print("@@@",mem_measurement, mem_temp)
#                                if mem_measurement < mem_temp:
#                                    mem_measurement = mem_temp
#
#                                #print(mem_measurement,mem_str,mem_units,mem_digit)
#
#                        #print('2 Time ',time.time()-start)
                        
                        mem.append(mem_measurement)
                        rss.append(memory_info.rss / (1024 * 1024 * 1024)) # GB
                        vms.append(memory_info.vms / (1024 * 1024 * 1024))
                        #pfaults.append(memory_info.pfaults)
                        #pageins.append(memory_info.pageins)
                        #print('^&*^&&^*',mem_measurement,memory_info.rss / (1024 * 1024 * 1024),memory_info.vms / (1024 * 1024 * 1024))
                        #print('\n')
                        #print(memory_info)
                        
                    except:
                        process_done = True
            
                    
                    
                stdout, stderr = sub_process.communicate()
                compute_time_str = str(stdout).split('\\n')[-2]

                #print('&&&&&&&&&',stdout)
                #print('*****',compute_time_str, compute_time_str[0], compute_time_str[2:])

                if compute_time_str[0] == 'b':
                    compute_time_str = compute_time_str[2:].split(' ')
                else:
                    compute_time_str = compute_time_str.split(' ')

                total_compute_time =  float(compute_time_str[0])
                total_gridding_time =  float(compute_time_str[1])
                total_data_load_time =  float(compute_time_str[2])


                max_rss = max(rss)
                max_vms = max(vms)
                max_mem = max(mem)

                profiling_list.append([current_datetime,n_time_chunks,n_chan_chunks,image_size,total_compute_time,total_gridding_time, total_data_load_time, max_rss,max_vms, max_mem])
                print([current_datetime,n_time_chunks,n_chan_chunks,image_size,total_compute_time, total_gridding_time, total_data_load_time, max_rss,max_vms,max_mem])
                #print(len(rss),len(vms),len(mem),len(pfaults),len(pageins))

                sub_memory_profiling_pd = pd.DataFrame({label: array for label, array in zip(memory_column_labels, [[current_datetime]*len(rss),[total_compute_time]*len(rss),[total_gridding_time]*len(rss), [total_data_load_time]*len(rss), [n_time_chunks]*len(rss),[n_chan_chunks]*len(rss),[image_size]*len(rss),rss,vms,mem])})
                memory_profiling_pd = pd.concat([memory_profiling_pd, sub_memory_profiling_pd], ignore_index=True)


    profiling_pd = pd.DataFrame()
    print(memory_profiling_pd)
    memory_profiling_pd.to_csv(profile_results_name+'_memory_profiling.xlsx',index=False)
    profiling_pd = pd.DataFrame(profiling_list, columns=column_labels)
    profiling_pd.to_excel(profile_results_name+'_memory_profiling.xlsx',index=False)
    print('*'*100)
    print(profiling_pd)

if __name__ == "__main__":
    #[200,500,1000,1500,3000,5000]
    #profile_code(name="main_numba.py",profile_results_name="mac_numba",is_python=True,image_size_list=[200,500],n_time_chunks_list=[1,2],n_chan_chunks_list=[1,2],sampling_interval=1)

    #profile_code(name="main_pybind11.py",profile_results_name="mac_pybind11",is_python=True,image_size_list=[200,500],n_time_chunks_list=[1,2],n_chan_chunks_list=[1,2],sampling_interval=0.5)


#    profile_code(name="main_numba.py",profile_results_name="mac_numba",is_python=True,image_size_list=[200,500],n_time_chunks_list=[3],n_chan_chunks_list=[3],sampling_interval=0.5)
#
#    profile_code(name="main_pybind11.py",profile_results_name="mac_pybind11",is_python=True,image_size_list=[200,500],n_time_chunks_list=[3],n_chan_chunks_list=[3],sampling_interval=0.5)
    
    #image_size = [500, 1000, 1500, 2000, 2500, 5000]
    image_size = [5000]
    n_chan_chunks_list = [10]
    n_time_chunks_list = [1]
    
    profile_code(name="cpp_gridder",profile_results_name="linux_cpp",is_python=False,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
    profile_code(name="main_numba.py",profile_results_name="linux_numba",is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)
    profile_code(name="main_pybind11.py",profile_results_name="linux_pybind11",is_python=True,image_size_list=image_size,n_time_chunks_list=n_time_chunks_list,n_chan_chunks_list=n_chan_chunks_list,sampling_interval=0.1)




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
