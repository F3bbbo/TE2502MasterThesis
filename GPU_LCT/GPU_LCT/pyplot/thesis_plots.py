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
            if not is_input:
                os.makedirs(abs_file_path)
    else:
        abs_file_path = os.path.join(script_dir, kallmann_folder)
    abs_file_path = os.path.join(abs_file_path, fileName)
    return abs_file_path;

def get_results_from_file(file_name, max_vertex_count = -1):
    with open(file_name, "r") as file:
        y_groups = file.readline().split(',')
        y_groups.append("combined")
        meta_data = file.readline().split(',')
        iterations = int(meta_data[0])
        increase_iterations = int(meta_data[1])

        # Create base of return values
        y_labels = dict()
        y_labels[y_groups[0]] = list()
        y_labels[y_groups[1]] = list()
        y_labels[y_groups[2]] = list()
        x_labels = list()
        std_dev = dict()
        std_dev[y_groups[0]] = list()
        std_dev[y_groups[1]] = list()
        std_dev[y_groups[2]] = list()
        counter = 0
        for line in file:
            if(counter == 0):
                sizes = line.split(',')
                sizes = [int(x) for x in sizes]
                num_vertices = sum(sizes)
                iter_values = dict()
                iter_values[y_groups[0]] = list()
                iter_values[y_groups[1]] = list()
                iter_values[y_groups[2]] = list()
                if (max_vertex_count > -1) and max_vertex_count < num_vertices:
                    break;
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

                # Calculate average and std of third group
                y_labels[y_groups[2]].append(np.mean(iter_values[y_groups[2]]))
                std_dev[y_groups[2]].append(np.std(iter_values[y_groups[2]]))
                # Reset counter var
                counter = 0
            else:
                l = line.split(',')
                iter_values[y_groups[0]].append(float(l[0]))
                iter_values[y_groups[1]].append(float(l[1]))
                iter_values[y_groups[2]].append(float(l[0]) + float(l[1]))
                counter += 1

    return y_labels, y_groups,  x_labels, std_dev

def get_second_result_from_file(file_name):
    with open(file_name, "r") as file:
        file.readline()
        file.readline()
        file.readline()
        first_shader_times = file.readline().split(',')
        iteration_times = list()
        # add the amount of lists needed for all the stages of the algorithm
        for i in range(0, len(first_shader_times)):
            iteration_times.append(list())
        # add the first time values to the iteration list
        for save_list, shader_time in zip(iteration_times, first_shader_times):
            save_list.append(float(shader_time))
        # add the rest of test times of the file
        for line in file:
            shader_times = line.split(',')
            for save_list, shader_time in zip(iteration_times, shader_times):
                if(shader_time is not '\n'):
                    save_list.append(float(shader_time))
        #  calculate the mean values and std_dev
        y_label = list()
        std_dev = list()
        for iters in iteration_times:
            y_label.append(np.mean(iters))
            std_dev.append(np.std(iters))
    return y_label, std_dev

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
def make_second_test_plot(save_file, y_labels, std_dev, title1="CDT Shaders Performance", title2 = "LCT Shaders Performance", y_axis_label = "Execution time(ms)", x_axis_label = ""):

    fig = plt.figure()
    ax = fig.add_subplot(1,1,1)
    ax1 = fig.add_subplot(2, 1, 1)
    ax2 = fig.add_subplot(2, 1, 2)

    # Turn off axis lines and ticks of the big subplot
    ax.spines['top'].set_color(None)
    ax.spines['bottom'].set_color(None)
    ax.spines['left'].set_color(None)
    ax.spines['right'].set_color(None)
    ax.tick_params(labelcolor='w', top=False, bottom=False, left=False, right=False)
    # Set common axis settings
    fig.tight_layout()
    ax.set_xlabel(x_axis_label)
    ax.set_ylabel(y_axis_label)
    plt.subplots_adjust(bottom=0.25, hspace=0.95)
    #figure 1
    shader_names = ["Locate\nStep 1", "Locate\nStep 2", "Marking\nStep 1", "Marking\nStep 2", "Flipping\nStep 1", "Flipping\nStep 2", "Flipping\nStep 3"]
    ax1.title.set_text(title1)
    ax1.bar(shader_names, y_labels[:7], yerr=std_dev[:7])
    # fix x_labels settings
    plt.setp(ax1.xaxis.get_majorticklabels(), rotation=70)
    #figure 2
    shader_names = ["Locate\nDisturbance", "Add new\npoints", "Locate Point\nTriangle", "Validate\nEdges", "Insert\n in Edge", "Marking\nStep 2", "Flipping\nStep 1", "Flipping\nStep 2", "Flipping\nStep 3"]
    ax2.title.set_text(title2)
    ax2.bar(shader_names, y_labels[7:], yerr=std_dev[7:])
    # fix x_labels settings
    plt.setp(ax2.xaxis.get_majorticklabels(), rotation=70)

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
y_axis_label = "Execution time(ms)"
x_axis_label = "Number of vertices"
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
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)

    test_type = first_CG_results[1][1]
    #print(first_CG_results[2][test_type])
    y_labels_list = [first_CG_results[0][test_type], first_G_results[0][test_type], first_kall_results[0][test_type]]
    x_labels_list = [first_CG_results[2], first_G_results[2], first_kall_results[2]]
    std_dev_list = [first_CG_results[3][test_type], first_G_results[3][test_type], first_kall_results[3][test_type]]
    alg_names = ["CPUGPU", "GPU", "Kallmann"]
    save_file_name = abs_path("First_test_LCT_GPU_Kallmann.png", True, False)
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)

    #first_CG_file = open(abs_path("first_test_CPUGPU-9-3600-v2.txt"), "r")
    #first_G_file = open(abs_path("first_test_CPUGPU-100-90000-v2.txt"), "r")
    #first_Kall_file = open(abs_path("first_test_CPU-100-90000-v0.txt", False), "r")
# Second plot
if (plots.get(2) is not None):
    second_test_results = get_second_result_from_file(abs_path("second_test-12100-v2.txt"))
    save_file_name = abs_path("Second_test_results_12100", True, False)
    make_second_test_plot(save_file_name, second_test_results[0], second_test_results[1])
# Third plot
if(plots.get(3) is not None):
        third_CG_25_results = get_results_from_file(abs_path("third_test_CPUGPU-300-7500-v2-0.25.txt"))
        third_CG_50_results = get_results_from_file(abs_path("third_test_CPUGPU-300-7500-v2-0.50.txt"))
        third_CG_75_results = get_results_from_file(abs_path("third_test_CPUGPU-300-7500-v2-0.75.txt"))
        third_G_25_results = get_results_from_file(abs_path("third_test_GPU-300-108300-v2-0.25.txt"))
        third_G_50_results = get_results_from_file(abs_path("third_test_GPU-300-108300-v2-0.50.txt"))
        third_G_75_results = get_results_from_file(abs_path("third_test_GPU-300-108300-v2-0.75.txt"))
        third_kall_25_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.25.txt", False))
        third_kall_50_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.50.txt", False))
        third_kall_75_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.75.txt", False))
        # CDT
        test_type = first_CG_results[1][0]
        # create  25% plot
        y_labels_list = [third_CG_25_results[0][test_type], third_G_25_results[0][test_type], third_kall_25_results[0][test_type]]
        x_labels_list = [third_CG_25_results[2], third_G_25_results[2], third_kall_25_results[2]]
        std_dev_list = [third_CG_25_results[3][test_type], third_G_25_results[3][test_type], third_kall_25_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_CDT_GPU_Kallmann_0.25.png", True, False)
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)

        # create 50% plot
        y_labels_list = [third_CG_50_results[0][test_type], third_G_50_results[0][test_type], third_kall_50_results[0][test_type]]
        x_labels_list = [third_CG_50_results[2], third_G_50_results[2], third_kall_50_results[2]]
        std_dev_list = [third_CG_50_results[3][test_type], third_G_50_results[3][test_type], third_kall_50_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_CDT_GPU_Kallmann_0.50.png", True, False)
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)


        # create 75% plot
        y_labels_list = [third_CG_75_results[0][test_type], third_G_75_results[0][test_type], third_kall_75_results[0][test_type]]
        x_labels_list = [third_CG_75_results[2], third_G_75_results[2], third_kall_75_results[2]]
        std_dev_list = [third_CG_75_results[3][test_type], third_G_75_results[3][test_type], third_kall_75_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_CDT_GPU_Kallmann_0.75.png", True, False)
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)

        # LCT
        test_type = first_CG_results[1][1]
        # create  25% plot
        y_labels_list = [third_CG_25_results[0][test_type], third_G_25_results[0][test_type], third_kall_25_results[0][test_type]]
        x_labels_list = [third_CG_25_results[2], third_G_25_results[2], third_kall_25_results[2]]
        std_dev_list = [third_CG_25_results[3][test_type], third_G_25_results[3][test_type], third_kall_25_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_LCT_GPU_Kallmann_0.25.png", True, False)
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)

        # create 50% plot
        y_labels_list = [third_CG_50_results[0][test_type], third_G_50_results[0][test_type], third_kall_50_results[0][test_type]]
        x_labels_list = [third_CG_50_results[2], third_G_50_results[2], third_kall_50_results[2]]
        std_dev_list = [third_CG_50_results[3][test_type], third_G_50_results[3][test_type], third_kall_50_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_LCT_GPU_Kallmann_0.50.png", True, False)
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)


        # create 75% plot
        y_labels_list = [third_CG_75_results[0][test_type], third_G_75_results[0][test_type], third_kall_75_results[0][test_type]]
        x_labels_list = [third_CG_75_results[2], third_G_75_results[2], third_kall_75_results[2]]
        std_dev_list = [third_CG_75_results[3][test_type], third_G_75_results[3][test_type], third_kall_75_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_LCT_GPU_Kallmann_0.75.png", True, False)
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, y_axis_label, x_axis_label)
