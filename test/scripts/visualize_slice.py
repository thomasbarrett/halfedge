import matplotlib.pyplot as plt
import numpy as np
import glob
import os
import matplotlib.pyplot as plt
from matplotlib.contour import ContourSet

print_platform_width = 100
print_platform_depth = 100
fig, ax = plt.subplots(figsize=(20, 30))
plt.margins(0, 0)
plt.axis('off')

files = glob.glob('test/img/*')
for f in files:
    os.remove(f)

for i in range(300):
    ax.clear()
    ax.set_xlim(-print_platform_width/2, print_platform_width/2)
    ax.set_ylim(-print_platform_depth/2, print_platform_depth/2)
    ax.get_xaxis().set_visible(False)
    ax.get_yaxis().set_visible(False)
    
    polygons = glob.glob(f'test/csv/slice{i}_*.csv')

    if len(polygons) == 0:
        continue
        
    '''
    contours = list(map(lambda p: np.loadtxt(p).tolist(), polygons))
    commands = [list(map(lambda i: 1 if i == 0 else 2, range(len(contour)))) for contour in contours]
    cs = ContourSet(ax, [0, 1], [[np.concatenate(contours).tolist()]], [[np.concatenate(commands).tolist()]], filled=True)
    '''
    contours = list(map(lambda p: [np.loadtxt(p).tolist()], polygons))
    commands = [[list(map(lambda i: 1 if i == 0 else 2, range(len(contour[0]))))] for contour in contours]
    levels = list(range(len(contours)))
    cs = ContourSet(ax, levels, contours, commands, filled=False)
    '''
    for polygon in polygons:
        vertices = np.loadtxt(polygon)
        plt.fill(vertices[:,0], vertices[:,1], 'black')
    '''
    plt.subplots_adjust(left=0, right=1, top=1, bottom=0)
    plt.savefig(f'test/img/slice{i}.png')