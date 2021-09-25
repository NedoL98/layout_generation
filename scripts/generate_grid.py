with open("data/sorting_grid_small", "w") as f:
    ind = 0
    width = 4 * 3 + 1
    height = 10
    f.write("%d,%d\n" % (width, height))
    for w in range(width):
        for h in range(height):
            if h == 0 or h + 1 == height:
                f.write("%d,Induct,None,%d,%d,inf,inf,inf,1,1\n" % (ind, w, h))
            elif h % 2 == 0 and (w % 4) != 0:
                f.write("%d,Eject,None,%d,%d,inf,inf,inf,1,1\n" % (ind, w, h))
            else:
                f.write("%d,Travel,None,%d,%d,inf,inf,inf,1,1\n" % (ind, w, h))
            ind += 1