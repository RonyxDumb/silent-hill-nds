from pathlib import Path

src = Path('/src/src/bodyprog/gfx/bodyprog_80040B74.c')
if not src.exists():
    raise SystemExit('ERRORE: bodyprog_80040B74.c non trovato.')

text = src.read_text(encoding='utf-8-sig', errors='replace')
backup = src.with_suffix('.c.before_nds_v16')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

old = '''#ifdef SH_PC_PORT
    } /* close else block */
#endif

    #undef CHUNK_SUBCELL_SIZE
}'''

new = '''    } /* close shared chunk-draw block */

    #undef CHUNK_SUBCELL_SIZE
}'''

if new not in text:
    if old not in text:
        raise SystemExit('ERRORE: chiusura Ipd_ChunkDraw non riconosciuta.')
    text = text.replace(old, new, 1)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
if '    } /* close shared chunk-draw block */' not in out:
    raise SystemExit('ERRORE: patch v16 non applicata.')

print('NDS Ipd_ChunkDraw brace v16 patch applicata')
print('  chiusura blocco condiviso: non piu esclusiva SH_PC_PORT')
print('  logica di rendering: invariata')
