# Silent Hill DS

Port WIP **sperimentale** di *Silent Hill* per Nintendo DS, basato sul [decomp di SlickAmogus](https://github.com/SlickAmogus/silent-hill-decomp).

**Stato attuale:** funzionano NitroFS, la galleria delle texture TIM e dei campioni audio VAB originali, la visualizzazione di TIM nel Native Bridge e il test delle primitive grafiche. **Il gioco originale non è ancora giocabile:** mancano l'integrazione del game loop, il renderer completo e il port ARM9 degli overlay.

## Compilazione

Con devkitPro/devkitARM configurato e gli asset della propria copia del gioco estratti in `assets/USA`:

```bat
prepare_scene.bat
```

Dopo la prima preparazione, dalla root del progetto:

```bat
make nds
```

ROM generata: `nds_port/silent_hill_ds_scene.nds`. Per la galleria separata: `prepare_gallery.bat --max-textures 96 --max-audio 96`.

**Nota:** ROM e asset originali non sono inclusi nel repository.
