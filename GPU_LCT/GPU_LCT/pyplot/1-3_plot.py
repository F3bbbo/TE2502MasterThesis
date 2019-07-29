import numpy as np
import matplotlib.pyplot as plt
import math
import sys

# process input arguments
if (len(sys.argv) != 6):
    print("provide five arguments [1,3] [C,GC,G] [START_NUM] [END_NUM] [0,1,2]")
    sys.exit(1)

# first parameter
try:
    int(sys.argv[1])
except ValueError:
    print("First argument has to be an integer")
    sys.exit(1)

if (not (sys.argv[1] == "1" or sys.argv[1] == "3")):
    print("First argument has to be 1 or 3")
    sys.exit(1)

filename = None
type = int(sys.argv[1])
if (type == 1):
    filename = "first_test_";
else:
    filename = "third_test_";

# second parameter
if (sys.argv[2] == "C" or sys.argv[2] == "c"):
    filename += "CPU-";
elif (len(sys.argv[2]) == 2 and (sys.argv[2][0] == "G" or sys.argv[2][0] == "g") and (sys.argv[2][1] == "C" or sys.argv[2][1] == "c")):
    filename += "CPUGPU-";
elif (sys.argv[2] == "G" or sys.argv[2] == "g"):
    filename += "GPU-";
else:
    print("Second argument has to be [Cc], [Gg,Cc] or [Gg]]")
    sys.exit(1)

# third parameter
try:
    int(sys.argv[3])
except ValueError:
    print("Third argument has to be an integer")
    sys.exit(1)

CDT = int(sys.argv[3])

# fourth parameter
try:
    int(sys.argv[4])
except ValueError:
    print("fourth argument has to be an integer")
    sys.exit(1)

LCT = int(sys.argv[4])

# fifth parameter
try:
    int(sys.argv[5])
except ValueError:
    print("fifth argument has to be an integer")
    sys.exit(1)

version = str(sys.argv[5])
# end processing of input
filename += str(CDT) + '-' + str(LCT) + "-v" + version + ".txt"

Matrix = None
xlabels = None
iterations = 0;
increase_iterations = 0;

with open(filename, 'r') as file:
    file.readline() # Skip firt line

    metadata = file.readline().split(",")
    iterations = int(metadata[0])
    increase_iterations = int(metadata[1])

    Matrix = [[0 for x in range(increase_iterations)] for y in range(4)]
    results = [[0 for x in range(iterations)] for y in range(2)]
    xlabels = [""] * increase_iterations
    counter = 0
    completed_iter = 0
    for line in file:
        if (counter == 0):
            num_vertices = line
            counter += 1

        elif (counter > iterations):
            # compute CDT mean
            for result in results[0]:
                Matrix[0][completed_iter] += result
            Matrix[0][completed_iter] /= iterations

            # compute CDT standard deviation
            for result in results[0]:
                Matrix[1][completed_iter] += (result - Matrix[0][completed_iter]) ** 2
            Matrix[1][completed_iter] = math.sqrt(Matrix[1][completed_iter] / iterations)

            # compute LCT mean
            for result in results[1]:
                Matrix[2][completed_iter] += result
            Matrix[2][completed_iter] /= iterations

            # compute LCT standard deviation
            for result in results[1]:
                Matrix[3][completed_iter] += (result - Matrix[2][completed_iter]) ** 2
            Matrix[3][completed_iter] = math.sqrt(Matrix[3][completed_iter] / iterations)

            xlabels[completed_iter] = num_vertices
            counter = 0
            results = [[0 for x in range(iterations)] for y in range(2)]
            completed_iter += 1
        else:
            l = line.split(',')
            results[0][counter - 1] += int(l[0])
            results[1][counter - 1] += int(l[1][:-1])
            counter += 1

# plotting starts here
fig, ax = plt.subplots()

ind = np.arange(len(Matrix[0]))    # the x locations for the groups
width = 0.35         # the width of the bars
p1 = ax.bar(ind, Matrix[0], width, bottom=0, yerr=Matrix[1], edgecolor="blue")
p2 = ax.bar(ind + width, Matrix[2], width, bottom=0, yerr=Matrix[3], edgecolor="orange")

ax.set_title('Time to build CDT and LCT using version ' + version)
ax.set_xticks(ind + width / 2)
ax.set_xticklabels(xlabels)
if (type == 1):
    ax.set_xlabel('Number of inserted vertices')
else:
    ax.set_xlabel('number of already inserted vertices, number of additional vertices inserted')

ax.legend((p1[0], p2[0]), ('CDT', 'LCT'))
ax.set_ylabel('milliseconds')
ax.autoscale_view()

plt.show()
