#!/usr/bin/env python3
"""Gera os STL das peças do robo4 (impressão 3D) a partir das cotas do diagrama-pecas.svg.

Uso:  ~/.local/share/cad-venv/bin/python cad/gerar_stl.py [pasta_de_saida]
Precisa de: pip install manifold3d trimesh numpy

Medidas em mm. Cada peça é desenhada com a origem no canto de cima à esquerda (y para baixo),
igual às cotas do guia. Furos já vêm com folga para impressão FDM.
"""
import sys
from pathlib import Path

import numpy as np
import trimesh
from manifold3d import CrossSection, JoinType, Manifold

ESP = 3.0          # espessura das placas
N = 64             # segmentos por círculo

# Folgas para FDM (o plástico encolhe o furo)
FURO_M2 = 2.3      # parafuso das abas do servo
FURO_M3 = 3.4      # parafusos M3 (Uno, pé, cantoneira)
FURO_HORN = 3.2    # centro do horn (passa a chave no parafuso do eixo)
FOLGA_JANELA = 0.3 # janela do servo: 23 x 12,5 vira 23,3 x 12,8
RASGO_HORN = 1.8   # rasgo para os parafusinhos do horn, serve em qualquer horn de 1 ponta


def ret(x0, y0, x1, y1):
    """Retângulo dado pelos cantos, com y para baixo (vira y negativo no 3D)."""
    return CrossSection.square([x1 - x0, y1 - y0]).translate([x0, -y1])


def ret_arred(w, h, r):
    return CrossSection.square([w - 2 * r, h - 2 * r]).translate([r, -(h - r)]).offset(r, JoinType.Round, circular_segments=N)


def furo(x, y, d):
    return CrossSection.circle(d / 2, N).translate([x, -y])


def rasgo(x, y0, y1, w):
    """Rasgo vertical com pontas redondas, do y0 ao y1."""
    r = w / 2
    return (CrossSection.square([w, (y1 - y0)]).translate([x - r, -y1])
            + furo(x, y0, w) + furo(x, y1, w))


def janela(x0, y0, x1, y1):
    f = FOLGA_JANELA / 2
    return ret(x0 - f, y0 - f, x1 + f, y1 + f)


def placa(contorno, furos):
    cs = contorno
    for f in furos:
        cs = cs - f
    return Manifold.extrude(cs, ESP)


# ---------- Peças ----------

def parede():
    """Parede lateral 140 x 35 (imprimir 2). Servos do quadril com eixo a 20 e 120, 20 abaixo do topo."""
    furos = [
        janela(14.0, 13.75, 36.8, 26.25),   # servo de trás: eixo a 6 da ponta (x=20)
        janela(103.2, 13.75, 126.0, 26.25), # servo da frente: eixo em x=120
        furo(11.5, 20, FURO_M2), furo(39.3, 20, FURO_M2),
        furo(100.7, 20, FURO_M2), furo(128.5, 20, FURO_M2),
        furo(62, 27.5, FURO_M3), furo(82, 27.5, FURO_M3),  # cantoneiras
    ]
    return placa(ret(0, 0, 140, 35), furos)


def base():
    """Base 140 x 80 (imprimir 1). Furos do Uno + furos das 4 cantoneiras."""
    cx, cy = 35.7, 13.4
    uno = [(14.0, 2.5), (15.3, 50.7), (66.1, 7.6), (66.1, 35.5)]
    furos = [furo(cx + dx, cy + dy, FURO_M3) for dx, dy in uno]
    furos += [furo(x, y, FURO_M3) for x in (62, 82) for y in (7.5, 72.5)]
    return placa(ret(0, 0, 140, 80), furos)


def coxa():
    """Coxa 20 x 82 (imprimir 4). Horn do quadril a 8 do topo, eixo do joelho a 63."""
    furos = [
        furo(10, 8, FURO_HORN),
        rasgo(10, 13, 21, RASGO_HORN),
        janela(3.75, 46.2, 16.25, 69.0),
        furo(10, 43.7, FURO_M2), furo(10, 71.5, FURO_M2),
    ]
    return placa(ret_arred(20, 82, 3.3), furos)


def canela():
    """Canela 14 x 74 (imprimir 4). Horn do joelho a 8 do topo, pé 60 abaixo."""
    furos = [
        furo(7, 8, FURO_HORN),
        rasgo(7, 13, 21, RASGO_HORN),
        furo(7, 68, FURO_M3),
    ]
    return placa(ret_arred(14, 74, 3.3), furos)


def cantoneira():
    """Cantoneira em L 10 x 12 x 12 (imprimir 4). Prende a parede na base com M3."""
    largura, perna = 10.0, 12.0
    deitada = Manifold.cube([largura, perna, ESP])
    em_pe = Manifold.cube([largura, ESP, perna])
    # dois reforços triangulares nas bordas, fora do caminho da cabeça do parafuso
    def reforco(x0):
        pts = [[x, y, z] for x in (x0, x0 + 1.6) for y, z in ((ESP, ESP), (ESP + 4, ESP), (ESP, ESP + 4))]
        return Manifold.hull_points(pts)
    peca = deitada + em_pe + reforco(0) + reforco(largura - 1.6)
    r = FURO_M3 / 2
    furo_base = Manifold.cylinder(ESP * 3, r, r, N).translate([largura / 2, 7.5, -ESP])
    furo_parede = Manifold.cylinder(ESP * 3, r, r, N).rotate([-90, 0, 0]).translate([largura / 2, -ESP, 7.5])
    return peca - furo_base - furo_parede


PECAS = {
    "robo4-parede": (parede, 2),
    "robo4-base": (base, 1),
    "robo4-coxa": (coxa, 4),
    "robo4-canela": (canela, 4),
    "robo4-cantoneira": (cantoneira, 4),
}


def para_trimesh(m: Manifold) -> trimesh.Trimesh:
    g = m.to_mesh()
    return trimesh.Trimesh(np.asarray(g.vert_properties)[:, :3], np.asarray(g.tri_verts), process=True)


def placa_de_impressao(malhas):
    """Todas as 15 peças lado a lado numa mesa de 220 x 220 (x, y, girar 90°)."""
    pos = {
        "robo4-base": [(0, 0, 0)],
        "robo4-parede": [(0, 84, 0), (0, 123, 0)],
        "robo4-coxa": [(144, 0, 0), (166, 0, 0), (188, 0, 0), (0, 162, 1)],
        "robo4-canela": [(144, 86, 0), (160, 86, 0), (176, 86, 0), (192, 86, 0)],
        "robo4-cantoneira": [(90, 162, 0), (104, 162, 0), (118, 162, 0), (132, 162, 0)],
    }
    partes = []
    for nome, lugares in pos.items():
        for x, y, girar in lugares:
            c = malhas[nome].copy()
            if girar:
                c.apply_transform(trimesh.transformations.rotation_matrix(np.pi / 2, [0, 0, 1]))
            lo = c.bounds[0]
            c.apply_translation([x - lo[0], y - lo[1], -lo[2]])
            partes.append(c)
    return trimesh.util.concatenate(partes)


def main():
    saida = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent / "pecas" / "stl")
    saida.mkdir(parents=True, exist_ok=True)
    malhas = {}
    for nome, (fn, qtd) in PECAS.items():
        m = para_trimesh(fn())
        assert m.is_watertight and m.volume > 0, nome
        m.export(saida / f"{nome}.stl")
        malhas[nome] = m
        dx, dy, dz = m.extents
        print(f"{nome}.stl  {dx:.1f} x {dy:.1f} x {dz:.1f} mm  imprimir {qtd}")
    mesa = placa_de_impressao(malhas)
    mesa.export(saida / "robo4-mesa-completa.stl")
    dx, dy, _ = mesa.extents
    print(f"robo4-mesa-completa.stl  {dx:.0f} x {dy:.0f} mm  (todas as 15 peças)")


if __name__ == "__main__":
    main()
