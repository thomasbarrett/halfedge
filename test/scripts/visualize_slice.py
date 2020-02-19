import matplotlib.pyplot as plt
import numpy as np
import glob

print_platform_width = 2
print_platform_depth = 3
fig, ax = plt.subplots(figsize=(2, 3))
plt.margins(0, 0)
plt.axis('off')

for i in range(300):
    ax.clear()
    ax.set_xlim(-print_platform_width/2, print_platform_width/2)
    ax.set_ylim(-print_platform_depth/2, print_platform_depth/2)
    ax.get_xaxis().set_visible(False)
    ax.get_yaxis().set_visible(False)
    
    
    polygons = glob.glob(f'test/csv/slice{i}_*.csv')
    for polygon in polygons:
        vertices = np.loadtxt(polygon)
        plt.fill(vertices[:,0], vertices[:,1], 'black')
    
    plt.subplots_adjust(left=0, right=1, top=1, bottom=0)
    plt.savefig(f'test/img/slice{i}.png')