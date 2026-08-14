#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""cristal_cura.py — a curadoria do cristal, resolvida (ordem do dono, 13/08).

Aplica a fusão de conceitos MEDIDA (tests/fusao_conceitos.js) aos candidatos
reais do cristal, por duas regras, e escreve a decisão inteira — fusões E
recusas — em cristal/curadoria.tsv:

  Regra A (mecânica, medida): pares de TEXTO idêntico — o mesmo conteúdo
    ingerido por dois esquemas de endereço (id nu vs id com prefixo de hub);
    diferem só nas arestas reescritas e em meta.fonte. Mantém-se o endereço
    específico (o mais longo). Nada aqui é julgado: a igualdade é byte a byte.
  Regra B (julgada, lida): mesmo conceito em prosa diferente. A fusão guarda
    AS DUAS faces intactas («a dualidade é a memória da divisão») — nada se
    apaga. Cada par foi lido antes de decidir; o motivo fica no tsv.
  MANTER (julgada, lida): candidatos que a regra cega fundiria e a leitura
    separou — ficam, com motivo. A recusa é parte da resolução.

A fusão é a do medidor: z = {"fusao":[x,y],"id":<mantido>,"tipo":"conceito"},
com as linhas originais VERBATIM dentro de z; conservação E(z)=E(x)+E(y)+E_∂,
E_∂ = E(esqueleto); a fibra corta o texto e devolve byte a byte.

  python3 tools/cristal_cura.py            # aplica (recusa se já aplicada)
  python3 tools/cristal_cura.py --desfaz   # desfaz TODAS as fusões (a volta)
"""
import json, os, sys

RAIZ = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
FONTE = os.path.join(RAIZ, 'cristal', 'cristal.jsonl')
TSV = os.path.join(RAIZ, 'cristal', 'curadoria.tsv')

# campos de TEXTO: tudo o que o conceito DIZ; fora ficam id (endereço),
# arestas (endereçamento do grafo) e meta (rótulo de ingestão)
TEXTO = ('titulo', 'descricao', 'exemplos', 'contraexemplos', 'tipo', 'origem',
         'palavras_chave', 'sinonimos', 'memoria', 'epistemico', 'confianca')

# Regra B — mesmo conceito, prosa diferente (mantido, absorvido, motivo)
JULGADAS = [
    ('processo', 'processos',
     'mesmo conceito: programa em execução como unidade de isolamento e scheduling'),
    ('reticulado', 'reticulados',
     'mesmo conceito: ordem parcial com join e meet'),
    ('alexander_alekhine', 'alekhine', 'a mesma pessoa, dois registos'),
    ('bobby_fischer', 'fischer', 'a mesma pessoa, dois registos'),
    ('garry_kasparov', 'kasparov', 'a mesma pessoa, dois registos'),
    ('paul_morphy', 'morphy', 'a mesma pessoa, dois registos'),
    ('numeros_de_pisot', 'numeros_pisot',
     'mesmo conceito: algébricos >1 com conjugados de módulo <1'),
    ('rede_inteligente', 'smart_grid',
     'mesmo conceito e mesmo domínio: a rede com medição e controle bidirecionais'),
]

# MANTER — a regra cega fundiria; a leitura separou
MANTIDAS = [
    ('arte', 'artes', 'o conceito geral vs a enumeração das práticas'),
    ('pergunta', 'perguntas', 'conceito de interrogação vs secção de um documento'),
    ('contraexemplo', 'contraexemplos', 'contraexemplo MQ/Dougherty vs Broca≠Linux — conteúdos distintos'),
    ('referencia', 'referencias', 'listas de referência de fontes distintas'),
    ('modelo_de_ameaca', 'modelo_de_ameacas', 'torção/assinatura vs threat modeling — homónimos'),
    ('analise_de_fourier', 'transformada_de_fourier', 'análise ⊋ transformada; domínios distintos'),
    ('admissibilidade_deadlock', 'admissibilidade_nao-deadlock', 'enunciado (catálogo) vs prova (extrato de paper)'),
    ('o_ponto_hermitiano', 'ponto_hermitiano', 'dois aspectos: família de multiplicações vs pressão algébrica'),
    ('dois_ciclos_reconstrucao', 'os_dois_ciclos_de_reconstrucao', 'conteúdos sem relação (fractal vs tesouraria Pell)'),
    ('definition_reta_de_ouro_seja_152_a_constante_do_vazi', 'reta_de_ouro', 'extrato de paper vs conceito do catálogo'),
    ('geometria', 'geometria_ramo', 'hub Broca vs ramo clássico'),
    ('recursao', 'recursao_linguistica', 'computação vs linguística'),
    ('broca_os_hub_o_que_e', 'tiffany_cabeca_broca_o_que_e',
     'SO fractal vs cabeça conversacional — conteúdos distintos (o endereço nu o_que_e foi absorvido pelo gémeo idêntico broca_os_hub_o_que_e)'),
]


def canon(o):
    return json.dumps(o, ensure_ascii=False, sort_keys=True, separators=(',', ':'))


def E(s):
    return sum(b * b for b in s.encode('utf-8'))


def texto(r):
    return canon([r.get(k) for k in TEXTO])


def eh_par_sp(a, b):
    return b == a + 's' or a == b + 's'


def fibra(lz):
    """Corta o TEXTO de z na vírgula de profundidade 0 — byte a byte."""
    ini = lz.index('[') + 1
    fim = lz.rindex(']')
    miolo = lz[ini:fim]
    prof, corte = 0, -1
    for i, ch in enumerate(miolo):
        if ch == '{':
            prof += 1
        elif ch == '}':
            prof -= 1
        elif ch == ',' and prof == 0:
            corte = i
            break
    assert corte > 0, 'fibra sem corte'
    return miolo[:corte], miolo[corte + 1:]


def le_fonte():
    linhas = [l.rstrip('\n') for l in open(FONTE, encoding='utf-8') if l.strip()]
    for l in linhas:
        assert canon(json.loads(l)) == l, 'fonte não-canónica: ' + l[:60]
    ids = [json.loads(l)['id'] for l in linhas]
    assert ids == sorted(ids), 'fonte fora de ordem'
    assert len(ids) == len(set(ids)), 'id duplicado na fonte'
    return linhas


def desfaz():
    linhas = le_fonte()
    saida, n = [], 0
    for l in linhas:
        r = json.loads(l)
        if 'fusao' in r:
            lx, ly = fibra(l)
            saida.extend([lx, ly])
            n += 1
        else:
            saida.append(l)
    saida.sort(key=lambda l: json.loads(l)['id'])
    with open(FONTE, 'w', encoding='utf-8') as f:
        f.write('\n'.join(saida) + '\n')
    print('desfeitas %d fusões; corpus: %d conceitos' % (n, len(saida)))
    return 0


def main():
    if '--desfaz' in sys.argv:
        return desfaz()
    linhas = le_fonte()
    regs = {json.loads(l)['id']: l for l in linhas}
    if any('fusao' in json.loads(l) for l in linhas):
        print('curadoria já aplicada — nada a fazer (desfazer: --desfaz)')
        return 1

    # Regra A: grupos de texto idêntico, descobertos AGORA (não uma lista morta)
    portexto = {}
    for i, l in regs.items():
        portexto.setdefault(texto(json.loads(l)), []).append(i)
    fusoes = []           # (mantido, absorvido, regra, motivo)
    for t, g in sorted(portexto.items()):
        if len(g) < 2:
            continue
        assert len(g) == 2, 'grupo de texto idêntico com %d membros: %s' % (len(g), g)
        g.sort(key=len)
        fusoes.append((g[1], g[0], 'texto-identico',
                       'mesmo texto por dois esquemas de endereço; mantém o específico'))

    # Regra B: julgadas — verificadas contra os dados antes de aplicar
    ja = {i for f in fusoes for i in f[:2]}
    for mantido, absorvido, motivo in JULGADAS:
        assert mantido in regs and absorvido in regs, (mantido, absorvido)
        assert mantido not in ja and absorvido not in ja, (mantido, absorvido)
        rm, ra = json.loads(regs[mantido]), json.loads(regs[absorvido])
        tm = (rm.get('titulo') or '').strip().casefold()
        ta = (ra.get('titulo') or '').strip().casefold()
        assert tm == ta or eh_par_sp(mantido, absorvido), \
            'par julgado sem laço verificável: %s / %s' % (mantido, absorvido)
        fusoes.append((mantido, absorvido, 'julgada', motivo))

    # MANTER: os dois lados têm de existir, ter texto DIFERENTE e SOBREVIVER
    # à curadoria (um mantem que cita endereço absorvido é livro furado)
    absorvidos = {f[1] for f in fusoes}
    for a, b, motivo in MANTIDAS:
        assert a in regs and b in regs, (a, b)
        assert a not in absorvidos and b not in absorvidos, (a, b)
        assert texto(json.loads(regs[a])) != texto(json.loads(regs[b])), (a, b)

    # aplica — z pelos DOIS caminhos (objeto canónico == texto colado)
    dE = 0
    for mantido, absorvido, regra, motivo in fusoes:
        lx, ly = regs[mantido], regs[absorvido]
        z = {'fusao': [json.loads(lx), json.loads(ly)], 'id': mantido,
             'tipo': 'conceito'}
        lz = canon(z)
        colado = '{"fusao":[' + lx + ',' + ly + '],"id":"' + mantido + '","tipo":"conceito"}'
        assert lz == colado, 'os dois caminhos de z divergem: ' + mantido
        esq = '{"fusao":[,],"id":"' + mantido + '","tipo":"conceito"}'
        assert E(lz) - E(lx) - E(ly) == E(esq) - E('') * 2 - 0 == E(esq), mantido
        px, py = fibra(lz)
        assert px == lx and py == ly, 'a fibra não devolve: ' + mantido
        dE += E(lz) - E(lx) - E(ly)
        del regs[absorvido]
        regs[mantido] = lz

    saida = [regs[i] for i in sorted(regs)]
    with open(FONTE, 'w', encoding='utf-8') as f:
        f.write('\n'.join(saida) + '\n')
    with open(TSV, 'w', encoding='utf-8') as f:
        f.write('# curadoria do cristal — 13/08/2026; a fusão guarda as duas partes\n')
        f.write('# acao\tid_resultado\tid_absorvido\tregra\tmotivo\n')
        for mantido, absorvido, regra, motivo in fusoes:
            f.write('funde\t%s\t%s\t%s\t%s\n' % (mantido, absorvido, regra, motivo))
        for a, b, motivo in MANTIDAS:
            f.write('mantem\t%s\t%s\tjulgada\t%s\n' % (a, b, motivo))
    nA = sum(1 for f in fusoes if f[2] == 'texto-identico')
    nB = len(fusoes) - nA
    print('fusões: %d (%d texto-idêntico + %d julgadas); mantidas: %d pares'
          % (len(fusoes), nA, nB, len(MANTIDAS)))
    print('corpus: %d -> %d conceitos; ΔE = +%d (soma dos contornos)'
          % (len(linhas), len(saida), dE))
    print('E(fonte) agora: %d' % sum(E(l) for l in saida))
    return 0


if __name__ == '__main__':
    sys.exit(main())
