import numpy as np
import matplotlib.pyplot as plt
import sys

if (len(sys.argv) != 4):
    sys.exit(1)

filename = "first_test_";

if (sys.argv[1] == "GPU"):
    filename += "GPU-";
elif (sys.argv[1] == "CPU"):
    filename += "CPUGPU-";
else:
    sys.exit(0)

CDT = int(sys.argv[2])
LCT = int(sys.argv[3])

if (not isinstance(CDT, int) or not isinstance(LCT, int)):
    sys.exit(2)

filename += str(CDT) + '-' + str(LCT) + ".txt"

Matrix = None
sums = [0] * 2
iterations = 0;
increase_iterations = 0;

with open(filename, 'r') as file:
    file.readline() # Skip firt line

    metadata = file.readline().split(",")
    iterations = int(metadata[0])
    increase_iterations = int(metadata[1])

    Matrix = [[0 for x in range(increase_iterations)] for y in range(3)]
    counter = 0
    completed_iter = 0
    for line in file:
        if (counter == 0):
            num_obstacles = int(line)
            counter += 1
        elif (counter == 11):
            sums[0] /= iterations
            sums[1] /= iterations
            Matrix[0][completed_iter] = sums[0]
            Matrix[1][completed_iter] = sums[1]
            Matrix[2][completed_iter] = num_obstacles
            sums[0] = 0
            sums[1] = 0

            counter = 0
            completed_iter += 1
        else:
            l = line.split(',')
            sums[0] += int(l[1])
            sums[1] += int(l[2][:-1])
            counter += 1


fig, ax = plt.subplots()

ind = np.arange(len(Matrix[0]))    # the x locations for the groups
width = 0.35         # the width of the bars
p1 = ax.bar(ind, Matrix[0], width, bottom=0)
p2 = ax.bar(ind + width, Matrix[1], width, bottom=0)

ax.set_title('Time to build CDT and LCT')
ax.set_xticks(ind + width / 2)
ax.set_xticklabels(Matrix[2])

ax.legend((p1[0], p2[0]), ('CDT', 'LCT'))
ax.set_ylabel('milliseconds')
ax.autoscale_view()

plt.show()
