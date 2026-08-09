#!/usr/bin/env python3
# Silahlarin firlattigi kovan sprite'lari.
#
# Kovanlar oyunda ~2 pixel yuksekliginde cizilir: AK47_Single001 27x7, ak47_clip_001 5x4. Bu olcekte govde
# uzerinde detay yeri yoktur, okunurlugu tasiyan tek sey ust satirin acik, alt satirin koyu olmasidir -- yani
# silindirik bir yuzeyin isik aldigi izlenimi. Kovan zaten SpinSpeed ile donduru mekten dolayi tek frame yeter;
# animasyon eklense her frame'i dondurulmus 2 pixel olurdu ve fark gorunmezdi.
#
# Calistir: python3 tools/shell_casing_gen.py
import os
from PIL import Image

OUT = "/home/selviniah/CLionProjects/EnterTheGungeonClone/Resources/Guns"


def write(path, rows):
    """rows: ust satirdan alta, her hucre (r,g,b) veya None (saydam)."""
    h = len(rows)
    w = len(rows[0])
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    px = img.load()
    for y, row in enumerate(rows):
        for x, c in enumerate(row):
            if c is not None:
                px[x, y] = (*c, 255)

    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path)
    print(f"{path}  {w}x{h}")


# Pirinc: ust satir isik alir, alt satir govdenin golgesidir. Uclar (rim ve agiz) her iki satirda da koyudur,
# boylece kovanin nerede bitip nerede basladigi 2 pixel icinde bile okunur.
RIM_LIT = (150, 112, 44)
RIM_DARK = (116, 84, 30)
BODY_LIT = (238, 208, 132)
BODY_MID = (214, 172, 80)
BODY_DARK = (188, 144, 60)
BODY_SHADE = (150, 112, 44)
MOUTH_LIT = (128, 92, 34)   # acik uc: icerisi bos, yani govdeden koyu
MOUTH_DARK = (98, 70, 26)
PRIMER_LIT = (188, 108, 56)  # revolver kovaninin dibindeki bakir kapsul
PRIMER_DARK = (146, 80, 40)

# AK47: 7.62x39, tufek kovani oldugundan uzun. Soldan saga rim -> govde -> govde -> agiz.
write(f"{OUT}/AK47/AK47Shell.png", [
    [RIM_LIT, BODY_LIT, BODY_MID, MOUTH_LIT],
    [RIM_DARK, BODY_DARK, BODY_SHADE, MOUTH_DARK],
])

# RogueSpecial: revolver kovani, tufeginkinden bir pixel kisa. Dipteki bakir kapsul onu AK'ninkinden ayirir;
# ikisi ayni yerde birikirse hangi silahin biraktigi belli olur.
write(f"{OUT}/RogueSpecial/RogueSpecialShell.png", [
    [PRIMER_LIT, BODY_LIT, MOUTH_LIT],
    [PRIMER_DARK, BODY_DARK, MOUTH_DARK],
])
