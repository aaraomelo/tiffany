#!/usr/bin/env python3
# tools/orfas.py — AS MEDIDAS QUE SE FAZEM E NÃO CONTAM.
#
# Um contador que é incrementado, IMPRESSO, e nunca chega a um veredicto: o número aparece
# no ecrã, o leitor lê-o como medido, e se ele mudar nada falha. Foi assim que o
# `norma.c` calculava «z num subcorpo?», imprimia a coluna, e o resíduo não a via — e a
# coluna era a ÚNICA que apanhava a sabotagem.
#
# O detector segue o VEREDICTO para trás: o texto completo de cada ok()/VD()/pulso(), as
# variáveis de veredicto (res, passou, erro, mau, viol…), a CONDIÇÃO dos ifs que lhes
# mexem, o que a main devolve — e depois fecha por atribuições até estabilizar.
#
# Foi afinado em QUATRO voltas contra casos conhecidos, porque as três primeiras versões
# davam falsos positivos: 1174 (lia só a linha do ok, e as asserções são multi-linha),
# 358 (não seguia cadeias indirectas), 176 (não conhecia `res++` nem `passou`). O controlo
# é correr nos ficheiros onde a resposta já se sabe.
#
#   python3 tools/orfas.py
import re, sys, glob
VER = r'(res|falhas|mau\w*|erros?|passou|viol\w*|ruim|bad|falhou)'

def vivas_de(src):
    """o conjunto VIVO: o que chega a um veredicto, directa ou indirectamente."""
    vivas = set()
    # (a) o texto completo de cada ok(...)/VD(...)/pulso(...)
    for m in re.finditer(r'\b(ok|VD|conclui|pulso|limite)\s*\(', src):
        i = m.end()-1; d=0; j=i
        while j < len(src):
            if src[j]=='(': d+=1
            elif src[j]==')':
                d-=1
                if d==0: break
            j+=1
        vivas |= set(re.findall(r'\b([a-z_][a-z_0-9]{1,})\b', src[i:j+1]))
    # (b) as próprias variáveis de veredicto, e tudo o que lhes é atribuído
    vivas |= set(re.findall(VER, src))
    for m in re.finditer(r'\b' + VER + r'\s*(?:\+?=|\+\+|--)([^;]*);', src):
        vivas |= set(re.findall(r'\b([a-z_][a-z_0-9]{1,})\b', m.group(2) or ''))
    # (c) a CONDIÇÃO de um if que mexe num veredicto
    for m in re.finditer(r'if\s*\(([^;{]*?)\)\s*\{?\s*' + VER + r'\s*(?:\+\+|--|\+?=)', src):
        vivas |= set(re.findall(r'\b([a-z_][a-z_0-9]{1,})\b', m.group(1)))
    # (d) o que a main devolve
    for m in re.finditer(r'return\s+([^;]*);', src):
        vivas |= set(re.findall(r'\b([a-z_][a-z_0-9]{1,})\b', m.group(1)))
    # (e) FECHO: se w vive e v aparece numa atribuição a w, v vive
    for _ in range(8):
        novas = set()
        for m in re.finditer(r'\b([a-z_][a-z_0-9]{1,})\s*(?:\+?=)([^;=][^;]*);', src):
            if m.group(1) in vivas:
                novas |= set(re.findall(r'\b([a-z_][a-z_0-9]{1,})\b', m.group(2)))
        for m in re.finditer(r'if\s*\(([^;{]*?)\)\s*\{?\s*([a-z_][a-z_0-9]{1,})\s*(?:\+\+|--|\+?=)', src):
            if m.group(2) in vivas:
                novas |= set(re.findall(r'\b([a-z_][a-z_0-9]{1,})\b', m.group(1)))
        if novas <= vivas: break
        vivas |= novas
    return vivas

achados = []
for f in sorted(glob.glob('tests/*.c')):
    src = open(f, encoding='utf-8', errors='replace').read()
    vivas = vivas_de(src)
    cont = set(re.findall(r'\b([a-z_][a-z_0-9]{2,})\s*\+\+', src)) \
         | set(re.findall(r'\b([a-z_][a-z_0-9]{2,})\s*\+=', src))
    for v in sorted(cont):
        if v in ('unidades','falhas','res','casos','tot'): continue
        if not re.search(r'printf[^;]*\b'+re.escape(v)+r'\b', src, re.S): continue
        if v in vivas: continue
        achados.append((f, v))
for f, v in achados: print(f, v)
print('TOTAL', len(achados), file=sys.stderr)
