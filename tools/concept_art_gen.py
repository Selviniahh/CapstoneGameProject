#!/usr/bin/env python3
# Enter the Gungeon tarzi front-idle konsept karakterleri
import os
from PIL import Image

OUT = "/home/selviniah/CLionProjects/EnterTheGungeonClone/Resources/Player/Idle/Front"
SCRATCH = "/tmp/claude-1000/-home-selviniah/294678cb-b4ca-4d78-bccf-0bcfdb6abdd2/scratchpad"


def mix(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


def dark(c, t=0.22):
    return mix(c, (18, 10, 24), t)


def lite(c, t=0.22):
    return mix(c, (255, 250, 235), t)


class Cv:
    def __init__(self, w, h, outline):
        self.w, self.h = w, h
        self.p = {}
        self.noshade = set()
        self.outline = outline

    def s(self, x, y, c, shade=True):
        if 0 <= x < self.w and 0 <= y < self.h and c is not None:
            self.p[(x, y)] = c
            (self.noshade.discard if shade else self.noshade.add)((x, y))

    def rect(self, x0, y0, x1, y1, c, shade=True):
        for y in range(min(y0, y1), max(y0, y1) + 1):
            for x in range(min(x0, x1), max(x0, x1) + 1):
                self.s(x, y, c, shade)

    def row(self, x0, x1, y, c, shade=True):
        self.rect(x0, y, x1, y, c, shade)

    def col(self, x, y0, y1, c, shade=True):
        self.rect(x, y0, x, y1, c, shade)

    def ell(self, cx, cy, rx, ry, c, shade=True):
        for y in range(cy - ry, cy + ry + 1):
            for x in range(cx - rx, cx + rx + 1):
                dx = (x + 0.5 - cx) / (rx + 0.5)
                dy = (y + 0.5 - cy) / (ry + 0.5)
                if dx * dx + dy * dy <= 1.0:
                    self.s(x, y, c, shade)

    def tri(self, cx, base_y, h, half, c, up=True, shade=True):
        for i in range(h):
            y = base_y - i if up else base_y + i
            hw = max(0, int(round(half * (1 - i / float(h)))))
            self.row(cx - hw, cx + hw - 1, y, c, shade)

    def clear(self, x0, y0, x1, y1):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.p.pop((x, y), None)
                self.noshade.discard((x, y))

    def autoshade(self):
        src = dict(self.p)
        for (x, y), c in src.items():
            if (x, y) in self.noshade:
                continue
            f = 0.0
            if src.get((x, y + 1)) != c:
                f += 0.13
            if src.get((x + 1, y)) != c:
                f += 0.08
            if src.get((x, y - 1)) is None and src.get((x - 1, y)) is None:
                self.p[(x, y)] = lite(c, 0.12)
            elif f > 0:
                self.p[(x, y)] = dark(c, min(f, 0.2))

    def autooutline(self):
        add = {}
        for (x, y) in list(self.p.keys()):
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                n = (x + dx, y + dy)
                if n not in self.p and 0 <= n[0] < self.w and 0 <= n[1] < self.h:
                    add[n] = self.outline
        self.p.update(add)

    def img(self):
        im = Image.new("RGBA", (self.w, self.h), (0, 0, 0, 0))
        px = im.load()
        for (x, y), c in self.p.items():
            px[x, y] = (c[0], c[1], c[2], 255)
        return im


# ---------------- ortak parcalar ----------------
def head(c, cx, top, w, h, col, r=2, shade=True):
    x0, x1 = cx - w // 2, cx - w // 2 + w - 1
    for y in range(top, top + h):
        d = min(y - top, top + h - 1 - y)
        ins = max(0, r - d)
        c.row(x0 + ins, x1 - ins, y, col, shade)
    return x0, x1


def torso(c, cx, top, bot, w, col, shoulder=1):
    x0 = cx - w // 2
    c.rect(x0, top, x0 + w - 1, bot, col)
    if shoulder:                                   # omuz yuvarlatma
        c.s(x0, top, None)
        for p in ((x0, top), (x0 + w - 1, top)):
            c.p.pop(p, None)


def legs(c, cx, top, bot, pant, boot=None, lw=3, gap=2):
    lx = cx - gap // 2 - lw
    rx = cx + (gap - gap // 2)
    for x0 in (lx, rx):
        c.rect(x0, top, x0 + lw - 1, bot, pant)
    if boot:
        for x0 in (lx, rx):
            c.rect(x0 - 1, bot - 1, x0 + lw - 1, bot, boot)


def hands(c, xl, xr, y, col, r=1):
    c.ell(xl, y, r, r, col)
    c.ell(xr, y, r, r, col)


def eyes(c, cx, y, gap, col=(26, 20, 26), w=2, h=2, glint=(250, 250, 250)):
    lx, rx = cx - gap // 2 - w, cx + (gap - gap // 2)
    c.rect(lx, y, lx + w - 1, y + h - 1, col, False)
    c.rect(rx, y, rx + w - 1, y + h - 1, col, False)
    if glint:
        c.s(lx, y, glint, False)
        c.s(rx, y, glint, False)


CHARS = []


def ch(fn):
    CHARS.append(fn)
    return fn


# ============ 01 Kapusonlu nisanci ============
@ch
def c01():
    c = Cv(24, 30, (30, 22, 18))
    cx = 12
    hood, hood2 = (62, 110, 72), (44, 82, 56)
    tunic, skin = (92, 74, 52), (240, 200, 152)
    c.rect(cx - 8, 16, cx + 7, 24, hood2)                 # arkadaki pelerin
    c.row(cx - 7, cx + 6, 25, hood2)
    legs(c, cx, 24, 28, (76, 60, 46), (46, 34, 28))
    torso(c, cx, 18, 24, 11, tunic)
    c.rect(cx - 2, 18, cx + 1, 24, (152, 124, 84))        # gogus paneli
    c.row(cx - 5, cx + 4, 22, (52, 40, 30))               # kemer
    head(c, cx, 3, 15, 13, skin, 2)                       # yuz
    head(c, cx, 2, 17, 9, hood, 3)                        # kapuson tepesi
    c.rect(cx - 8, 8, cx - 6, 17, hood)                   # kapuson yanlari
    c.rect(cx + 5, 8, cx + 7, 17, hood)
    c.rect(cx - 6, 10, cx + 5, 10, dark(hood, 0.45))      # yuz golgesi
    c.rect(cx - 6, 16, cx + 5, 17, hood2)                 # yakadaki kumas
    eyes(c, cx, 12, 4, (48, 36, 28), 2, 2, (252, 246, 220))
    c.row(cx - 1, cx, 15, (196, 156, 114), False)
    hands(c, cx - 8, cx + 7, 21, tunic)
    return c, "hooded_ranger"


# ============ 02 Sovalye ============
@ch
def c02():
    c = Cv(24, 30, (26, 24, 34))
    cx = 12
    st, st2 = (172, 180, 194), (116, 126, 146)
    legs(c, cx, 24, 28, st2, (66, 70, 84))
    torso(c, cx, 17, 24, 13, st)
    c.rect(cx - 7, 17, cx - 4, 20, lite(st, 0.28))        # omuz plakalari
    c.rect(cx + 3, 17, cx + 6, 20, lite(st, 0.28))
    c.row(cx - 6, cx + 5, 22, (148, 104, 44))             # kemer
    c.rect(cx - 1, 22, cx, 22, (232, 196, 84), False)
    c.col(cx - 1, 18, 21, st2)
    head(c, cx, 3, 15, 12, st, 3)                          # migfer
    c.row(cx - 6, cx + 5, 3, lite(st, 0.3))
    c.tri(cx, 2, 4, 2, (206, 54, 48), True, False)         # sorguc
    c.rect(cx - 1, 0, cx, 1, (206, 54, 48), False)
    c.rect(cx - 6, 9, cx + 5, 11, (26, 24, 32), False)     # T vizor
    c.rect(cx - 1, 9, cx, 14, (26, 24, 32), False)
    c.s(cx - 5, 10, (110, 206, 224), False)
    c.s(cx + 4, 10, (110, 206, 224), False)
    c.row(cx - 5, cx + 4, 13, st2)
    hands(c, cx - 9, cx + 8, 20, st2)
    return c, "iron_knight"


# ============ 03 Buyucu ============
@ch
def c03():
    c = Cv(26, 34, (30, 22, 46))
    cx = 13
    robe, robe2 = (78, 68, 176), (54, 46, 130)
    skin, gold = (238, 196, 152), (224, 184, 68)
    for i, y in enumerate(range(20, 32)):                  # koni cuppe
        w = 9 + i
        c.row(cx - w // 2, cx - w // 2 + w - 1, y, robe if i % 3 else robe2)
    c.row(cx - 7, cx + 6, 32, robe2)
    c.row(cx - 5, cx + 4, 20, gold)                        # kusak
    head(c, cx, 9, 14, 12, skin, 2)
    c.tri(cx, 8, 9, 6, robe, True)                         # sivri sapka
    c.s(cx - 1, 0, gold, False)
    c.s(cx, 0, gold, False)
    c.rect(cx - 9, 7, cx + 8, 8, robe2)                    # sapka kenari
    c.row(cx - 8, cx + 7, 6, robe)
    c.row(cx - 7, cx + 6, 9, dark(robe2, 0.3))
    eyes(c, cx, 13, 4, (46, 62, 138), 2, 2, (204, 240, 255))
    c.rect(cx - 4, 16, cx + 3, 19, (238, 238, 246))        # sakal
    c.tri(cx, 22, 3, 3, (238, 238, 246), False)
    hands(c, cx - 9, cx + 8, 22, skin)
    return c, "arcane_wizard"


# ============ 04 Silahsor ============
@ch
def c04():
    c = Cv(28, 30, (44, 26, 18))
    cx = 14
    hat, hat2 = (112, 78, 46), (82, 56, 34)
    skin, pon = (238, 192, 144), (182, 96, 62)
    legs(c, cx, 23, 28, (86, 68, 52), (52, 38, 30))
    for i, y in enumerate(range(16, 25)):                  # ponco
        w = 13 + (i // 3)
        c.row(cx - w // 2, cx - w // 2 + w - 1, y, pon)
    c.row(cx - 7, cx + 6, 19, (152, 72, 48))
    c.row(cx - 7, cx + 6, 22, (212, 128, 84))
    c.rect(cx - 2, 16, cx + 1, 18, (240, 232, 210))        # yaka
    head(c, cx, 3, 15, 13, skin, 2)
    c.rect(cx - 6, 12, cx + 5, 15, (194, 66, 54))          # bandana
    c.row(cx - 5, cx + 4, 12, (218, 84, 66))
    eyes(c, cx, 9, 4, (54, 34, 26), 2, 2, (250, 246, 226))
    c.row(cx - 5, cx + 4, 7, (176, 138, 100))              # sapka golgesi
    c.rect(cx - 11, 5, cx + 10, 6, hat)                    # genis kenar
    c.row(cx - 10, cx + 9, 7, hat2)
    c.rect(cx - 5, 1, cx + 4, 4, hat)                      # taci
    c.row(cx - 4, cx + 3, 0, hat)
    c.row(cx - 5, cx + 4, 4, hat2)
    hands(c, cx - 10, cx + 9, 21, (92, 70, 52))
    return c, "gunslinger"


# ============ 05 Siborg ============
@ch
def c05():
    c = Cv(24, 30, (22, 26, 34))
    cx = 12
    m1, m2, glow = (152, 160, 172), (98, 106, 120), (255, 96, 62)
    legs(c, cx, 23, 28, m2, (56, 60, 72), 3, 3)
    torso(c, cx, 16, 23, 13, m1)
    c.rect(cx - 6, 16, cx + 5, 17, m2)
    c.rect(cx - 3, 19, cx + 2, 22, (44, 50, 60))
    c.rect(cx - 2, 20, cx + 1, 21, glow, False)
    head(c, cx, 3, 15, 12, m1, 2)
    c.rect(cx - 6, 8, cx + 5, 10, (36, 40, 50), False)     # goz bandi
    c.rect(cx - 5, 8, cx - 1, 10, glow, False)             # tarayici
    c.s(cx - 5, 8, lite(glow, 0.55), False)
    c.rect(cx - 6, 12, cx + 1, 14, (44, 50, 60))           # cene plakasi
    for x in range(cx - 5, cx + 1, 2):
        c.col(x, 13, 13, m2, False)
    c.col(cx + 6, 5, 7, m2)                                # anten
    c.col(cx + 6, 1, 4, m2)
    c.s(cx + 6, 0, glow, False)
    hands(c, cx - 9, cx + 8, 20, m2)
    return c, "cyborg_unit"


# ============ 06 Slime ============
@ch
def c06():
    c = Cv(26, 24, (22, 58, 38))
    cx = 13
    g1, g2 = (100, 218, 132), (60, 172, 96)
    c.ell(cx, 13, 10, 8, g1)
    c.rect(cx - 10, 13, cx + 9, 20, g1)
    c.row(cx - 11, cx + 10, 21, g1)
    c.row(cx - 10, cx + 9, 22, g2)
    c.ell(cx, 7, 7, 6, g1)
    c.rect(cx - 5, 16, cx + 4, 19, g2)                     # icteki cekirdek
    c.rect(cx - 3, 15, cx + 2, 15, g2)
    eyes(c, cx, 7, 5, (26, 48, 34), 3, 3, (240, 255, 244))
    c.row(cx - 2, cx + 1, 11, (26, 48, 34), False)
    c.s(cx - 3, 12, (26, 48, 34), False)
    c.s(cx + 2, 12, (26, 48, 34), False)
    c.rect(cx - 7, 3, cx - 5, 4, (216, 255, 228), False)   # parlama
    c.s(cx - 7, 5, (216, 255, 228), False)
    hands(c, cx - 11, cx + 10, 15, g1)
    return c, "slime_blob"


# ============ 07 Hayalet ============
@ch
def c07():
    c = Cv(24, 30, (40, 44, 84))
    cx = 12
    gh, gh2 = (200, 212, 244), (150, 164, 208)
    head(c, cx, 3, 17, 14, gh, 4)
    c.rect(cx - 7, 16, cx + 6, 23, gh)
    for i, y in enumerate(range(24, 28)):                  # yirtik etek
        c.row(cx - 7 + i, cx + 6 - i, y, gh2)
    c.rect(cx - 6, 24, cx - 5, 27, gh2)
    c.rect(cx + 4, 24, cx + 5, 26, gh2)
    c.rect(cx - 1, 24, cx, 28, gh2)
    c.rect(cx - 7, 19, cx + 6, 21, gh2)
    c.rect(cx - 6, 8, cx - 3, 12, (46, 50, 96), False)     # oyuk gozler
    c.rect(cx + 2, 8, cx + 5, 12, (46, 50, 96), False)
    c.rect(cx - 6, 8, cx - 5, 9, (140, 224, 255), False)
    c.rect(cx + 2, 8, cx + 3, 9, (140, 224, 255), False)
    c.rect(cx - 2, 14, cx + 1, 16, (46, 50, 96), False)    # agiz
    hands(c, cx - 10, cx + 9, 19, gh)
    return c, "wraith"


# ============ 08 Mantar adam ============
@ch
def c08():
    c = Cv(28, 28, (56, 28, 28))
    cx = 14
    cap, spot, stalk = (208, 62, 58), (246, 240, 220), (238, 226, 198)
    legs(c, cx, 22, 26, stalk, (200, 184, 152), 3, 3)
    torso(c, cx, 14, 22, 12, stalk)
    c.rect(cx - 6, 18, cx + 5, 19, (216, 200, 168))
    c.ell(cx, 8, 12, 7, cap)                               # sapka
    c.row(cx - 11, cx + 10, 13, dark(cap, 0.3))
    c.rect(cx - 3, 2, cx + 2, 3, spot, False)
    c.rect(cx - 9, 6, cx - 6, 8, spot, False)
    c.rect(cx + 5, 5, cx + 8, 7, spot, False)
    c.rect(cx - 2, 10, cx + 1, 11, spot, False)
    eyes(c, cx, 15, 5, (66, 44, 38), 2, 2, None)
    c.row(cx - 1, cx, 18, (66, 44, 38), False)
    hands(c, cx - 9, cx + 8, 18, stalk)
    return c, "myconid"


# ============ 09 Astronot ============
@ch
def c09():
    c = Cv(26, 30, (34, 38, 52))
    cx = 13
    suit, suit2, glass = (234, 236, 242), (176, 182, 196), (48, 80, 132)
    c.rect(cx - 9, 16, cx - 6, 23, (148, 154, 168))        # sirt tanki
    c.rect(cx + 5, 16, cx + 8, 23, (148, 154, 168))
    legs(c, cx, 23, 28, suit, (92, 98, 112))
    torso(c, cx, 16, 23, 13, suit)
    c.row(cx - 6, cx + 5, 16, suit2)
    c.rect(cx - 3, 18, cx + 2, 21, suit2)
    c.rect(cx - 2, 19, cx + 1, 20, (104, 214, 132), False)
    head(c, cx, 2, 17, 13, suit, 4)                        # kask
    c.ell(cx, 8, 6, 5, glass, False)
    c.rect(cx - 1, 3, cx + 2, 4, dark(suit2, 0.1))
    c.rect(cx - 4, 5, cx - 2, 6, (156, 206, 248), False)   # cam parlamasi
    c.s(cx - 4, 7, (156, 206, 248), False)
    c.rect(cx + 1, 10, cx + 4, 11, (78, 122, 182), False)
    hands(c, cx - 10, cx + 9, 20, suit2)
    return c, "astronaut"


# ============ 10 Iskelet ============
@ch
def c10():
    c = Cv(24, 28, (48, 38, 32))
    cx = 12
    bone, bone2 = (240, 236, 216), (198, 192, 170)
    legs(c, cx, 22, 26, bone, bone2, 2, 4)
    c.rect(cx - 5, 21, cx + 4, 22, bone2)                  # kalca
    c.col(cx - 1, 15, 21, bone2)                           # omurga
    c.col(cx, 15, 21, bone2)
    for y in (15, 17, 19):                                 # kaburga
        c.row(cx - 5, cx + 4, y, bone)
        c.row(cx - 6, cx + 5, y, bone)
    c.rect(cx - 7, 14, cx + 6, 15, bone)                   # kopruck kemigi
    head(c, cx, 2, 15, 12, bone, 3)                        # kafatasi
    c.rect(cx - 5, 7, cx - 2, 10, (30, 26, 30), False)
    c.rect(cx + 1, 7, cx + 4, 10, (30, 26, 30), False)
    c.s(cx - 4, 8, (255, 150, 62), False)
    c.s(cx + 2, 8, (255, 150, 62), False)
    c.rect(cx - 1, 11, cx, 12, (74, 62, 56), False)
    c.row(cx - 4, cx + 3, 13, bone2)
    for x in range(cx - 3, cx + 3, 2):
        c.col(x, 13, 13, (74, 62, 56), False)
    hands(c, cx - 9, cx + 8, 18, bone)
    return c, "skeleton"


# ============ 11 Ninja ============
@ch
def c11():
    c = Cv(30, 28, (22, 22, 32))
    cx = 12
    ki, ki2, scarf = (52, 56, 80), (34, 38, 58), (200, 56, 64)
    c.rect(cx + 6, 13, cx + 13, 15, scarf)                 # ucusan atki
    c.rect(cx + 11, 16, cx + 16, 17, scarf)
    c.rect(cx + 15, 14, cx + 17, 15, dark(scarf, 0.2))
    legs(c, cx, 22, 26, ki2, (24, 26, 40))
    torso(c, cx, 15, 22, 12, ki)
    c.rect(cx - 6, 15, cx + 5, 16, ki2)
    c.row(cx - 6, cx + 5, 20, (212, 198, 122))             # obi
    head(c, cx, 2, 15, 12, ki, 2)
    c.rect(cx - 6, 7, cx + 5, 10, (26, 28, 40), False)     # goz yarigi (karanlik)
    c.rect(cx - 5, 8, cx - 3, 9, (232, 226, 208), False)   # gozler
    c.rect(cx + 2, 8, cx + 4, 9, (232, 226, 208), False)
    c.s(cx - 3, 9, (150, 156, 180), False)
    c.s(cx + 2, 9, (150, 156, 180), False)
    c.rect(cx - 7, 12, cx + 6, 14, scarf)                  # boyun atkisi
    c.row(cx - 6, cx + 5, 2, ki2)
    hands(c, cx - 9, cx + 8, 19, ki2)
    return c, "shadow_ninja"


# ============ 12 Korsan ============
@ch
def c12():
    c = Cv(26, 30, (40, 28, 22))
    cx = 13
    coat, coat2, skin = (116, 46, 54), (86, 34, 42), (234, 188, 142)
    legs(c, cx, 23, 28, (56, 48, 60), (34, 28, 36))
    torso(c, cx, 15, 23, 13, coat)
    c.rect(cx - 1, 15, cx, 23, (230, 224, 206))            # gomlek
    c.rect(cx - 6, 15, cx - 4, 18, coat2)
    c.rect(cx + 3, 15, cx + 5, 18, coat2)
    c.row(cx - 6, cx + 5, 21, (48, 42, 36))
    c.rect(cx - 1, 21, cx, 21, (228, 192, 74), False)
    head(c, cx, 3, 15, 12, skin, 2)
    eyes(c, cx, 9, 4, (50, 36, 30), 2, 2, (250, 248, 236))
    c.rect(cx + 1, 9, cx + 4, 11, (32, 28, 32), False)     # goz bandi
    c.rect(cx + 1, 7, cx + 6, 8, (32, 28, 32), False)
    c.rect(cx - 4, 13, cx + 3, 14, (146, 98, 68))          # sakal
    c.rect(cx - 8, 4, cx + 7, 5, (36, 32, 38))             # tricorn
    c.row(cx - 6, cx + 5, 3, (36, 32, 38))
    c.tri(cx, 2, 3, 4, (36, 32, 38))
    c.rect(cx - 1, 2, cx, 3, (230, 224, 206), False)
    hands(c, cx - 10, cx + 9, 19, coat2)
    return c, "pirate_captain"


# ============ 13 Veba doktoru ============
@ch
def c13():
    c = Cv(26, 34, (30, 26, 24))
    cx = 13
    coat, coat2, mask = (48, 46, 58), (32, 30, 42), (208, 192, 152)
    legs(c, cx, 28, 31, (36, 32, 30), (24, 22, 24))
    for i, y in enumerate(range(17, 30)):                  # uzun palto
        w = 13 + i // 3
        c.row(cx - w // 2, cx - w // 2 + w - 1, y, coat)
    c.rect(cx - 1, 17, cx, 29, coat2)
    c.row(cx - 7, cx + 6, 22, (128, 88, 46))               # kemer
    c.rect(cx - 5, 17, cx + 4, 18, coat2)                  # pelerin yakasi
    head(c, cx, 4, 14, 11, mask, 2)
    c.tri(cx, 14, 7, 3, mask, False)                       # gaga
    c.s(cx - 1, 20, dark(mask, 0.35))
    c.rect(cx - 6, 8, cx - 3, 11, (56, 62, 78), False)     # cam gozler
    c.rect(cx + 2, 8, cx + 5, 11, (56, 62, 78), False)
    c.s(cx - 6, 8, (196, 220, 244), False)
    c.s(cx + 2, 8, (196, 220, 244), False)
    c.rect(cx - 8, 5, cx + 7, 6, coat)                     # genis sapka
    c.row(cx - 6, cx + 5, 4, coat2)
    c.rect(cx - 5, 1, cx + 4, 3, coat)
    c.row(cx - 5, cx + 4, 3, coat2)
    hands(c, cx - 9, cx + 8, 21, coat2)
    return c, "plague_doctor"


# ============ 14 Viking ============
@ch
def c14():
    c = Cv(30, 30, (48, 32, 24))
    cx = 15
    fur, fur2, skin = (134, 102, 68), (100, 76, 50), (240, 196, 150)
    legs(c, cx, 24, 28, (88, 66, 46), (56, 42, 30), 4, 2)
    torso(c, cx, 16, 24, 15, fur)
    c.rect(cx - 8, 16, cx - 5, 19, fur2)                   # kurk omuzlar
    c.rect(cx + 4, 16, cx + 7, 19, fur2)
    c.rect(cx - 4, 18, cx + 3, 23, (154, 160, 172))        # gogus zirhi
    c.row(cx - 4, cx + 3, 18, lite((154, 160, 172), 0.25))
    c.row(cx - 6, cx + 5, 24, (74, 56, 40))
    head(c, cx, 4, 14, 11, skin, 2)
    c.rect(cx - 7, 3, cx + 6, 7, (146, 152, 166))          # migfer
    c.row(cx - 6, cx + 5, 2, (172, 178, 192))
    c.col(cx - 1, 3, 9, (172, 178, 192))                   # burun koruma
    for i in range(4):                                     # boynuzlar
        c.rect(cx - 9 - i, 4 - i, cx - 8 - i, 6 - i, (240, 230, 202))
        c.rect(cx + 8 + i, 4 - i, cx + 9 + i, 6 - i, (240, 230, 202))
    eyes(c, cx, 9, 5, (54, 36, 28), 2, 2, (250, 248, 232))
    c.rect(cx - 5, 12, cx + 4, 14, (200, 136, 66))         # sakal
    c.row(cx - 3, cx + 2, 15, (200, 136, 66))
    hands(c, cx - 11, cx + 10, 20, skin)
    return c, "viking_raider"


# ============ 15 Robot ============
@ch
def c15():
    c = Cv(24, 30, (30, 32, 42))
    cx = 12
    b1, b2, scr = (192, 152, 64), (144, 110, 48), (28, 44, 54)
    c.rect(cx - 8, 22, cx + 7, 27, (76, 80, 92))           # palet
    for x in range(cx - 7, cx + 8, 3):
        c.col(x, 23, 26, (50, 54, 64))
    c.row(cx - 8, cx + 7, 22, (104, 108, 120))
    torso(c, cx, 15, 21, 14, b1)
    c.rect(cx - 4, 17, cx + 3, 20, b2)
    c.rect(cx - 2, 18, cx + 1, 19, (255, 178, 62), False)
    c.rect(cx - 6, 4, cx + 5, 14, b1)                      # kutu kafa
    c.rect(cx - 5, 6, cx + 4, 11, scr, False)              # ekran
    c.rect(cx - 4, 8, cx - 2, 9, (124, 246, 202), False)
    c.rect(cx + 1, 8, cx + 3, 9, (124, 246, 202), False)
    c.row(cx - 2, cx + 1, 10, (58, 158, 138), False)
    c.row(cx - 6, cx + 5, 4, b2)
    c.rect(cx - 8, 6, cx - 7, 11, b2)                      # yan moduller
    c.rect(cx + 6, 6, cx + 7, 11, b2)
    c.col(cx, 1, 3, (122, 126, 138))
    c.rect(cx - 1, 0, cx + 1, 1, (232, 74, 64), False)
    hands(c, cx - 10, cx + 9, 18, (104, 108, 120))
    return c, "cog_robot"


# ============ 16 Kedi ============
@ch
def c16():
    c = Cv(28, 30, (52, 32, 26))
    cx = 12
    fur, fur2, cloth = (234, 170, 94), (198, 134, 64), (74, 112, 152)
    c.rect(cx + 6, 20, cx + 11, 22, fur)                   # kuyruk
    c.rect(cx + 10, 15, cx + 12, 21, fur)
    c.rect(cx + 10, 13, cx + 12, 14, fur2)
    legs(c, cx, 23, 27, fur, fur2, 3, 3)
    torso(c, cx, 16, 23, 12, cloth)
    c.rect(cx - 2, 16, cx + 1, 18, (246, 228, 200))
    c.row(cx - 6, cx + 5, 21, dark(cloth, 0.3))
    head(c, cx, 3, 15, 12, fur, 2)
    for i in range(4):                                     # kulaklar
        c.rect(cx - 7 + i, 2 - i, cx - 4, 2 - i, fur)
        c.rect(cx + 3, 2 - i, cx + 6 - i, 2 - i, fur)
    c.rect(cx - 6, 0, cx - 5, 1, (240, 152, 162), False)
    c.rect(cx + 4, 0, cx + 5, 1, (240, 152, 162), False)
    eyes(c, cx, 9, 4, (76, 182, 100), 2, 3, (242, 255, 244))
    c.s(cx - 4, 11, (28, 40, 30), False)
    c.s(cx + 3, 11, (28, 40, 30), False)
    c.rect(cx - 1, 12, cx, 13, (220, 126, 134), False)
    c.row(cx - 8, cx - 6, 12, fur2)                        # biyiklar
    c.row(cx + 5, cx + 7, 12, fur2)
    hands(c, cx - 9, cx + 8, 19, fur)
    return c, "beast_kin"


# ============ 17 Iblis ============
@ch
def c17():
    c = Cv(28, 30, (60, 18, 22))
    cx = 14
    sk, sk2, cloth = (212, 70, 62), (168, 44, 44), (46, 34, 50)
    for i in range(4):                                     # kanatlar
        c.rect(cx - 12 + i, 12 + i, cx - 8, 20 - i, (124, 34, 42))
        c.rect(cx + 7, 12 + i, cx + 11 - i, 20 - i, (124, 34, 42))
    c.rect(cx - 11, 10, cx - 9, 12, (124, 34, 42))
    c.rect(cx + 8, 10, cx + 10, 12, (124, 34, 42))
    legs(c, cx, 23, 27, sk2, (44, 32, 36))
    torso(c, cx, 16, 23, 13, sk)
    c.rect(cx - 6, 16, cx + 5, 18, cloth)
    c.row(cx - 6, cx + 5, 22, (206, 166, 62))
    head(c, cx, 3, 15, 12, sk, 2)
    for i in range(4):                                     # boynuzlar
        c.rect(cx - 8 + i // 2, 2 - i, cx - 6 + i // 2, 3 - i, (50, 36, 42))
        c.rect(cx + 5 - i // 2, 2 - i, cx + 7 - i // 2, 3 - i, (50, 36, 42))
    eyes(c, cx, 8, 4, (252, 216, 74), 3, 3, None)
    c.s(cx - 4, 9, (60, 24, 28), False)
    c.s(cx + 3, 9, (60, 24, 28), False)
    c.rect(cx - 3, 12, cx + 2, 13, (62, 22, 28), False)    # sirit
    c.s(cx - 3, 12, (250, 244, 226), False)
    c.s(cx + 2, 12, (250, 244, 226), False)
    hands(c, cx - 10, cx + 9, 20, sk2)
    return c, "imp_demon"


# ============ 18 Uzayli ============
@ch
def c18():
    c = Cv(24, 26, (28, 50, 44))
    cx = 12
    sk, sk2, suit = (152, 216, 172), (114, 178, 134), (62, 72, 100)
    legs(c, cx, 21, 24, suit, (38, 46, 64), 3, 3)
    torso(c, cx, 15, 21, 10, suit)
    c.rect(cx - 2, 17, cx + 1, 19, (124, 244, 214), False)
    c.row(cx - 5, cx + 4, 15, (94, 106, 138))
    c.ell(cx, 8, 9, 7, sk)                                 # buyuk kafa
    c.rect(cx - 2, 14, cx + 1, 15, sk2)
    c.rect(cx - 7, 8, cx - 3, 12, (20, 20, 28), False)     # siyah gozler
    c.rect(cx + 2, 8, cx + 6, 12, (20, 20, 28), False)
    c.rect(cx - 7, 8, cx - 6, 9, (144, 184, 224), False)
    c.rect(cx + 2, 8, cx + 3, 9, (144, 184, 224), False)
    c.row(cx - 1, cx, 13, sk2, False)
    c.rect(cx - 4, 2, cx - 1, 3, lite(sk, 0.32), False)
    hands(c, cx - 8, cx + 7, 18, sk)
    return c, "grey_alien"


# ============ 19 Kultist ============
@ch
def c19():
    c = Cv(26, 34, (34, 24, 44))
    cx = 13
    robe, robe2, gold = (92, 62, 128), (66, 44, 96), (206, 176, 84)
    for i, y in enumerate(range(16, 31)):                  # cuppe
        w = 13 + (i * 2) // 3
        c.row(cx - w // 2, cx - w // 2 + w - 1, y, robe)
    for x in range(cx - 8, cx + 9, 4):
        c.col(x, 27, 30, robe2)
    c.row(cx - 6, cx + 5, 20, gold)                        # kusak
    c.rect(cx - 1, 21, cx, 24, gold)
    c.rect(cx - 2, 24, cx + 1, 25, gold)
    head(c, cx, 4, 14, 12, (20, 16, 28), 2)                # kapuson icindeki karanlik
    head(c, cx, 3, 16, 11, robe, 3)                        # kapuson
    c.rect(cx - 9, 10, cx - 7, 18, robe)
    c.rect(cx + 6, 10, cx + 8, 18, robe)
    c.rect(cx - 6, 11, cx + 5, 12, dark(robe, 0.45))
    c.rect(cx - 6, 16, cx + 5, 17, robe2)
    c.rect(cx - 5, 13, cx - 3, 14, (255, 216, 92), False)  # parlayan gozler
    c.rect(cx + 2, 13, cx + 4, 14, (255, 216, 92), False)
    c.s(cx - 5, 13, (255, 160, 60), False)
    c.s(cx + 4, 13, (255, 160, 60), False)
    hands(c, cx - 10, cx + 9, 21, (24, 18, 32))
    return c, "cultist"


# ============ 20 Muhendis ============
@ch
def c20():
    c = Cv(26, 30, (46, 34, 24))
    cx = 12
    ap, ap2, skin = (154, 108, 60), (116, 80, 44), (242, 202, 160)
    c.rect(cx + 6, 15, cx + 11, 22, (124, 128, 140))       # sirt kazani
    c.rect(cx + 7, 13, cx + 9, 15, (88, 92, 104))
    c.rect(cx + 7, 17, cx + 10, 19, (214, 154, 62))
    legs(c, cx, 23, 28, (82, 88, 100), (48, 52, 62))
    torso(c, cx, 15, 23, 13, (112, 136, 152))
    c.rect(cx - 5, 18, cx + 4, 23, ap)                     # onluk
    c.row(cx - 5, cx + 4, 18, ap2)
    c.rect(cx - 2, 20, cx + 1, 22, (210, 174, 78))
    c.rect(cx - 6, 15, cx - 4, 17, ap2)
    c.rect(cx + 3, 15, cx + 5, 17, ap2)
    head(c, cx, 3, 15, 12, skin, 2)
    eyes(c, cx, 11, 4, (62, 46, 36), 2, 2, (250, 246, 230))
    c.rect(cx - 7, 6, cx + 6, 8, (102, 78, 50))            # gozluk bandi
    c.ell(cx - 4, 7, 2, 2, (202, 154, 68), False)
    c.ell(cx + 3, 7, 2, 2, (202, 154, 68), False)
    c.s(cx - 5, 6, (244, 226, 168), False)
    c.s(cx + 2, 6, (244, 226, 168), False)
    c.rect(cx - 6, 3, cx + 5, 5, (88, 66, 42))             # kasket
    c.rect(cx - 8, 5, cx - 6, 5, (68, 50, 32))
    c.rect(cx - 3, 14, cx + 2, 15, (198, 142, 80))         # biyik
    hands(c, cx - 9, cx + 8, 20, ap2)
    return c, "engineer"


# ===================== URET =====================
def main():
    os.makedirs(OUT, exist_ok=True)
    items = []
    for i, fn in enumerate(CHARS, 1):
        c, name = fn()
        c.autoshade()
        c.autooutline()
        im = c.img().crop(c.img().getbbox())
        path = os.path.join(OUT, "Claude_ConceptArt_%02d.png" % i)
        im.save(path)
        items.append((im, name, i))
        print("%02d %-16s %2dx%-2d  %s" % (i, name, im.width, im.height,
                                           "OK" if 20 <= im.width <= 40 and 20 <= im.height <= 40 else "!! BOYUT"))

    S, cols = 6, 5
    cw = max(x[0].width for x in items) * S + 18
    chh = max(x[0].height for x in items) * S + 18
    rows = (len(items) + cols - 1) // cols
    sheet = Image.new("RGBA", (cw * cols, chh * rows), (48, 44, 58, 255))
    for idx, (im, name, i) in enumerate(items):
        big = im.resize((im.width * S, im.height * S), Image.NEAREST)
        gx, gy = idx % cols, idx // cols
        sheet.alpha_composite(big, (gx * cw + (cw - big.width) // 2,
                                    gy * chh + (chh - big.height) // 2))
    sheet.save(os.path.join(SCRATCH, "sheet.png"))


main()
