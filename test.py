import random

items = [ # name, size)
    ("A", 2),    ("b", 3),    ("s", 5),    ("a", 5),
    ("hy", 7),    ("sdf", 8),    ("dsf", 9),    ("dff", 1),
    ("svf", 5),    ("fv", 3),    ("fsdfv", 6),    ("fg", 5),
    ("f", 7),    ("dfdgv", 8),    ("sv", 15),    ("dfv", 4),
    ("ASD", 3),    ("DFD", 5),    ("SDF", 7),    ("SDCF", 2),

]

random.shuffle(items)
groups = []
iters = sum(x[1] for x in items)
for i in range(int(iters/30)):
    group = []
    while sum(x[1] for x in group) < 30:
        group.append(items.pop())
    items.append(group.pop())
    groups.append(group)


iters = 5
for i in range(iters):
    random.shuffle(items)
    for n, group in enumerate(groups):
        gr = group
        while sum(x[1] for x in gr) < 30:
            gr.append(items.pop())
        items.append(gr.pop())
        groups[n] = gr



for n, group in enumerate(groups):
    print(sum(x[1] for x in group), group)

print("left over", items)
