import matplotlib.pyplot as plt
import numpy as np

for i in range(300):
    slice_vertices = np.loadtxt(f'test/csv/slice{i}.csv')
    if slice_vertices.shape[0] != 0:
        plt.plot(slice_vertices[:,0], slice_vertices[:,1], 'o')
        plt.savefig(f'test/img/slice{i}.png')
