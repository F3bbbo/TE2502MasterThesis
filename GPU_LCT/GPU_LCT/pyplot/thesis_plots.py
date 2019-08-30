import numpy as np
import matplotlib.pyplot as plt
import math
import sys
import re
import os
import scipy.stats
import scipycopy.stats
from decimal import Decimal

num_args = len(sys.argv) - 1
# arguments:
# -plots 1 2 3 4 ...
# -in locations
def print_help():
    options = """   Valid arguments
    -p/ -plots [1 2 3 4 ...] any number of integers seperated by space
        0 : All plots
        1 : First test plots
        2 : Second test plots
        3 : Third test plots
        4 : Mann Whitney plots
    -w / -workfolder "path" set the workspace folder
    -i / -infolder "path" set folder where input is
    -o / -outfolder "path" set folder where output will go
    -k / kallmannfolder "path" set the folder where Kallann test results are
    """
    print(options)
def all_plots():
    return dict({1 : True, 2 : True, 3 : True, 4 : True})

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
        test_data = dict()
        test_data[y_groups[0]] = list()
        test_data[y_groups[1]] = list()
        test_data[y_groups[2]] = list()
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
                # Add test values to test_data list
                test_data[y_groups[0]].append(iter_values[y_groups[0]])
                test_data[y_groups[1]].append(iter_values[y_groups[1]])
                test_data[y_groups[2]].append(iter_values[y_groups[2]])

                # Reset counter var
                counter = 0
            else:
                l = line.split(',')
                iter_values[y_groups[0]].append(float(l[0]))
                iter_values[y_groups[1]].append(float(l[1]))
                iter_values[y_groups[2]].append(float(l[0]) + float(l[1]))
                counter += 1

    return y_labels, y_groups,  x_labels, std_dev, test_data

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

def make_line_plot(save_file, y_labels_list, x_labels_list, std_dev_list, algorithm_names, title="", y_axis_label="", x_axis_label=""):
    fig, ax = plt.subplots()
    ax.title.set_text(title)
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

def createWilcoxonTable(sample_list1, sample_list2, map_sizes, tex_file, sample_name_1 = 'Sample 1', sample_name_2 = 'Sample 2'):
    f = open(tex_file, 'w+')
    table_head = ('\\begin{tabular}{|c|c|c|c|c|c|c|c|c|c|}\n'
            '\\hline\n'
            '\\multirow{2}{*}{Map Size} & \\multicolumn{4}{c|}{' + sample_name_1 +'} & \\multicolumn{4}{c|}{' + sample_name_2 + '} & ' '\\multirow{2}{*}{\\begin{tabular}[c]{@{}c@{}}P-value \\\\ (two-sided)\end{tabular}} \\\\ \\cline{2-9}\n'
            ' & \\begin{tabular}[c]{@{}c@{}}Sample \\\\ Size\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}Sum of \\\\ Ranks\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}Mean \\\\ Rank\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}U \\\\ Statistic\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}Sample \\\\ Size\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}Sum of \\\\ Ranks\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}Mean \\\\ Rank\\end{tabular} & \\begin{tabular}[c]{@{}c@{}}U \\\\ Statistic\\end{tabular} & \\\\ \\hline')
    f.write(table_head)

    for list in zip(sample_list1, sample_list2, map_sizes):
        sample1, sample2, mapsize = list[0], list[1], list[2]
        n1, ranksumx, rankmeanx, u1, n2, ranksumy, rankmeany, u2, p = scipycopy.stats.mannwhitneyu(sample1, sample2, alternative='two-sided')
        print('SampleSizeX=%d, RankSumX=%.3f, RankMeanX=%.3f, U1=%.3f, SampleSizeY=%d, RankSumY=%.3f, RankMeanY=%.3f, U2=%.3f, p=%f' % (n1, ranksumx, rankmeanx, u1, n2, ranksumy, rankmeany, u2, p))
        # Limit signficant digits to 3 for mean ranks
        rankmeanx = '%s' % float('%.3g' % rankmeanx)
        rankmeany = '%s' % float('%.3g' % rankmeany)
        # Convert p to scientific notation or limit significant digits to 3.
        if(p < 0.001):
            p = '%.2E' % Decimal(p)
        else:
            p = '%s' % float('%.3g' % p)
        res = [n1, ranksumx, rankmeanx, u1, n2, ranksumy, rankmeany, u2, p]
        results =[str(i) for i in res]
        row = str(mapsize) + ' & ' + ' & '.join(results) + '\\\\ \\hline\n'
        f.write(row)
    table_end = '\\end{tabular}'
    f.write(table_end)
    f.close()

def createWilcoxonTableSmall(sample_list1, sample_list2, map_sizes, tex_file, sample_name_1 = 'Sample 1', sample_name_2 = 'Sample 2'):
    f = open(tex_file, 'w+')
    table_head = ('\\begin{tabular}{|c|c|c|c|}'
                '\\hline'
                '\\multirow{2}{*}{Map Size} & \\multirow{2}{*}{\\begin{tabular}[c]{@{}c@{}}Mean Rank of \\\\ '+ sample_name_1 +'\\end{tabular}} & \\multirow{2}{*}{\\begin{tabular}[c]{@{}c@{}}Mean Rank of\\\\ ' + sample_name_2 +'\\end{tabular}} & \\multirow{2}{*}{\\begin{tabular}[c]{@{}c@{}}P-Value \\\\ (two-sided)\\end{tabular}} \\\\'
                '& & & \\\\ \\hline')
    f.write(table_head)
    for list in zip(sample_list1, sample_list2, map_sizes):
        sample1, sample2, mapsize = list[0], list[1], list[2]
        n1, ranksumx, rankmeanx, u1, n2, ranksumy, rankmeany, u2, p = scipycopy.stats.mannwhitneyu(sample1, sample2, alternative='two-sided')
        # Limit signficant digits to 3 for mean ranks
        rankmeanx = '%s' % float('%.3g' % rankmeanx)
        rankmeany = '%s' % float('%.3g' % rankmeany)
        # Convert p to scientific notation or limit significant digits to 3.
        if(p < 0.001):
            p = '%.2E' % Decimal(p)
        else:
            p = '%s' % float('%.3g' % p)
        #res = [n1, ranksumx, rankmeanx, u1, n2, ranksumy, rankmeany, u2, p]
        #results =[str(i) for i in res]
        row = str(mapsize) + ' & ' + str(rankmeanx)+ ' & ' + str(rankmeany) + ' & '+ str(p) + '\\\\ \\hline\n'
        f.write(row)
    table_end = '\\end{tabular}\n'
    f.write(table_end)
    f.close()

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
    first_CG_results = get_results_from_file(abs_path("first_test_CPUGPU-100-40000-v2.txt"))
    first_G_results = get_results_from_file(abs_path("first_test_GPU-100-52900-v2.txt"))
    first_kall_results = get_results_from_file(abs_path("first_test_CPU-100-84100-v0.txt", False), first_G_results[2][-1])

    test_type = first_CG_results[1][0]
    #print(first_CG_results[2][test_type])
    y_labels_list = [first_CG_results[0][test_type], first_G_results[0][test_type], first_kall_results[0][test_type]]
    x_labels_list = [first_CG_results[2], first_G_results[2], first_kall_results[2]]
    std_dev_list = [first_CG_results[3][test_type], first_G_results[3][test_type], first_kall_results[3][test_type]]
    alg_names = ["CPUGPU", "GPU", "Kallmann"]
    save_file_name = abs_path("First_test_CDT.png", True, False)
    title = "First test CDT execution times"
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

    test_type = first_CG_results[1][1]
    #print(first_CG_results[2][test_type])
    y_labels_list = [first_CG_results[0][test_type], first_G_results[0][test_type], first_kall_results[0][test_type]]
    x_labels_list = [first_CG_results[2], first_G_results[2], first_kall_results[2]]
    std_dev_list = [first_CG_results[3][test_type], first_G_results[3][test_type], first_kall_results[3][test_type]]
    alg_names = ["CPUGPU", "GPU", "Kallmann"]
    save_file_name = abs_path("First_test_LCT.png", True, False)
    title = "First test LCT execution times"
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

    test_type = first_CG_results[1][2]
    #print(first_CG_results[2][test_type])
    y_labels_list = [first_CG_results[0][test_type], first_G_results[0][test_type], first_kall_results[0][test_type]]
    x_labels_list = [first_CG_results[2], first_G_results[2], first_kall_results[2]]
    std_dev_list = [first_CG_results[3][test_type], first_G_results[3][test_type], first_kall_results[3][test_type]]
    alg_names = ["CPUGPU", "GPU", "Kallmann"]
    save_file_name = abs_path("First_test_Full_LCT.png", True, False)
    title = "First test full LCT execution times"
    make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

# Second plot
if (plots.get(2) is not None):
    second_test_results = get_second_result_from_file(abs_path("second_test-12100-v2.txt"))
    save_file_name = abs_path("Second_test_results_12100", True, False)
    make_second_test_plot(save_file_name, second_test_results[0], second_test_results[1])
# Third plot
if(plots.get(3) is not None):
        third_CG_25_results = get_results_from_file(abs_path("third_test_CPUGPU-300-120000-v2-0.25.txt"))
        third_CG_50_results = get_results_from_file(abs_path("third_test_CPUGPU-300-120000-v2-0.50.txt"))
        third_CG_75_results = get_results_from_file(abs_path("third_test_CPUGPU-300-120000-v2-0.75.txt"))
        third_G_25_results = get_results_from_file(abs_path("third_test_GPU-300-145200-v2-0.25.txt"))
        third_G_50_results = get_results_from_file(abs_path("third_test_GPU-300-145200-v2-0.50.txt"))
        third_G_75_results = get_results_from_file(abs_path("third_test_GPU-300-145200-v2-0.75.txt"))
        third_kall_25_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.25.txt", False), third_G_25_results[2][-1])
        third_kall_50_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.50.txt", False), third_G_50_results[2][-1])
        third_kall_75_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.75.txt", False), third_G_75_results[2][-1])
        # CDT
        test_type = third_CG_25_results[1][0]
        # create  25% plot
        y_labels_list = [third_CG_25_results[0][test_type], third_G_25_results[0][test_type], third_kall_25_results[0][test_type]]
        x_labels_list = [third_CG_25_results[2], third_G_25_results[2], third_kall_25_results[2]]
        std_dev_list = [third_CG_25_results[3][test_type], third_G_25_results[3][test_type], third_kall_25_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_CDTn_0_25.png", True, False)
        title = "Third test CDT execution times(75% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

        # create 50% plot
        y_labels_list = [third_CG_50_results[0][test_type], third_G_50_results[0][test_type], third_kall_50_results[0][test_type]]
        x_labels_list = [third_CG_50_results[2], third_G_50_results[2], third_kall_50_results[2]]
        std_dev_list = [third_CG_50_results[3][test_type], third_G_50_results[3][test_type], third_kall_50_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_CDT_0_50.png", True, False)
        title = "Third test CDT execution times(50% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)


        # create 75% plot
        y_labels_list = [third_CG_75_results[0][test_type], third_G_75_results[0][test_type], third_kall_75_results[0][test_type]]
        x_labels_list = [third_CG_75_results[2], third_G_75_results[2], third_kall_75_results[2]]
        std_dev_list = [third_CG_75_results[3][test_type], third_G_75_results[3][test_type], third_kall_75_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_CDT_0_75.png", True, False)
        title = "Third test CDT execution times(25% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

        # LCT
        test_type = third_CG_25_results[1][1]
        # create  25% plot
        y_labels_list = [third_CG_25_results[0][test_type], third_G_25_results[0][test_type], third_kall_25_results[0][test_type]]
        x_labels_list = [third_CG_25_results[2], third_G_25_results[2], third_kall_25_results[2]]
        std_dev_list = [third_CG_25_results[3][test_type], third_G_25_results[3][test_type], third_kall_25_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_LCT_0_25.png", True, False)
        title = "Third test LCT execution times(75% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

        # create 50% plot
        y_labels_list = [third_CG_50_results[0][test_type], third_G_50_results[0][test_type], third_kall_50_results[0][test_type]]
        x_labels_list = [third_CG_50_results[2], third_G_50_results[2], third_kall_50_results[2]]
        std_dev_list = [third_CG_50_results[3][test_type], third_G_50_results[3][test_type], third_kall_50_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_LCT_0_50.png", True, False)
        title = "Third test LCT execution times(50% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)


        # create 75% plot
        y_labels_list = [third_CG_75_results[0][test_type], third_G_75_results[0][test_type], third_kall_75_results[0][test_type]]
        x_labels_list = [third_CG_75_results[2], third_G_75_results[2], third_kall_75_results[2]]
        std_dev_list = [third_CG_75_results[3][test_type], third_G_75_results[3][test_type], third_kall_75_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_LCT_0_75.png", True, False)
        title = "Third test LCT execution times(25% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

        # Full LCT
        test_type = third_CG_25_results[1][2]
        # create  25% plot
        y_labels_list = [third_CG_25_results[0][test_type], third_G_25_results[0][test_type], third_kall_25_results[0][test_type]]
        x_labels_list = [third_CG_25_results[2], third_G_25_results[2], third_kall_25_results[2]]
        std_dev_list = [third_CG_25_results[3][test_type], third_G_25_results[3][test_type], third_kall_25_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_Full_LCT_0_25.png", True, False)
        title = "Third test full LCT execution times(75% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

        # create 50% plot
        y_labels_list = [third_CG_50_results[0][test_type], third_G_50_results[0][test_type], third_kall_50_results[0][test_type]]
        x_labels_list = [third_CG_50_results[2], third_G_50_results[2], third_kall_50_results[2]]
        std_dev_list = [third_CG_50_results[3][test_type], third_G_50_results[3][test_type], third_kall_50_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_Full_LCT_0_50.png", True, False)
        title = "Third test full LCT execution times(50% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)


        # create 75% plot
        y_labels_list = [third_CG_75_results[0][test_type], third_G_75_results[0][test_type], third_kall_75_results[0][test_type]]
        x_labels_list = [third_CG_75_results[2], third_G_75_results[2], third_kall_75_results[2]]
        std_dev_list = [third_CG_75_results[3][test_type], third_G_75_results[3][test_type], third_kall_75_results[3][test_type]]
        alg_names = ["CPUGPU", "GPU", "Kallmann"]
        save_file_name = abs_path("Third_test_Full_LCT_0_75.png", True, False)
        title = "Third test full LCT execution times(25% of map)"
        make_line_plot(save_file_name, y_labels_list, x_labels_list, std_dev_list, alg_names, title, y_axis_label, x_axis_label)

if(plots.get(4) is not None):
    print('Wilcoxon rank-sum test (aka Mann-Whitney U test)')
    # Example data
    #datalist1 = [[1,2,1],[1,2,2],[1,2,3],[1,2,4],[1,2,5],[1,2,6],[1,2,7],[1,2,8]]
    #datalist2 = [[2,4,6],[2,4,5],[2,4,4],[2,4,3],[2,4,2],[2,4,1]]
    #map_sizes = [50, 100, 150, 200, 250, 300, 350, 400, 450, 500]
    ##############
    #tex_file = 'table.tex'
    #createWilcoxonTable(datalist1, datalist2, map_sizes, tex_file)
    #other_tex_file = 'table2.tex'
    #createWilcoxonTableSmall(datalist1, datalist2, map_sizes, other_tex_file)
    GPU_name = "GPU LCT"
    CPU_name = "CPUGPU LCT"
    Kallmann_name = "Kallmann LCT"
    #-------------------------------------------------------------------------
    # First Test
    #-------------------------------------------------------------------------
    first_CG_results = get_results_from_file(abs_path("first_test_CPUGPU-100-40000-v2.txt"))
    first_G_results = get_results_from_file(abs_path("first_test_GPU-100-52900-v2.txt"))
    first_kall_results = get_results_from_file(abs_path("first_test_CPU-100-84100-v0.txt", False), first_G_results[2][-1])
    # GPU vs Kallmann
    save_file_name = abs_path("First_test_Full_MannWhitney_Full_LCT_GPU_Kallmann.tex", True, False)
    test_type = first_G_results[1][2]
    createWilcoxonTable(first_G_results[4][test_type], first_kall_results[4][test_type], first_G_results[2], save_file_name, GPU_name, Kallmann_name)
    save_file_name = abs_path("First_test_MannWhitney_Full_LCT_GPU_Kallmann.tex", True, False)
    createWilcoxonTableSmall(first_G_results[4][test_type], first_kall_results[4][test_type], first_G_results[2], save_file_name, GPU_name, Kallmann_name)

    # CPU vs GPU
    save_file_name = abs_path("First_test_Full_MannWhitney_Full_LCT_GPU_CPUGPU.tex", True, False)
    test_type = first_G_results[1][2]
    createWilcoxonTable(first_G_results[4][test_type], first_CG_results[4][test_type], first_G_results[2], save_file_name, GPU_name, CPU_name)
    save_file_name = abs_path("First_test_MannWhitney_Full_LCT_GPU_CPUGPU.tex", True, False)
    createWilcoxonTableSmall(first_G_results[4][test_type], first_CG_results[4][test_type], first_G_results[2], save_file_name, GPU_name, CPU_name)
    #-------------------------------------------------------------------------
    # Third Test
    #-------------------------------------------------------------------------
    third_CG_25_results = get_results_from_file(abs_path("third_test_CPUGPU-300-120000-v2-0.25.txt"))
    third_CG_50_results = get_results_from_file(abs_path("third_test_CPUGPU-300-120000-v2-0.50.txt"))
    third_CG_75_results = get_results_from_file(abs_path("third_test_CPUGPU-300-120000-v2-0.75.txt"))
    third_G_25_results = get_results_from_file(abs_path("third_test_GPU-300-145200-v2-0.25.txt"))
    third_G_50_results = get_results_from_file(abs_path("third_test_GPU-300-145200-v2-0.50.txt"))
    third_G_75_results = get_results_from_file(abs_path("third_test_GPU-300-145200-v2-0.75.txt"))
    third_kall_25_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.25.txt", False), third_G_25_results[2][-1])
    third_kall_50_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.50.txt", False), third_G_50_results[2][-1])
    third_kall_75_results = get_results_from_file(abs_path("third_test_CPU-300-270000-v0-0.75.txt", False), third_G_75_results[2][-1])

    #-------------------------------------------------------------------------
    #  25 %
    #-------------------------------------------------------------------------
    # GPU vs Kallmann
    save_file_name = abs_path("Third_test_Full_MannWhitney_Full_LCT_GPU_Kallmann_0.25.tex", True, False)
    test_type = third_G_25_results[1][2]
    createWilcoxonTable(third_G_25_results[4][test_type], third_kall_25_results[4][test_type], third_G_25_results[2], save_file_name, GPU_name, Kallmann_name)
    save_file_name = abs_path("Third_test_MannWhitney_Full_LCT_GPU_Kallmann_0.25.tex", True, False)
    createWilcoxonTableSmall(third_G_25_results[4][test_type], third_kall_25_results[4][test_type], third_G_25_results[2], save_file_name, GPU_name, Kallmann_name)

    # CPU vs GPU
    save_file_name = abs_path("Third_test_Full_MannWhitney_Full_LCT_GPU_CPUGPU_0.25.tex", True, False)
    test_type = third_G_25_results[1][2]
    createWilcoxonTable(third_G_25_results[4][test_type], third_CG_25_results[4][test_type], third_G_25_results[2], save_file_name, GPU_name, CPU_name)
    save_file_name = abs_path("Third_test_MannWhitney_Full_LCT_GPU_CPUGPU_0.25.tex", True, False)
    createWilcoxonTableSmall(third_G_25_results[4][test_type], third_CG_25_results[4][test_type], third_G_25_results[2], save_file_name, GPU_name, CPU_name)

    #-------------------------------------------------------------------------
    #  50 %
    #-------------------------------------------------------------------------
    # GPU vs Kallmann
    save_file_name = abs_path("Third_test_Full_MannWhitney_Full_LCT_GPU_Kallmann_0.50.tex", True, False)
    test_type = third_G_75_results[1][2]
    createWilcoxonTable(third_G_50_results[4][test_type], third_kall_50_results[4][test_type], third_G_50_results[2], save_file_name, GPU_name, Kallmann_name)
    save_file_name = abs_path("Third_test_MannWhitney_Full_LCT_GPU_Kallmann_0.50.tex", True, False)
    createWilcoxonTableSmall(third_G_50_results[4][test_type], third_kall_50_results[4][test_type], third_G_50_results[2], save_file_name, GPU_name, Kallmann_name)

    # CPU vs GPU
    save_file_name = abs_path("Third_test_Full_MannWhitney_Full_LCT_GPU_CPUGPU_0.50.tex", True, False)
    test_type = third_G_75_results[1][2]
    createWilcoxonTable(third_G_50_results[4][test_type], third_CG_50_results[4][test_type], third_G_50_results[2], save_file_name, GPU_name, CPU_name)
    save_file_name = abs_path("Third_test_MannWhitney_Full_LCT_GPU_CPUGPU_0.50.tex", True, False)
    createWilcoxonTableSmall(third_G_50_results[4][test_type], third_CG_50_results[4][test_type], third_G_50_results[2], save_file_name, GPU_name, CPU_name)


    #-------------------------------------------------------------------------
    #  75 %
    #-------------------------------------------------------------------------
    # GPU vs Kallmann
    save_file_name = abs_path("Third_test_Full_MannWhitney_Full_LCT_GPU_Kallmann_0.75.tex", True, False)
    test_type = third_G_75_results[1][2]
    createWilcoxonTable(third_G_75_results[4][test_type], third_kall_75_results[4][test_type], third_G_75_results[2], save_file_name, GPU_name, Kallmann_name)
    save_file_name = abs_path("Third_test_MannWhitney_Full_LCT_GPU_Kallmann_0.75.tex", True, False)
    createWilcoxonTableSmall(third_G_75_results[4][test_type], third_kall_75_results[4][test_type], third_G_75_results[2], save_file_name, GPU_name, Kallmann_name)

    # CPU vs GPU
    save_file_name = abs_path("Third_test_Full_MannWhitney_Full_LCT_GPU_CPUGPU_0.75.tex", True, False)
    test_type = third_G_75_results[1][2]
    createWilcoxonTable(third_G_75_results[4][test_type], third_CG_75_results[4][test_type], third_G_75_results[2], save_file_name, GPU_name, CPU_name)
    save_file_name = abs_path("Third_test_MannWhitney_Full_LCT_GPU_CPUGPU_0.75.tex", True, False)
    createWilcoxonTableSmall(third_G_75_results[4][test_type], third_CG_75_results[4][test_type], third_G_75_results[2], save_file_name, GPU_name, CPU_name)
