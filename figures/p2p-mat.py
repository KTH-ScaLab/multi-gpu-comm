"""
This script creates two figure files "p2p-mat-{bw,lat}.pdf", respectively
describing bandwidth and latency of for each pair of two GCDs.
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors

def load_str(s):
    return np.fromstring(s, sep=' ').reshape((8, 8))

# --- LOAD DATA ---
# TODO: load from CSV file

# CHANGE ME: place the tabulated bandwidth measurements (GB/s)
bw = load_str('''
1256.28	50.3	37.61	37.44	37.34	37.11	50.36	49.67
50.25	1297.68	37.43	37.61	37.31	37.58	49.75	49.09
37.63	37.34	1261.67	50.34	50.29	49.75	37.43	37.06
37.37	37.65	50.35	1314.23	49.74	49.15	37.32	37.64
37.4	37.07	50.3	49.68	1246.1	50.3	37.61	37.39
37.34	37.61	49.74	49	    50.23	1246.72	37.47	37.64
50.38	49.74	37.42	37.1	37.6	37.46	1263.74	50.25
49.74	49.23	37.36	37.63	37.45	37.64	50.34	1274.37
''')

# CHANGE ME: place the tabulated latency measurements (us)
lat = load_str('''
1.77	10.74	8.83	14.01	14.61	12.08	10.93	12.75
10.64	1.73	14.28	8.72	12.71	9.15	12.75	18.23
8.91	13.97	1.76	10.72	10.73	12.68	14.35	10.97
14.32	8.86	10.6	1.69	10.67	18.23	12.62	8.98
14.5	12.28	10.87	12.71	1.77	10.53	9.06	14.21
12.44	8.81	12.41	17.95	10.73	1.7	13.99	8.73
11.01	12.56	14.06	10.91	8.7	14.28	1.78	10.76
10.87	17.78	12.37	8.77	14.18	8.76	10.48	1.72
''')

np.fill_diagonal(bw, None)
np.fill_diagonal(lat, None)

# --- PLOT SETTINGS: titles, colors ---
mats   = [bw, lat]
labels = ['bw', 'lat']
titles = ['bandwidth (GB/s)', 'latency ($\mu s$)']
colors = ['Greens', 'Reds']

def truncate_colormap(cmap, minval=0.0, maxval=1.0, n=100):
    '''
    from https://stackoverflow.com/a/18926541
    '''
    if isinstance(cmap, str):
        cmap = plt.get_cmap(cmap)
    new_cmap = matplotlib.colors.LinearSegmentedColormap.from_list(
        'trunc({n},{a:.2f},{b:.2f})'.format(n=cmap.name, a=minval, b=maxval),
        cmap(np.linspace(minval, maxval, n)))
    return new_cmap

# --- PLOT ---
for (label, mat, title, color) in zip(labels, mats, titles, colors):
    fig = plt.figure(figsize=(3, 2.8), layout='constrained')
    ax = plt.gca()

    cmap = truncate_colormap(color, minval=0, maxval=.75)

    pos = ax.imshow(mat, cmap=cmap, origin='lower', vmin=np.min(mat[mat > 0]))
    fig.colorbar(pos, ax=ax)

    for (j, i), n in np.ndenumerate(mat):
        if i == j: continue

        ax.text(i, j, f'{n:.0f}', # #.2g
                ha='center', va='center')

    ax.set_xticks(range(8))
    ax.set_yticks(range(8))
    ax.set_xlabel('GCD')

    ax.tick_params(axis='both', which='both',
                   top=False, bottom=False, left=False,
                   labelbottom=True, labelleft=True)

    ax.set_ylabel('GCD')
    plt.title(title)

    fname = f'p2p-mat-{label}.pdf'
    plt.savefig(fname)
    print(f'saved "{title}" as "{fname}"')