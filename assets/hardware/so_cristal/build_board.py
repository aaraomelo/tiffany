#!/usr/bin/env python3
# build_board.py — monta a placa do SO no cristal (multiplicador log/antilog + somador)
# roda com: flatpak run --command=python3 org.kicad.KiCad build_board.py
import pcbnew
FP="/app/extensions/Library/Footprints/footprints"
def mm(x): return pcbnew.FromMM(x)

b = pcbnew.BOARD()

# ---- componentes: (ref, valor, libdir, footprint, x_mm, y_mm, rot_graus) ----
COMPS = [
 ("U1","TL074","Package_SO","SOIC-14_3.9x8.7mm_P1.27mm", 40,22, 0),
 ("U2","TL072","Package_SO","SOIC-8_3.9x4.9mm_P1.27mm",  40,42, 0),
 ("Q1","MMBT3904","Package_TO_SOT_SMD","SOT-23", 22,14, 0),
 ("Q2","MMBT3904","Package_TO_SOT_SMD","SOT-23", 22,30, 0),
 ("Q3","MMBT3904","Package_TO_SOT_SMD","SOT-23", 58,14, 0),
 ("C1","100nF","Capacitor_SMD","C_0805_2012Metric", 55,22, 0),
 ("C2","100nF","Capacitor_SMD","C_0805_2012Metric", 55,42, 0),
 ("J1","IN a b G","Connector_PinHeader_2.54mm","PinHeader_1x03_P2.54mm_Vertical", 8,30, 0),
 ("J2","OUT M A G","Connector_PinHeader_2.54mm","PinHeader_1x03_P2.54mm_Vertical", 72,30, 0),
 ("J3","V+ V- G","Connector_PinHeader_2.54mm","PinHeader_1x03_P2.54mm_Vertical", 40,7, 0),
]
# resistores 100k, em duas fileiras
RX0, RY = 16, 50
for i,r in enumerate(range(1,12)):
    COMPS.append((f"R{r}","100k","Resistor_SMD","R_0805_2012Metric", RX0+ (i%6)*9, RY + (i//6)*7, 90))
# furos de montagem
for i,(x,y) in enumerate([(5,5),(75,5),(5,58),(75,58)]):
    COMPS.append((f"H{i+1}","","MountingHole","MountingHole_3.2mm_M3", x,y,0))

fps={}
for ref,val,lib,fp,x,y,rot in COMPS:
    f=pcbnew.FootprintLoad(f"{FP}/{lib}.pretty", fp)
    f.SetReference(ref); f.SetValue(val)
    f.SetPosition(pcbnew.VECTOR2I(mm(x),mm(y)))
    if rot: f.SetOrientationDegrees(rot)
    b.Add(f); fps[ref]=f

# ---- netlist: net -> [(ref,pad), ...] ----
NETS = {
 "VCC": [("U1","4"),("U2","8"),("J3","1"),("C1","1")],
 "VEE": [("U1","11"),("U2","4"),("J3","2"),("C2","1")],
 "GND": [("U1","3"),("U1","5"),("U1","10"),("U1","12"),("U2","3"),("U2","5"),
         ("J1","3"),("J2","3"),("J3","3"),("C1","2"),("C2","2"),
         ("Q1","1"),("Q2","1"),("Q3","2")],
 "A_in":[("J1","1"),("R1","1"),("R7","1")],
 "B_in":[("J1","2"),("R2","1"),("R8","1")],
 # log a
 "n1":  [("R1","2"),("U1","2"),("Q1","3")],
 "LOGA":[("U1","1"),("Q1","2"),("R3","1")],
 # log b
 "n2":  [("R2","2"),("U1","6"),("Q2","3")],
 "LOGB":[("U1","7"),("Q2","2"),("R4","1")],
 # somador dos logs (Pontryagin Σ)
 "n3":  [("R3","2"),("R4","2"),("U1","9"),("R5","1")],
 "SUMV":[("U1","8"),("R5","2"),("Q3","1")],
 # antilog -> MULT
 "n4":  [("Q3","3"),("U1","13"),("R6","1")],
 "MULT":[("U1","14"),("R6","2"),("J2","1")],
 # somador ⊕
 "n5":  [("R7","2"),("R8","2"),("U2","2"),("R9","1")],
 "ADDN":[("U2","1"),("R9","2"),("R10","1")],
 "n6":  [("R10","2"),("U2","6"),("R11","1")],
 "ADD": [("U2","7"),("R11","2"),("J2","2")],
}
for name, conns in NETS.items():
    net = pcbnew.NETINFO_ITEM(b, name); b.Add(net)
    for ref,pad in conns:
        f=fps[ref]; found=False
        for p in f.Pads():
            if p.GetNumber()==pad: p.SetNet(net); found=True
        if not found: print("PAD NAO ACHADO:", ref, pad)

# ---- contorno da placa (Edge.Cuts): retangulo 80x64 mm ----
W,H=80,64
pts=[(0,0),(W,0),(W,H),(0,H),(0,0)]
for i in range(4):
    seg=pcbnew.PCB_SHAPE(b); seg.SetShape(pcbnew.SHAPE_T_SEGMENT)
    seg.SetStart(pcbnew.VECTOR2I(mm(pts[i][0]),mm(pts[i][1])))
    seg.SetEnd(pcbnew.VECTOR2I(mm(pts[i+1][0]),mm(pts[i+1][1])))
    seg.SetLayer(pcbnew.Edge_Cuts); seg.SetWidth(mm(0.15)); b.Add(seg)

b.BuildListOfNets(); b.BuildConnectivity()
out="/home/aaraolopes/Documentos/chess/hardware/so_cristal/so_cristal.kicad_pcb"
pcbnew.SaveBoard(out, b)
print("SALVO:", out)
print("footprints:", len(fps), " nets:", len(NETS))
