To Run any current builds of Lithtech
-------------------------------------

* install DXVK-native (1.9.x) see [build_dxvk-native](https://gitlab.com/Katana-Steel/lith_docker/-/blob/master/build_dxvk.sh)
* grab a recent build from [Gitlab Pipelines](https://gitlab.com/Katana-Steel/lithtech/-/pipelines)
* extract the archive
* grab the `tests/create_nolf2.sh` script
* run it like this: `tests/create_nolf2.sh ~/Games/NOLF2-location build`
* copy and/or link **rez**, **cfg** and **profile** files from your NOLF2 install
* finally run `./Lithtech`
