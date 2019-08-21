import numpy as np
import matplotlib.pyplot as plt
import math
import sys
import re
import os

num_args = len(sys.argv) - 1
# arguments:
# -plots 1 2 3 4 ...
# -in locations
def print_help():
    options = """   Valid arguments
    -p/ -plots [1 2 3 4 ...] any number of integers seperated by space
        0 : All plots
        1 : First test plot
        2 : Second test plot
        3 : Third test plot
    -w / -workfolder "path" set the workspace folder
    -i / -infolder "path" set folder where input is
    -o / -outfolder "path" set folder where output will go
    -k / kallmannfolder "path" set the folder where Kallann test results are
    """
    print(options)
def all_plots():
    return dict({1 : True, 2 : True, 3 : True})

plots = all_plots()
in_folder = "..\\Output files"
out_folder = "Thesis_plots"
work_folder = ""
kallmann_folder = "..\\..\\CPU_LCT\\Output files"
out_file_name = "Result_plots"

def abs_path(fileName, is_gpu = True, is_input = True):
    global work_folder
    global in_folder
    global out_folder
    global kallmann_folder
    script_dir = os.path.dirname(os.path.abspath(__file__)) #<-- absolute dir the script is in
    #print("script dir: " + script_dir)
    if(is_gpu):
        abs_file_path = os.path.join(script_dir, work_folder)
        if(is_input):
            abs_file_path = os.path.join(abs_file_path, in_folder)
        else:
            abs_file_path = os.path.join(abs_file_path, out_folder)
        if not os.path.exists(abs_file_path):
            os.makedirs(abs_file_path)
    else:
        abs_file_path = os.path.join(script_dir, kallmann_folder)
    abs_file_path = os.path.join(abs_file_path, fileName)
    return abs_file_path;

def get_results_from_file(file_name):
    with open(file_name, "r") as file:
        y_groups = file.readline().split(',')
        meta_data = file.readline().split(',')
        iterations = int(meta_data[0])
        increase_iterations = int(meta_data[1])
        # Create base of return values
        y_labels = dict()
        y_labels[y_groups[0]] = list()
        y_labels[y_groups[1]] = list()
        x_labels = list()
        std_dev = dict()
        std_dev[y_groups[0]] = list()
        std_dev[y_groups[1]] = list()
        counter = 0
        for line in file:
            if(counter == 0):
                num_vertices = int(line)
                iter_values = dict()
                iter_values[y_groups[0]] = list()
                iter_values[y_groups[1]] = list()

                counter += 1
            elif(counter > iterations):
                # add the average values to y_labels ans std_dev
                x_labels.append(num_vertices)
                # Calculate average and std of first group
                y_labels[y_groups[0]].append(np.mean(iter_values[y_groups[0]]))
                std_dev[y_groups[0]].append(np.std(iter_values[y_groups[0]]))

                # Calculate average and std of second group
                y_labels[y_groups[1]].append(np.mean(iter_values[y_groups[1]]))
                std_dev[y_groups[1]].append(np.std(iter_values[y_groups[1]]))

                # Reset counter var
                counter = 0
            else:
                l = line.split(',')
                iter_values[y_groups[0]].append(float(l[0]))
                iter_values[y_groups[1]].append(float(l[1]))
                counter += 1

    return y_labels, y_groups,  x_labels, std_dev

def make_line_plot(save_file, y_labels_list, x_labels_list, std_dev_list, algorithm_names, y_axis_label="", x_axis_label=""):
    fig, ax = plt.subplots()
    ax.set_ylabel(y_axis_label)
    ax.set_xlabel(x_axis_label)
    for y_labels, x_labels, std_dev, alg_name in zip(y_labels_list, x_labels_list, std_dev_list, algorithm_names):
        ax.errorbar(x_labels, y_labels, yerr=std_dev, label=alg_name)
    plt.legend()
    if(save_file == ""):
        plt.show()
    else:
        plt.savefig(save_file)
    return

def process_parameter(start_i):
    global plots
    global in_folder
    global out_folder
    global num_args
    if(start_i < num_args):
        next_i = start_i + 1
        reg_match = re.match("-[a-z]*", sys.argv[start_i])
        #print(reg_match)
        if(bool(reg_match) == True):
            argument = reg_match[0][1:]
            if(argument == "p" or argument == "plots"):
                zero_found = False
                plots = dict()
                while(next_i <= num_args and sys.argv[next_i].isdigit()):
                    tmp = int(sys.argv[next_i])
                    if(tmp == 0):
                        zero_found = True
                    plots[tmp] = True
                    next_i += 1
                if(zero_found):
                    plots = all_plots()
            elif(argument == "i" or argument == "infolder"):
                in_folder =  sys.argv[next_i]
                next_i += 1
                while(next_i <= num_args and sys.argv[next_i] is not "-"):
                    in_folder += " " + sys.argv[next_i]
                    next_i += 1
            elif(argument == "o" or argument == "outfolder"):
                out_folder = sys.argv[next_i]
                next_i += 1
                while(next_i <= num_args and sys.argv[next_i] is not "-"):
                    out_folder += " " + sys.argv[next_i]
                    next_i += 1
            elif(argument == "w" or argument == "workfolder"):
                work_folder =  sys.argv[next_i]
                next_i += 1
                while(next_i <= num_args and sys.argv[next_i] is not "-"):
                    out_folder += " " + sys.argv[next_i]
                    next_i += 1
            else:
                print_help()
                next_i = -1
        else:
            print_help()
            next_i = -1
    else:
        next_i = -1
    return next_i # -1 err
# check for input parameters
curr_i = 1
while (curr_i != -1):
    curr_i = process_parameter(curr_i)
# Test code
#print_help()
#print(plots)
#print("-plot"[0] is "-")

# First plot
if(plots.get(1) is not None):
    first_CG_results = get_results_from_file(abs_path("first_test_CPUGPU-9-3600-v2.txt"))
    first_G_results = get_results_from_file(abs_path("first_test_GPU-100-90000-v2.txt"))
    first_kall_results = get_results_from_file(abs_path("first_test_CPU-100-90000-v0.txt", False))

    test_type = first_CG_results[1][0]
    #print(first_CG_results[2][test_type])
    y_labels_list = [ first_G_results[0][test_type], first_kall_results[0][test_type]]
    x_labels_list = [first_G_results[2], first_kall_results[2]]
    std_dev_list = [first_G_results[3][test_type], first_kall_results[3][test_type]]
    alg_names = [ "GPU", "Kallmann"]
    save_file_name = abs_path("First_test_CDT_GPU_Kallmann.png", True, False)
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names)

    test_type = first_CG_results[1][1]
    #print(first_CG_results[2][test_type])
    y_labels_list = [first_CG_results[0][test_type], first_G_results[0][test_type], first_kall_results[0][test_type]]
    x_labels_list = [first_CG_results[2], first_G_results[2], first_kall_results[2]]
    std_dev_list = [first_CG_results[3][test_type], first_G_results[3][test_type], first_kall_results[3][test_type]]
    alg_names = ["CPUGPU", "GPU", "Kallmann"]
    save_file_name = abs_path("First_test_LCT_GPU_Kallmann.png", True, False)
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names)

    #first_CG_file = open(abs_path("first_test_CPUGPU-9-3600-v2.txt"), "r")
    #first_G_file = open(abs_path("first_test_CPUGPU-100-90000-v2.txt"), "r")
    #first_Kall_file = open(abs_path("first_test_CPU-100-90000-v0.txt", False), "r")
# Second plot


# Third plot
