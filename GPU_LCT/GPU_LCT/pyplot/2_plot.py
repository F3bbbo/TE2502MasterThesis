import numpy as np
import matplotlib.pyplot as plt
import math
import sys

# process input arguments
if (len(sys.argv) != 2):
    print("provide one argument [NUM]")
    sys.exit(1)

try:
    int(sys.argv[1])
except ValueError:
    print("Argument has to be an integer")
    sys.exit(1)

filename = "second_test-" + str(sys.argv[1]) + ".txt"

iterations = None
num_vertices = None
num_CDT_shaders = 7
num_LCT_shaders = 9
input_data = None
results = [[0 for x in range(num_CDT_shaders + num_LCT_shaders)] for y in range(2)]
with open(filename, 'r') as file:
    file.readline()
    iterations = int(file.readline()[:-1])
    num_vertices = file.readline()[:-1]
    input_data = [[0 for x in range(iterations)] for y in range(num_CDT_shaders + num_LCT_shaders)]
    for i in range(iterations):
        shader_times = file.readline().split(",")
        counter = 0
        for shader_time in shader_times:
            if (counter == num_CDT_shaders + num_LCT_shaders - 1):
                input_data[counter][i] = int(shader_time[:-1])
            else:
                input_data[counter][i] = int(shader_time)
            counter += 1

# compute means
for shader in range(num_CDT_shaders + num_LCT_shaders):
    for time in range(iterations):
        results[0][shader] += input_data[shader][time]
    results[0][shader] /= iterations

# compute standard deviations
for shader in range(num_CDT_shaders + num_LCT_shaders):
    for time in range(iterations):
        results[1][shader] += (input_data[shader][time] - results[0][shader]) ** 2
    results[1][shader] = math.sqrt(results[1][shader] / iterations)

# plotting starts here
fig, axs = plt.subplots(1, 2)

plt.suptitle('Construction of LCT with ' + str(num_vertices) + ' vertices')
# CDT plotting
ind = np.arange(num_CDT_shaders)    # the x locations for the groups
width = 0.75         # the width of the bars
colors = ["green", "yellow", "blue", "blue", "red", "red", "red"]
p1 = axs[0].bar(ind, results[0][:num_CDT_shaders], width, bottom=0, yerr=results[1][:num_CDT_shaders], edgecolor=colors, color=colors)

axs[0].set_title('Mean run time of CDT construction parts')
axs[0].set_xticks(ind)
axs[0].set_xticklabels([1, 1, 1, 2, 1, 2, 3])
axs[0].set_xlabel('index to shader kernel in each part')

axs[0].legend((p1[0], p1[1], p1[3], p1[4]), ('Locate step', 'insertion step', 'marking step', 'flipping step'))
axs[0].set_ylabel('milliseconds')
axs[0].autoscale_view()

# LCT plotting
ind = np.arange(num_LCT_shaders)    # the x locations for the groups
width = 0.75         # the width of the bars
colors = ["purple", "purple", "green", "green", "yellow", "red", "red", "red", "red"]
p1 = axs[1].bar(ind, results[0][-(num_LCT_shaders):], width, bottom=0, yerr=results[1][-(num_LCT_shaders):], edgecolor=colors, color=colors)

axs[1].set_title('Mean run time of LCT construction parts')
axs[1].set_xticks(ind)
axs[1].set_xticklabels([1, 2, 1, 2, 1, 1, 1, 2, 3])
axs[1].set_xlabel('index to shader kernel in each part')

axs[1].legend((p1[0], p1[2], p1[4], p1[6]), ('Locate disturbances step', 'selection step', 'insertion step', 'flipping step'))
axs[1].set_ylabel('milliseconds')
axs[1].autoscale_view()

plt.show()
