Requirements:
- [pygame 2](https://www.pygame.org/news)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) version 0.6.0 or above
- [cxxopts](https://github.com/jarro2783/cxxopts) (included in the source code)

Layout generation launch example with visualization:

```
build/layout_generation data/inputs/sorting_grid_small_full 100 0.2 3 1 10
python3 scripts/visualize_path.py data/inputs/sorting_grid_small_full data/best_assignment_epoch_3
```