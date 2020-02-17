import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

fig, ax = plt.subplots()
ax.set_xlim(-1, 1)
ax.set_ylim(-1, 1)
ln, = plt.plot([], [], 'o', markersize=1)

def update(i):
    slice_vertices = np.loadtxt(f'test/csv/slice{i}.csv')
    if slice_vertices.shape[0] != 0:
        xmin, ymin = np.amin(slice_vertices, axis=0)
        xmax, ymax = np.amax(slice_vertices, axis=0)
        ax.set_xlim(xmin, xmax)
        ax.set_ylim(ymin, ymax)
        ln.set_data(slice_vertices[:,0], slice_vertices[:,1])
    return ln,


ani = animation.FuncAnimation(fig, update, frames=range(300))
plt.show()