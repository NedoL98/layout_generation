Requirements:
- [pygame 2](https://www.pygame.org/news)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) version 0.6.0 or above
- [cxxopts](https://github.com/jarro2783/cxxopts) (included in the source code)

Layout generation launch example with visualization:

```
build/layout_generation data/inputs/sorting_grid_small_full -s 100 -r 0.2 -a 3 -c 1 -e 10 -p 0.3 -h 3
python3 scripts/visualize_path.py data/inputs/sorting_grid_small_full data/best_assignment_epoch_3
```