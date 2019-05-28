import numpy as np
import matplotlib.pyplot as plt
import math
import sys

# process input arguments
if (len(sys.argv) != 4):
    print("provide three arguments [G,C] [START_NUM] [END_NUM]")
    sys.exit(1)

filename = "first_test_";

if (sys.argv[1] == "G" or sys.argv[1] == "g"):
    filename += "GPU-";
elif (sys.argv[1] == "C" or sys.argv[1] == "c"):
    filename += "CPUGPU-";
else:
    print("First argument has to be [Gg] or [Cc]]")
    sys.exit(1)

try:
    int(sys.argv[2])
except ValueError:
    print("Second argument has to be an integer")
    sys.exit(1)

try:
    int(sys.argv[3])
except ValueError:
    print("third argument has to be an integer")
    sys.exit(1)
# end processing of input

CDT = int(sys.argv[2])
LCT = int(sys.argv[3])
filename += str(CDT) + '-' + str(LCT) + ".txt"

Matrix = None
iterations = 0;
increase_iterations = 0;

with open(filename, 'r') as file:
    file.readline() # Skip firt line

    metadata = file.readline().split(",")
    iterations = int(metadata[0])
    increase_iterations = int(metadata[1])

    Matrix = [[0 for x in range(increase_iterations)] for y in range(5)]
    results = [[0 for x in range(iterations)] for y in range(2)]
    counter = 0
    completed_iter = 0
    for line in file:
        if (counter == 0):
            num_obstacles = int(line)
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

            Matrix[4][completed_iter] = num_obstacles
            counter = 0
            results = [[0 for x in range(iterations)] for y in range(2)]
            completed_iter += 1
        else:
            l = line.split(',')
            results[0][counter - 1] += int(l[1])
            results[1][counter - 1] += int(l[2][:-1])
            counter += 1

# plotting starts here
fig, ax = plt.subplots()

ind = np.arange(len(Matrix[0]))    # the x locations for the groups
width = 0.35         # the width of the bars
p1 = ax.bar(ind, Matrix[0], width, bottom=0, yerr=Matrix[1])
p2 = ax.bar(ind + width, Matrix[2], width, bottom=0, yerr=Matrix[3])

ax.set_title('Time to build CDT and LCT')
ax.set_xticks(ind + width / 2)
ax.set_xticklabels(Matrix[4])

ax.legend((p1[0], p2[0]), ('CDT', 'LCT'))
ax.set_ylabel('milliseconds')
ax.autoscale_view()

plt.show()
