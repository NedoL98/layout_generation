50 epoch, generation size 3, 10000 tasks, 10 agents, single assigner.

|Dataset + Density|Baseline score|Genetic (0.05 learning rate)|Genetic (0.3 learning rate)|
|:-----:|:---------:|:---------:|:---------:|
|Small + `~0.35`|`0.499079`|`0.39298` |`0.452718`|
|Small + `~0.18`|`0.528469`|`0.541269`|`0.545146`|
|Small + `~0.09`|`0.572099`|`0.578433`|`0.581355`|
|Tiny  + `~0.17`|`0.583761`|`0.620355`|`0.622940`|

50 epoch, generation size 3, 10000 tasks, 10 agents, three assigners.
|Dataset + Density|Baseline score|Genetic (0.05 learning rate)|Genetic (0.3 learning rate)|
|:-----:|:---------:|:---------:|:---------:|
|Small + `~0.35`|`0.498666`|`0.458017`|`0.485222`|
|Small + `~0.18`|`0.528648`|`0.548468`|`0.545007`|
|Small + `~0.09`|`0.567153`|`0.592483`|`0.587739`|
|Tiny  + `~0.17`|`0.583002`|`0.617878`|`0.617286`|

Different number of agents (50 epoch, generation size 3, 10000 tasks, three assigners).
|Dataset + Density|3 agents|5 agents|10 agents|15 agents|
|:-----:|:---------:|:---------:|:---------:|:---------:|
|Small + `~0.18`|`0.1707669`|`0.282681`|`0.5450070`|`0.795243`|
|Small + `~0.09`|`0.1822260`|`0.300329`|`0.5877390`|`0.861334`|
|Tiny  + `~0.17`|`0.2076300`|`0.328344`|`0.6172860`|`0.888895`|
