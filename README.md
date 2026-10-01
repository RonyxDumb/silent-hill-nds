# Silent Hill DS

Port WIP **sperimentale** di *Silent Hill* per Nintendo DS, basato sul [decomp di SlickAmogus](https://github.com/SlickAmogus/silent-hill-decomp).

**Stato attuale:** la compilazione arriva a buon termine, ma il gioco ancora non riesce a raggiungere il MainLoop principale.

## Compilazione

Con devkitPro/devkitARM configurato e gli asset della propria copia del gioco estratti in `assets/USA` e `nds/nitrofs`:

```bat
nds/docker_build.bat
```
