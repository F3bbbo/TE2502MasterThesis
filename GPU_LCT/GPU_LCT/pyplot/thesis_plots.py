import numpy as np
import matplotlib.pyplot as plt
import math
import sys
import re
import os

num_args = len(sys.argv)
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
    -w / -workfolder "folder" set the workspace folder
    -i / -infolder "folder" set folder where input is
    -o / -outfolder "folder" set folder where output will go
    """
    print(options)
def all_plots():
    return dict({1 : True, 2 : True, 3 : True})

plots = all_plots()
in_folder = ""
out_folder = ""
work_folder = ""
out_file_name = "Result_plots"

def abs_path(fileName, is_input = True):
    global work_folder
    global in_folder
    global out_folder
    script_dir = os.path.dirname(__file__) #<-- absolute dir the script is in
    if(is_input):
        rel_path = os.path.join(in_folder,fileName)
    else:
        rel_path = os.path.join(out_folder, fileName)
    rel_path = os.path.join(work_folder, rel_path)
    abs_file_path = os.path.join(script_dir, rel_path)
    return abs_file_path;


def process_parameter(start_i):
    global plots
    global in_folder
    global out_folder
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
                in_folder = sys.argv[next_i]
                next_i += 1
            elif(argument == "o" or argument == "outfolder"):
                out_folder = sys.argv[next_i]
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
#print_help()
#print(plots)

# First plot
if(plots.get(1) is not None):

# Second plot


# Third plot
