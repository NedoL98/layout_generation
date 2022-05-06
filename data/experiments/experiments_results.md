50 epoch, generation size 3, 10000 tasks, 10 agents, single assigner.

|Dataset + Density|Baseline score|Genetic (0.05 learning rate)|Genetic (0.3 learning rate)|
|:-----:|:---------:|:---------:|:---------:|
|Small + `~0.35`|`0.0499079`|`0.039298` |`0.0452718`|
|Small + `~0.18`|`0.0528469`|`0.0541269`|`0.0545146`|
|Small + `~0.09`|`0.0572099`|`0.0578433`|`0.0581355`|
|Tiny  + `~0.17`|`0.0583761`|`0.0620355`|`0.0622940`|

50 epoch, generation size 3, 10000 tasks, 10 agents, three assigners.
|Dataset + Density|Baseline score|Genetic (0.05 learning rate)|Genetic (0.3 learning rate)|
|:-----:|:---------:|:---------:|:---------:|
|Small + `~0.35`|`0.0498666`|`0.0458017`|`0.0485222`|
|Small + `~0.18`|`0.0528648`|`0.0545007`|`0.0545007`|
|Small + `~0.09`|`0.0567153`|`0.0587739`|`0.0585897`|
|Tiny  + `~0.17`|`0.0583002`|`0.0617878`|`0.0617286`|

Different number of agents (50 epoch, generation size 3, 10000 tasks, three assigners).
|Dataset + Density|3 agents|5 agents|10 agents|15 agents|
|:-----:|:---------:|:---------:|:---------:|:---------:|
|Small + `~0.18`|`0.0569223`|`0.0565362`|`0.0545007`|`0.0530162`|
