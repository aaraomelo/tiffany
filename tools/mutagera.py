#!/usr/bin/env python3
# mutagera.py — GERA as mutações, em vez de as ter escritas à mão.
#
# O tools/mutacao.sh corre um conjunto FIXO de 27 mutações, escolhidas a dedo. Elas valem —
# apanharam buracos reais — mas o conjunto satura: em 03/08 corri-o e 26 foram matadas e a
# 27.ª já estava investigada e documentada como equivalente. A partir daí ele não descobre
# mais nada, porque só testa o que eu já suspeitava.
#
# Este gera as mutações a partir do CÓDIGO, sem eu escolher onde. Foi assim que apareceu o
# buraco do transplante.c: `r + p` -> `r - p` na guarda `r < 0` do mod(), e o medidor verde.
#
# ─── A DISTINÇÃO QUE FAZ ESTA FERRAMENTA VALER ────────────────────────────────────────────
#
# "Sobreviveu" não quer dizer nada sozinho. Há duas razões muito diferentes para uma mutação
# sobreviver, e confundi-las enche o relatório de alarme falso:
#
#   EQUIVALENTE — o output fica BIT A BIT IGUAL. A mutação não mudou o que o programa faz:
#                 ou o código corrige-a a seguir, ou aquele ramo nunca corre. Não é buraco
#                 de asserção; é código não exercitado (ou morto).
#   BURACO      — o output MUDA e o exit continua 0. O programa passou a fazer outra coisa
#                 e nenhuma asserção deu por isso. É aqui que falta medição.
#
# No transplante.c a mutação do mod() era do primeiro tipo — o output era idêntico porque o
# mod nunca recebia negativo. Escrever a asserção que faltava exigiu perceber isso primeiro:
# se eu tivesse tratado as duas categorias como uma, teria procurado a asserção errada.
#
# ─── E OS FILTROS, que são a outra metade ─────────────────────────────────────────────────
#
# A primeira versão desta ferramenta mutava comentários e continuações de asserção, e 16 das
# 18 "sobreviventes" eram ruído meu:
#   - `int D = B*B - 4*Cc;   /* m^2 + 4 */`  — o regex apanhou o COMENTÁRIO, não o código;
#   - `fracao < 1e-6);`                      — é a segunda linha de uma chamada ok(...), e
#                                              mutar ali muda o TESTE e não o que ele testa.
# Por isso aqui há uma máscara a sério: um scanner que marca cada caractere como código,
# string ou comentário, e um parser de parênteses que exclui as chamadas de asserção e de
# impressão INTEIRAS, mesmo quando ocupam cinco linhas.
#
# ─── O PISO, que é a forma mais comum de buraco nesta bateria ─────────────────────────────
#
# Das 89 sobreviventes do primeiro varrimento completo, 26 eram limites de ciclo (`i < n`
# trocado por `i <= n`) e a razão era quase sempre a mesma: a asserção pedia um PISO sobre o
# contador — `casos >= 30`, `n > 10000` — em vez do número. Um piso não deteta que se varreu
# a MAIS, e às vezes nem que se varreu a menos: no palavra.c pedia-se `casos > 3000` e o
# valor real era 17544.
#
# Em 03/08 sondei 27 desses contadores injetando `(printf(valor), condição-original)` no
# lugar da condição: se o valor sai ÚNICO em todas as passagens, a varredura é determinística
# e o piso pode virar igualdade. Os 27 viraram, e as mutações de limite de ciclo passaram a
# ser apanhadas. O método fica dito porque é reutilizável — é mais barato do que caçar cada
# limite de ciclo à mão, e diz o valor em vez de o adivinhar.
#
#   uso:  tools/mutagera.py                    varre os medidores rápidos da bateria
#         tools/mutagera.py corpo.c dual.c     só estes
#         tools/mutagera.py --por 5            5 mutações por medidor (por omissão 3)
#         tools/mutagera.py --ms 1500          teto de tempo por medidor (por omissão 600 ms)
#
# NÃO usa memória nem paralelismo: um medidor de cada vez, com timeout. A máquina do Aarão
# não é para torrar.
#
# ─── O QUE ELA ALCANÇA, e a guarda que estava errada ──────────────────────────────────────
#
# A primeira versão saltava um medidor quando não encontrava a string `return falhas` — a
# mesma guarda do mutacao.sh. Escrevi aqui, com números, que "143 medidores não sabem dizer
# que falharam" e que isso era o maior buraco da bateria.
#
# ERA FALSO, e foi a medição que o disse. Injetei uma falha na primeira asserção de cada um
# dos 143 e vi o código de saída:
#
#      99  SINALIZAM na mesma — usam `if(falhas){ ... return 1; }` seguido de `return 0;`,
#          que o grep não vê. A guarda excluía-os sem motivo.
#      42  não usam ok(...) — idioma próprio, inconclusivo por esta via.
#       2  CEGOS de verdade: toolkit_llm.c e veste.c, corrigidos em 03/08. O veste.c até
#          explicava o mal-entendido num comentário: "o unidade.h imprime o resíduo sozinho,
#          no atexit" — verdade, mas o atexit SÓ IMPRIME, não altera o código de saída.
#
# Por isso a guarda aqui deixou de ser um grep e passou a ser uma MEDIÇÃO: antes de mutar,
# injeta-se uma falha na primeira asserção e verifica-se que o medidor acusa. É uma compilação
# a mais por medidor, e é o que separa "a mutação sobreviveu" de "o medidor não tinha voz".
# Um instrumento que não se verifica a si próprio mede o que lhe apetece.

import os, re, subprocess, sys, tempfile, random

AQUI = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tests')

# ─────────────────────────────────────────────────────────── a máscara do código real

def mascara(src):
    """Devolve uma lista de bool: True onde o caractere é CÓDIGO (nem string, nem comentário)."""
    m = [True]*len(src)
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c == '/' and i+1 < n and src[i+1] == '*':
            j = src.find('*/', i+2); j = n if j < 0 else j+2
            for k in range(i, j): m[k] = False
            i = j
        elif c == '/' and i+1 < n and src[i+1] == '/':
            j = src.find('\n', i); j = n if j < 0 else j
            for k in range(i, j): m[k] = False
            i = j
        elif c in '"\'':
            asp, j = c, i+1
            while j < n:
                if src[j] == '\\': j += 2; continue
                if src[j] == asp: j += 1; break
                if src[j] == '\n': break        # string não fechada: não engolir o resto
                j += 1
            for k in range(i, min(j, n)): m[k] = False
            i = j
        elif c == '#' and (i == 0 or src[i-1] == '\n'):
            j = i                                # directiva: até fim de linha (com continuações)
            while j < n:
                j = src.find('\n', j)
                if j < 0: j = n; break
                if src[j-1] != '\\': break
                j += 1
            for k in range(i, min(j, n)): m[k] = False
            i = j
        else:
            i += 1
    return m

CHAMADAS = ('ok', 'VD', 'printf', 'puts', 'fprintf', 'sprintf', 'snprintf', 'conclui', 'medido')

def zonas_proibidas(src, m):
    """Marca False em m dentro de cada chamada de asserção/impressão, INTEIRA — mesmo
    quando ela ocupa várias linhas. É a correção do defeito que gerava 16 falsos positivos."""
    for mm in re.finditer(r'\b(' + '|'.join(CHAMADAS) + r')\s*\(', src):
        ab = mm.end()-1
        if not m[ab]: continue                   # a própria chamada está em comentário
        d, j = 1, ab+1
        while j < len(src) and d:
            if m[j]:
                if src[j] == '(': d += 1
                elif src[j] == ')': d -= 1
            j += 1
        for k in range(mm.start(), min(j, len(src))): m[k] = False
    return m

# ─────────────────────────────────────────────────────────── as mutações

MUTS = [
    (re.compile(r'(?<=[\w\)\]])\s\+\s(?=[\w\(])'), ' - ', '+ -> -'),
    (re.compile(r'(?<=[\w\)\]])\s-\s(?=[\w\(])'),  ' + ', '- -> +'),
    (re.compile(r'(?<=[\w\)\]])\s\*\s(?=[\w\(])'), ' + ', '* -> +'),
    (re.compile(r'(?<=[\w\)\]\s])<(?!=)'),         '<=',  '< -> <='),
    (re.compile(r'(?<=[\w\)\]\s])>(?!=)'),         '>=',  '> -> >='),
    (re.compile(r'=='),                            '!=',  '== -> !='),
]

IMPRESSAO = re.compile(r'\b(printf|puts|fprintf|putchar|fputs)\s*\(')

def so_imprime(src, m, pos):
    """A mutação está numa condição que só guarda IMPRESSÃO?

    Cinco medidores do núcleo têm a mesma forma:
        if((a==2&&b==3)||(a==2&&b==4)||...) printf(...);
    — uma lista de casos escrita à mão que escolhe QUAIS LINHAS mostrar na tabela. Mutar um
    `==` ali muda o relatório e não o veredito, porque o veredito vive num contador medido
    sobre tudo. São 'buracos' que não valem asserção nova: fechá-los exigiria testar o
    stdout, e uma tabela impressa não é o que a bateria certifica.

    Isto separa-os, para o relatório mostrar o que se pode agir."""
    ini = src.rfind('\n', 0, pos)+1
    cab = src[ini:pos]
    if not re.search(r'\b(if|while)\s*\(', cab): return False
    # do `(` do if até ao `)` que o fecha
    ab = src.index('(', ini + cab.index('if') if 'if' in cab else ini)
    d, j = 1, ab+1
    while j < len(src) and d:
        if m[j]:
            if src[j] == '(': d += 1
            elif src[j] == ')': d -= 1
        j += 1
    if d: return False
    # o corpo guardado: até `;` (instrução) ou o bloco `{...}`
    k = j
    while k < len(src) and src[k] in ' \t\n': k += 1
    if k < len(src) and src[k] == '{':
        d2, e = 1, k+1
        while e < len(src) and d2:
            if m[e]:
                if src[e] == '{': d2 += 1
                elif src[e] == '}': d2 -= 1
            e += 1
        corpo = src[k:e]
    else:
        e = src.find(';', k); corpo = src[k:e+1] if e > 0 else ''
    if not corpo.strip(): return False
    # só imprime se TODA a chamada no corpo for de impressão e não houver atribuição
    chamadas = re.findall(r'\b([a-zA-Z_]\w*)\s*\(', corpo)
    if not chamadas: return False
    if any(c not in ('printf','puts','fprintf','putchar','fputs','sizeof') for c in chamadas):
        return False
    return not re.search(r'[^=!<>+\-*/%&|^]=[^=]', corpo)

def candidatos(src, m):
    """Posições mutáveis: em código, fora de asserções, e com a categoria da linha."""
    fora = []
    for rx, sub, nome in MUTS:
        for mm in rx.finditer(src):
            if all(m[k] for k in range(mm.start(), mm.end())):
                ini = src.rfind('\n', 0, mm.start())+1
                fim = src.find('\n', mm.start()); fim = len(src) if fim < 0 else fim
                linha = src[ini:fim]
                if so_imprime(src, m, mm.start()): cat = 'só-impressão'
                elif re.search(r'\bfor\s*\(', linha): cat = 'limite-de-ciclo'
                else: cat = 'aritmética'
                col = mm.start() - ini + 1
                # o excerto: o que fica de cada lado do ponto mutado, para nao haver duvida
                # sobre QUAL das ocorrencias e' — uma linha com cinco `*` tem cinco mutacoes
                # diferentes, e dizer so' "* -> +" nao chega para ir la' ver.
                exc = (src[max(ini, mm.start()-22):mm.start()] + '»' + src[mm.start():mm.end()]
                       + '«' + src[mm.end():min(fim, mm.end()+22)]).replace('\n', ' ')
                fora.append((mm.start(), mm.end(), sub, nome, cat,
                             src[:mm.start()].count('\n')+1, linha.strip()[:84], col, exc))
    return fora

# ─────────────────────────────────────────────────────────── correr

def injeta_falha(src, n=0):
    """Torna a n-ésima asserção ok(...) falsa, sem tocar em mais nada. Serve para verificar
    que o medidor sabe acusar — antes de lhe perguntar se uma mutação foi apanhada.

    Porque a n-ésima e não sempre a primeira: a primeira ok(...) de um ficheiro está muitas
    vezes num RAMO DE ERRO — `if(!f){ ok("o ficheiro abre", 0); return; }` — que não corre no
    caminho normal. Injetar ali não muda o exit, e eu concluía "este medidor é cego" quando o
    certo era "a asserção que escolhi não é alcançada". Aconteceu com o erg.c, que tem
    `return falhas ? 1 : 0` no fim e foi reportado como sem voz."""
    ms = list(re.finditer(r'\bok\s*\(\s*"', src))
    if n >= len(ms): return None
    m = ms[n]
    j = src.index('"', m.end())
    k = src.index(',', j)
    d, p = 1, k+1
    while p < len(src) and d:
        if src[p] == '(': d += 1
        elif src[p] == ')': d -= 1
        p += 1
    if d: return None
    return src[:k+1] + ' 0 && (' + src[k+1:p-1] + ')' + src[p-1:]

# OS ARGUMENTOS com que a bateria corre cada medidor. Sem eles, nove medidores correm num
# modo DIFERENTE do que a bateria mede — e uma mutação avaliada noutro modo não diz nada
# sobre o que está a ser certificado. A tabela é a mesma do args() do bateria.sh.
# (Descoberto ao ver o fator.c passar na bateria e falhar aqui: ele depende do diretório
#  de trabalho, e esta ferramenta já corre de tools/ — mas os argumentos faltavam.)
ARGS = {
    'neuronio': ['../teoria.tex'], 'neuronio_analog': ['../teoria.tex'],
    'banco': ['teste'], 'sql': ['teste'],
    'linear': ['/tmp/bat.pgm'], 'venom': ['/tmp/bat.pgm'],
    'ancora': ['pares.tsv', '20000'],
    'homogeneo': ['pares.tsv'], 'embedding': ['pares.tsv'],
    'regua': ['pares.tsv'], 'centro': ['pares.tsv'], 'bairro': ['pares.tsv'],
    'operador': ['pares.tsv', '6', '0', '0', '1'],
}

def corre(binario, seg=20, args=()):
    try:
        p = subprocess.run([binario, *args], capture_output=True, timeout=seg)
        return p.returncode, p.stdout
    except subprocess.TimeoutExpired:
        return 124, b''

def main():
    por, teto_ms, alvos = 3, 600, []
    a = sys.argv[1:]
    while a:
        if a[0] == '--por': por = int(a[1]); a = a[2:]
        elif a[0] == '--ms': teto_ms = int(a[1]); a = a[2:]
        else: alvos.append(a.pop(0))
    os.chdir(AQUI)
    if not alvos:
        alvos = sorted(f for f in os.listdir('.') if f.endswith('.c'))

    tmp = tempfile.mkdtemp(prefix='mutagera.')
    mc, mb = os.path.join(tmp, '_m.c'), os.path.join(tmp, '_m')
    buracos, equivalentes, cegos, matadas, saltados = [], [], [], 0, 0

    for f in alvos:
        src = open(f, encoding='utf-8', errors='replace').read()
        # a referência: tem de compilar, passar, e ser rápido
        if subprocess.run(['cc','-O2','-std=c99','-w','-I','.','-I','../lib','-I','../tools',f,'-lm','-o',mb],
                          capture_output=True).returncode:
            saltados += 1; continue
        import time
        arg = ARGS.get(f[:-2], ())
        t0 = time.time(); ec0, out0 = corre(mb, args=arg); dt = (time.time()-t0)*1000
        if ec0 != 0 or dt > teto_ms:
            saltados += 1; continue

        # O CONTROLO DO INSTRUMENTO, antes de o usar: este medidor sabe acusar?
        # Tentam-se VÁRIAS asserções, e não só a primeira — ver injeta_falha(). Basta uma
        # delas fazer o exit mudar para o medidor ter voz.
        acusou, tentadas = False, 0
        for nth in range(6):
            ctl = injeta_falha(src, nth)
            if ctl is None: break
            open(mc, 'w', encoding='utf-8').write(ctl)
            if subprocess.run(['cc','-O2','-std=c99','-w','-I','.','-I','../lib','-I','../tools',mc,'-lm','-o',mb],
                              capture_output=True).returncode:
                continue
            tentadas += 1
            if corre(mb, args=arg)[0] != 0: acusou = True; break
        if not tentadas:
            saltados += 1; continue                  # idioma próprio: inconclusivo por esta via
        if not acusou:
            cegos.append((f, tentadas))              # nenhuma das N acusou
            continue

        m = zonas_proibidas(src, mascara(src))
        cand = candidatos(src, m)
        if not cand: continue
        random.seed(sum(map(ord, f)))            # determinístico: o mesmo ficheiro, as mesmas
        for ini, fim, sub, nome, cat, ln, txt, col, exc in random.sample(cand, min(por, len(cand))):
            open(mc, 'w', encoding='utf-8').write(src[:ini] + sub + src[fim:])
            if subprocess.run(['cc','-O2','-std=c99','-w','-I','.','-I','../lib','-I','../tools',mc,'-lm','-o',mb],
                              capture_output=True).returncode:
                continue                          # não compila: mutação inválida
            ec, out = corre(mb, args=arg)
            if ec != 0:
                matadas += 1
            elif out == out0:
                equivalentes.append((f, ln, col, nome, cat, txt, exc))
            else:
                buracos.append((f, ln, col, nome, cat, txt, exc))

    tot = matadas + len(equivalentes) + len(buracos)
    n_apres = sum(1 for b in buracos if b[4] == 'só-impressão')
    print(f"\nmutações aplicadas: {tot}   matadas: {matadas}   equivalentes: {len(equivalentes)}"
          f"   só-impressão: {n_apres}   BURACOS: {len(buracos) - n_apres}")
    print(f"medidores saltados (não compilam, não passam, lentos, ou sem ok(...)): {saltados}\n")
    if cegos:
        print("SEM VOZ — nenhuma das falhas injetadas mudou o código de saída. Ou o medidor")
        print("não sabe acusar, ou as asserções tentadas não são alcançadas; nos dois casos")
        print("nada que se meça nestes conta, e nenhuma mutação aqui pode ser matada:")
        for f, t in cegos:
            print(f"  {f}  ({t} asserções injetadas, nenhuma mudou o exit)")
        print()

    apres = [b for b in buracos if b[4] == 'só-impressão']
    buracos = [b for b in buracos if b[4] != 'só-impressão']
    if buracos:
        print("BURACOS — o output MUDOU e nenhuma asserção acusou. É aqui que falta medir:")
        for f, ln, col, nome, cat, txt, exc in buracos:
            print(f"  {f}:{ln}:{col}  [{nome}, {cat}]")
            print(f"      {txt}")
            print(f"      exatamente aqui:  {exc}")
        print()
    if apres:
        print("SÓ-IMPRESSÃO — a condição mutada guarda apenas um printf: muda a tabela, não o")
        print("veredito. Não são asserções a faltar; fechá-los exigiria testar o stdout.")
        for f, ln, col, nome, cat, txt, exc in apres:
            print(f"  {f}:{ln}:{col}  [{nome}]  {txt[:76]}")
        print()
    if equivalentes:
        print("equivalentes — o output ficou BIT A BIT IGUAL: aquele ramo não corre, ou o")
        print("código corrige-se a seguir. Não é asserção a faltar; é código por exercitar.")
        for f, ln, col, nome, cat, txt, exc in equivalentes:
            print(f"  {f}:{ln}:{col}  [{nome}, {cat}]")
            print(f"      {txt}")
            print(f"      exatamente aqui:  {exc}")
        print()
    return 1 if (buracos or cegos) else 0

if __name__ == '__main__':
    sys.exit(main())
