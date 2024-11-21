"""
This script creates a figure file "p2p-adjacency.pdf" describing the matrix
of length of shortest path between pairs of two GCDs.
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches
import matplotlib.colors
from scipy.sparse.csgraph import dijkstra

# --- Define adjacency matrix ---
N_GCD = 8

# adjacency matrix, weight is the bandwidth of the connection (1x = 50+50 GB/s)
adj = np.zeros((N_GCD, N_GCD), dtype=int)
adj[0, 1] = adj[2, 3] = adj[4, 5] = adj[6, 7] = 4                         # 4x links
adj[0, 6] = adj[2, 4] = 2                                                 # 2x links
adj[0, 2] = adj[1, 5] = adj[1, 3] = adj[3, 7] = adj[4, 6] = adj[5, 7] = 1 # 1x links
adj += adj.T # make matrix symmetric

# (unused) weight of the weight-maximizing path for each GCD pair
bw = np.zeros((N_GCD, N_GCD), dtype=int)
bw[0, 1] = bw[2, 3] = bw[4, 5] = bw[6, 7] = 4                       # 4x links
bw[0, 6] = bw[2, 4] = bw[1, 7] = bw[0, 7] = bw[3, 4] = bw[2, 5] = 2 # 2x links
bw += bw.T # make matrix symmetric
bw[bw == 0] = 1                                                     # 1x links

# we use Dijsktra algorithm to compute length of shortest path for each pair
hops = dijkstra(adj, unweighted=1)

# --- PLOT SETTINGS: labels, colors ---
values = [0, 1, 2]
labels = ['self', 'direct', 'two hops']
colors = ['lightgray', 'gray', 'white']

cmap = matplotlib.colors.ListedColormap(colors)

fig = plt.figure(figsize=(3, 2.8), layout='constrained')
im  = plt.imshow(hops, origin='lower', cmap=cmap)

colors  = [ im.cmap(im.norm(value)) for value in values]
patches = [ matplotlib.patches.Patch(color=colors[i], label=f"{v}",
                                     ec='black', lw=.75)
            for i, v in enumerate(labels) ]

p = matplotlib.patches.Patch(facecolor=(0,0,0,0), label=f"Distance: ", lw=0)

leg = plt.legend(handles=[p] + patches[1:],
           handlelength=0.9, handleheight=1,
           loc='lower center',
           bbox_to_anchor=(0.5, 1), ncols=3,
           borderaxespad=0.5, columnspacing=0.7, framealpha=0.0)

for vpack in leg._legend_handle_box.get_children()[:1]:
    for hpack in vpack.get_children():
        hpack.get_children()[0].set_width(0)

plt.ylabel('GCD')
plt.xlabel('GCD')
plt.xticks(range(8))
plt.yticks(range(8))

plt.tick_params(axis='both', which='both',
                   top=False, bottom=False, left=False,
                   labelbottom=True, labelleft=True)

# --- SAVE ---
fname = f'p2p-adjacency.pdf'
plt.savefig(fname)
print(f'saved "adjacency" as "{fname}"')