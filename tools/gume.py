#!/usr/bin/env python3
# gume.py — O GUME AUTOMÁTICO DAS ASSERÇÕES: quais delas sobrevivem a tudo.
#
# O gume era feito à mão. Para cada asserção nova eu escrevia um `sed` que estragava
# UMA coisa e via se o medidor caía. Funciona, é lento, e tem o defeito de eu escolher
# a mutação — logo aponto-a ao sítio onde já sei que ela morde, que foi exactamente
# como duas tautologias minhas passaram no `dif.c`.
#
# Isto tira-me a escolha das mãos. Muta o ficheiro MECANICAMENTE, uma alteração de cada
# vez, corre o medidor, e regista que unidades passaram a falhar. No fim diz o que
# interessa:
#
#     A UNIDADE QUE SOBREVIVE A TODAS AS MUTAÇÕES É SUSPEITA.
#
# Não é prova de que está errada — pode ser que nenhuma mutação toque no que ela mede.
# Mas é o sítio onde olhar, e é a mesma pergunta de sempre, feita por máquina em vez de
# por mim: QUE ENTRADA FARIA ISTO FALHAR?
#
# O que ele NÃO faz, e é deliberado:
#   - não julga: reporta as sobreviventes e cala-se, porque a decisão é do Aarão;
#   - não muta comentários nem strings — mudar o TEXTO de uma asserção não a testa,
#     e uma mutação dentro de uma string é ruído com cara de medida;
#   - não conta mutações que não compilam: um ficheiro que não compila não mede nada,
#     e contá-lo como «matou a asserção» era a mensagem que não pode falhar outra vez.
#
#   uso:  python3 tools/gume.py tests/x.c [--max N] [--ver]

import re, subprocess, sys, os, tempfile, shutil

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TMP  = os.environ.get('GUME_TMP') or tempfile.mkdtemp(prefix='gume_')

# As mutações. Cada uma é (regex, substituição, nome) e aplica-se a UMA ocorrência por
# variante. São mecânicas de propósito: uma mutação que eu escolhesse a pensar no
# medidor testaria a minha ideia dele, e não ele.
MUT = [
    (r'==',            '!=',           'igual→diferente'),
    (r'!=',            '==',           'diferente→igual'),
    (r'(?<![<>=!])<(?![<=])',  '<=',   'menor→menor-igual'),
    (r'(?<![<>=!])>(?![>=])',  '>=',   'maior→maior-igual'),
    (r'&&',            '||',           'e→ou'),
    (r'\+\+',          '--',           'incrementa→decrementa'),
    (r'(?<![\w.])(\d+)(?![\w.])', None, 'constante+1'),
]

def campos_de_codigo(linha):
    """Devolve os intervalos da linha que são CÓDIGO — fora de string e de comentário.
    Sem isto a ferramenta mutava o texto das asserções, que é o que elas dizem e não o
    que elas fazem."""
    fora, i, n = [], 0, len(linha)
    ini = 0
    while i < n:
        c = linha[i]
        if c == '"':
            fora.append((ini, i)); i += 1
            while i < n and linha[i] != '"':
                i += 2 if linha[i] == '\\' else 1
            i += 1; ini = i; continue
        if c == "'":
            fora.append((ini, i)); i += 1
            while i < n and linha[i] != "'":
                i += 2 if linha[i] == '\\' else 1
            i += 1; ini = i; continue
        if linha[i:i+2] in ('/*', '//'):
            fora.append((ini, i)); return fora
        i += 1
    fora.append((ini, n))
    return fora

def gera(fonte):
    """Cada variante é (linha_1based, nome_da_mutacao, texto_completo_mutado)."""
    linhas = fonte.split('\n')
    fora_bloco, dentro = [], False
    for L in linhas:
        t = L.strip()
        if dentro:
            fora_bloco.append(False)
            if '*/' in t: dentro = False
            continue
        if t.startswith('/*') and '*/' not in t:
            dentro = True; fora_bloco.append(False); continue
        fora_bloco.append(not (t.startswith('*') or t.startswith('//') or t.startswith('#')))

    out = []
    for i, L in enumerate(linhas):
        if not fora_bloco[i]:
            continue
        for a, b in campos_de_codigo(L):
            trecho = L[a:b]
            for pad, sub, nome in MUT:
                for mo in re.finditer(pad, trecho):
                    if sub is None:                       # constante+1
                        v = int(mo.group(1))
                        if v > 10**9: continue
                        novo = str(v + 1)
                    else:
                        novo = sub
                    mut = L[:a] + trecho[:mo.start()] + novo + trecho[mo.end():] + L[b:]
                    if mut == L: continue
                    nl = list(linhas); nl[i] = mut
                    out.append((i + 1, nome, '\n'.join(nl)))
    return out

def unidades(saida):
    """{rótulo: passou?} — a mesma linha `#UNIT` que a bateria lê."""
    d = {}
    for L in saida.split('\n'):
        mo = re.match(r'^#UNIT (ok|falha|salta)\s+(.*)$', L)
        if mo:
            d[mo.group(2).strip()] = (mo.group(1) == 'ok')
    return d

def corre(caminho_c, tag):
    bina = os.path.join(TMP, 'g_' + tag)
    cc = subprocess.run(['cc', '-O2', '-std=c99', '-I', os.path.join(RAIZ, 'lib'),
                         '-I', os.path.join(RAIZ, 'tests'), '-o', bina, caminho_c, '-lm'],
                        capture_output=True, timeout=180)
    if cc.returncode != 0 or not os.path.exists(bina):
        return None                                   # não compilou: não informa nada
    try:
        r = subprocess.run([bina], capture_output=True, timeout=90,
                           cwd=os.path.join(RAIZ, 'tests'))
    except subprocess.TimeoutExpired:
        return None
    finally:
        if os.path.exists(bina): os.unlink(bina)
    return unidades(r.stdout.decode('utf-8', 'replace'))

def main():
    if len(sys.argv) < 2:
        print(__doc__ or 'uso: python3 tools/gume.py tests/x.c [--max N]'); return 2
    alvo = sys.argv[1]
    teto = 120
    if '--max' in sys.argv: teto = int(sys.argv[sys.argv.index('--max') + 1])
    ver  = '--ver' in sys.argv

    fonte = open(os.path.join(RAIZ, alvo), encoding='utf-8').read()
    base_c = os.path.join(TMP, '_base.c')
    open(base_c, 'w', encoding='utf-8').write(fonte)
    base = corre(base_c, 'base')
    if base is None:
        print(f"  {alvo}: o ORIGINAL não compila ou não corre — nada a medir"); return 1
    vivas = {r for r, p in base.items() if p}
    if not vivas:
        print(f"  {alvo}: o original não tem unidade verde nenhuma"); return 1

    todas = gera(fonte)
    # amostragem uniforme, e o que ficou de fora é DITO: um tecto calado leria-se como
    # cobertura completa, que é a lição da saturação.
    passo = max(1, len(todas) // teto)
    amostra = todas[::passo][:teto]

    print(f"  {alvo}: {len(vivas)} unidades verdes, {len(todas)} mutações possíveis, "
          f"{len(amostra)} corridas" + (f" (1 em cada {passo})" if passo > 1 else ""))

    morreu = {r: 0 for r in vivas}
    uteis = 0
    for k, (ln, nome, texto) in enumerate(amostra):
        mc = os.path.join(TMP, '_m.c')
        open(mc, 'w', encoding='utf-8').write(texto)
        res = corre(mc, 'm')
        if res is None: continue
        uteis += 1
        for r in vivas:
            if r not in res or not res[r]:
                morreu[r] += 1
        if ver:
            mortos = sum(1 for r in vivas if r not in res or not res[r])
            print(f"      linha {ln:5d}  {nome:22s}  matou {mortos}")

    print(f"      mutações que compilaram e correram: {uteis} de {len(amostra)}")
    sobrevive = [r for r in vivas if morreu[r] == 0]
    print(f"      unidades mortas por alguma mutação : {len(vivas) - len(sobrevive)}")
    print(f"      unidades que SOBREVIVERAM a todas  : {len(sobrevive)}")
    for r in sobrevive:
        print(f"        · {r[:100]}")
    if not sobrevive:
        print("      — todas caem sob mutação: nenhuma passa sem poder falhar")
    return 0

if __name__ == '__main__':
    try:
        sys.exit(main())
    finally:
        shutil.rmtree(TMP, ignore_errors=True)
