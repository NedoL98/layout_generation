Requirements:
- [pygame 2](https://www.pygame.org/news)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) version 0.6.0 or above

Layout generation launch example with visualization:

```
build/layout_generation data/inputs/sorting_grid_small_full 100 0.2 3 > data/sample_output
python3 scripts/visualize_path.py data/inputs/sorting_grid_small_full data/sample_output
```