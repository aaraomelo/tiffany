#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""cristal_tex.py — projeção LaTeX do cristal (eval.txt: LaTeX = projeção verificável).

Lê cristal/cristal.jsonl (a fonte, extraída por cristal_extrai.py) e emite
cristal/cristal_<grupo>.tex — um documento por grupo de domínios, no formato da
casa: uma \\section por conceito (título = fala; corpo = resposta, ingere.py).

A REGRA DE OURO (eval 13/08): não converter o corpus antes de conseguir
reconstruí-lo. Por isso cada secção é precedida do registo original em
comentário %CRISTAL — o mesmo desenho do /Type/FonteTeX no PDF (a página é a
leitura; a fonte viaja invisível; thm:composicao). O medidor da volta é
tests/cristal_volta.js: extrai os %CRISTAL e compara byte a byte com a fonte.

A projeção visível pode ser lóssica (unicode exótico translitera-se por tabela;
o que a tabela não cobre vira \\uXXXX{} e É CONTADO à vista — sem teto
silencioso). A exactidão vive no canal %CRISTAL.

    python3 tools/cristal_tex.py
"""
import json, os, re, sys, unicodedata

RAIZ = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
FONTE = os.path.join(RAIZ, 'cristal', 'cristal.jsonl')
HIST = os.path.join(RAIZ, 'cristal', 'historia.tsv')
# os cristal_*.tex vivem em cristal/, junto ao cristal.jsonl de que são a face
# embebida — papers/ ficou só com o fundo (universal, topológico, analítico)
PAPERS = os.path.join(RAIZ, 'cristal')


def le_historia():
    """id → nº de versões no jornal do broca-so (a linha do tempo não se amassa)."""
    h = {}
    if os.path.exists(HIST):
        for linha in open(HIST, encoding='utf-8'):
            if linha.startswith('#'):
                continue
            campos = linha.rstrip('\n').split('\t')
            if len(campos) == 2:
                h[campos[0]] = int(campos[1])
    return h

# ── grupos: domínio → ficheiro ────────────────────────────────────────────────
GRUPOS = {
    'matematica': ['matematica', 'algebra', 'fractal', 'continuo', 'reta mineral', 'unidades'],
    'fisica': ['fisica', 'cosmologia', 'cosmologia_dados', 'metal', 'joalheria',
               'litografia', 'mineracao', 'mineracao_pipe'],
    'computacao': ['programacao', 'computacao', 'codigo', 'ia', 'operacao_so', 'sistema',
                   'sistemas', 'sass_gpu', 'criptografia', 'cripto', 'biblioteca_pipe',
                   'goldchain', 'aprendizagem', 'linguagem', 'linguistica'],
    'engenharia': ['engenharia', 'arquitetura', 'cockpit', 'ponte', 'imagem', 'robotica',
                   'manufatura', 'biotecnologia'],
    'ciencias': ['ciencias_de_fora', 'biologia', 'filosofia', 'sagrado', 'medicina',
                 'mercado', 'arte', 'ficcao'],
    'floxina': ['floxina', 'hipotese', 'cristal', 'coracao', 'meta', 'missao', 'metodos',
                'bai_cosmologia'],
    'xadrez': ['xadrez', 'mestres', 'estrategia'],
    'papers': ['paper'],
    'manual': ['manual', 'referencia', 'broca_repos', 'hiper', 'conversa'],
}
TITULO = {
    'matematica': ('a matemática do cristal', 'álgebra, fractais, o contínuo, a reta mineral'),
    'fisica': ('a física do cristal', 'cosmologia, metais, joalheria, litografia'),
    'computacao': ('a computação do cristal', 'programação, sistemas, IA, goldchain'),
    'engenharia': ('a engenharia do cristal', 'arquitetura, cockpit, pontes, imagem, robótica'),
    'ciencias': ('as ciências de fora', 'biologia, filosofia, o sagrado, medicina'),
    'floxina': ('a floxina e as hipóteses', 'a ponta de investigação do projeto'),
    'xadrez': ('o xadrez do cristal', 'partidas, mestres, estratégia'),
    'papers': ('os papers do cristal', 'conceitos extraídos dos papers do projeto'),
    'manual': ('o manual do cristal', 'operação, referência, os repos, a conversa'),
    'diversos': ('os diversos do cristal', 'o que não coube nos outros grupos'),
}

# ── transliteração: unicode exótico → LaTeX (matemática em \ensuremath) ──────
M = {  # math-mode macros
    '−': '-', '→': '\\to ', '↔': '\\leftrightarrow ', '←': '\\leftarrow ',
    '↦': '\\mapsto ', '⇒': '\\Rightarrow ', '⇐': '\\Leftarrow ', '⇔': '\\Leftrightarrow ',
    '⟹': '\\Longrightarrow ', '⟺': '\\Longleftrightarrow ', '↓': '\\downarrow ',
    '↑': '\\uparrow ', '↕': '\\updownarrow ', '⇄': '\\rightleftarrows ', '⇌': '\\rightleftharpoons ',
    '⤷': '\\hookrightarrow ', '√': '\\sqrt{\\,}', '∈': '\\in ', '∉': '\\notin ',
    '≈': '\\approx ', '≤': '\\le ', '≥': '\\ge ', '≠': '\\ne ', '≡': '\\equiv ',
    '≅': '\\cong ', '∝': '\\propto ', '∼': '\\sim ', '≪': '\\ll ', '≫': '\\gg ',
    '≲': '\\lesssim ', '≻': '\\succ ', '≺': '\\prec ', '⪯': '\\preceq ', '≷': '\\gtrless ',
    '≉': '\\not\\approx ', '‖': '\\|', '∞': '\\infty ', '∂': '\\partial ', '∇': '\\nabla ',
    '∆': '\\Delta ', '∑': '\\sum ', '∏': '\\prod ', '∫': '\\int ', '∮': '\\oint ',
    '∗': '*', '∘': '\\circ ', '⊗': '\\otimes ', '⊕': '\\oplus ', '⊖': '\\ominus ',
    '⊙': '\\odot ', '☉': '\\odot ', '⊂': '\\subset ', '⊃': '\\supset ', '⊆': '\\subseteq ',
    '⊊': '\\subsetneq ', '⊄': '\\not\\subset ', '⊥': '\\perp ', '⊣': '\\dashv ',
    '∪': '\\cup ', '∩': '\\cap ', '∖': '\\setminus ', '∅': '\\varnothing ',
    '∀': '\\forall ', '∃': '\\exists ', '¬': '\\neg ', '∧': '\\wedge ', '∨': '\\vee ',
    '△': '\\triangle ', '∎': '\\blacksquare ', '⋆': '\\star ', '⋊': '\\rtimes ',
    '†': '\\dagger ', '′': "'", '∥': '\\parallel ', '℘': '\\wp ', 'ℏ': '\\hbar ',
    'ħ': '\\hbar ', '⌊': '\\lfloor ', '⌋': '\\rfloor ', '⌈': '\\lceil ', '⌉': '\\rceil ',
    '⟨': '\\langle ', '⟩': '\\rangle ',
    'α': '\\alpha ', 'β': '\\beta ', 'γ': '\\gamma ', 'δ': '\\delta ', 'ε': '\\varepsilon ',
    'ζ': '\\zeta ', 'η': '\\eta ', 'θ': '\\theta ', 'ι': '\\iota ', 'κ': '\\kappa ',
    'λ': '\\lambda ', 'μ': '\\mu ', 'µ': '\\mu ', 'ν': '\\nu ', 'ξ': '\\xi ',
    'π': '\\pi ', 'ϖ': '\\varpi ', 'ρ': '\\rho ', 'σ': '\\sigma ', 'τ': '\\tau ',
    'φ': '\\varphi ', 'ϕ': '\\phi ', 'χ': '\\chi ', 'ψ': '\\psi ', 'ω': '\\omega ',
    'Γ': '\\Gamma ', 'Δ': '\\Delta ', 'Θ': '\\Theta ', 'Λ': '\\Lambda ', 'Ξ': '\\Xi ',
    'Π': '\\Pi ', 'Σ': '\\Sigma ', 'Φ': '\\Phi ', 'Ψ': '\\Psi ', 'Ω': '\\Omega ',
    'ℝ': '\\mathbb{R}', 'ℂ': '\\mathbb{C}', 'ℤ': '\\mathbb{Z}', 'ℍ': '\\mathbb{H}',
    'ℚ': '\\mathbb{Q}', 'ℕ': '\\mathbb{N}', 'ℙ': '\\mathbb{P}', '𝕆': '\\mathbb{O}',
    '𝕊': '\\mathbb{S}', '𝔻': '\\mathbb{D}', '𝟙': '\\mathbf{1}',
    '𝔽': '\\mathbb{F}', '𝔤': '\\mathfrak{g}', '𝔊': '\\mathfrak{G}', '𝕶': '\\mathfrak{K}',
    '𝔰': '\\mathfrak{s}', '𝔬': '\\mathfrak{o}', '𝔲': '\\mathfrak{u}',
    '𝒞': '\\mathcal{C}', '𝒪': '\\mathcal{O}', '𝒢': '\\mathcal{G}', '𝓛': '\\mathcal{L}',
    '𝓔': '\\mathcal{E}', '𝓕': '\\mathcal{F}', '𝓖': '\\mathcal{G}', '𝒟': '\\mathcal{D}',
    '𝒫': '\\mathcal{P}', '𝒜': '\\mathcal{A}', '𝒲': '\\mathcal{W}', 'ℱ': '\\mathcal{F}',
    'ℓ': '\\ell ', '℘': '\\wp ', 'ı': '\\imath ',
    '½': '\\tfrac12 ', '¼': '\\tfrac14 ', '⅓': '\\tfrac13 ', '⅔': '\\tfrac23 ',
    '⅙': '\\tfrac16 ', '¹': '^{1}', '²': '^{2}', '³': '^{3}', '⁰': '^{0}',
    '⁴': '^{4}', '⁵': '^{5}', '⁶': '^{6}', '⁷': '^{7}', '⁸': '^{8}', '⁹': '^{9}',
    '⁺': '^{+}', '⁻': '^{-}', 'ⁿ': '^{n}', 'ⁱ': '^{i}', 'ˣ': '^{x}', 'ᵀ': '^{T}',
    'ᵏ': '^{k}', 'ᵐ': '^{m}', 'ᵗ': '^{t}', 'ᵘ': '^{u}', 'ᵅ': '^{\\alpha}',
    'ᴺ': '^{N}', '₀': '_{0}', '₁': '_{1}', '₂': '_{2}', '₃': '_{3}', '₄': '_{4}',
    '₅': '_{5}', '₆': '_{6}', '₇': '_{7}', '₈': '_{8}', '₉': '_{9}',
    '₊': '_{+}', '₋': '_{-}', 'ₖ': '_{k}', 'ₙ': '_{n}', 'ₛ': '_{s}', 'ₐ': '_{a}',
    'ₚ': '_{p}', 'ₘ': '_{m}', 'ₑ': '_{e}', 'ᵢ': '_{i}', 'ⱼ': '_{j}', 'ᵦ': '_{\\beta}',
    '₍': '_{(}', '₎': '_{)}',
    'ā': '\\bar{a}', 'ī': '\\bar{i}', '¯': '\\bar{\\,}', '̄': '', '̂': '', '̃': '',
    'ẋ': '\\dot{x}', 'ẏ': '\\dot{y}', 'ż': '\\dot{z}', 'ṡ': '\\dot{s}', 'Ṡ': '\\dot{S}',
    'ȧ': '\\dot{a}', 'ṣ': '\\d{s}', 'ẑ': '\\hat{z}', 'Ẑ': '\\hat{Z}', 'Ŝ': '\\hat{S}',
    'Ĥ': '\\hat{H}', 'ẍ': '\\ddot{x}', '̇': '', '̈': '', '̸': '',
    'Å': '\\text{\\AA}', 'Ø': '\\varnothing ', 'ø': '\\varnothing ',
}
T = {  # text-mode
    '─': '-', '│': '|', '┃': '|', '├': '|-', '┘': '', '┌': '', '┐': '', '└': '',
    '┬': '-', '┼': '+', '■': '\\rule{1ex}{1ex}', '□': '$\\square$', '◆': '$\\blacklozenge$',
    '◇': '$\\lozenge$', '★': '$\\star$', '•': '\\textbullet ', '·': '\\textperiodcentered ',
    '►': '$\\triangleright$', '◄': '$\\triangleleft$', '▼': '$\\triangledown$',
    '✓': '\\checkmark ', '✅': '[ok]', '❌': '[x]', '⚙': '[engr.]', '⚔': '[espadas]',
    '🌱': '[semente]', '🌞': '[sol]', '🌿': '[ramo]', '🌉': '[ponte]', '🪨': '[pedra]',
    '️': '', 'Ⓚ': '(K)', '¸': ',', '˜': '\\~{}', 'ß': '\\ss ', 'ć': "\\'c",
    '×': '$\\times$', '±': '$\\pm$', '°': '$^{\\circ}$', 'ºC': '\\textcelsius ',
    # artefactos de OCR dos PDFs de origem: acento solto e fragmentos de
    # chaveta grande (PUA F8F1-F8F4, o ⎧⎨⎩ de um cases) — projeta-se o essencial
    '´': "'", '': '\\{', '': '', '': '', '': '',
}

ESPECIAIS = {'\\': '\\textbackslash{}', '{': '\\{', '}': '\\}', '$': '\\$', '&': '\\&',
             '#': '\\#', '^': '\\^{}', '_': '\\_', '%': '\\%', '~': '\\~{}'}
fallbacks = {}


def _indice(ch):
    """Conteúdo de um sub/sobrescrito unicode, ou None."""
    v = M.get(ch, '')
    if v.startswith('_{') or v.startswith('^{'):
        return v[0], v[2:-1]
    return None


def tex_txt(s):
    out = []
    i, n = 0, len(s)
    while i < n:
        ch = s[i]
        idx = _indice(ch)
        if idx:
            # junta a sequência ₋₁ / ⁻¹ num só _{...} — dois seguidos é erro TeX
            tipo, corpo = idx
            i += 1
            while i < n and _indice(s[i]) and _indice(s[i])[0] == tipo:
                corpo += _indice(s[i])[1]
                i += 1
            out.append('\\ensuremath{%s{%s}}' % (tipo, corpo))
            continue
        i += 1
        if ch == '\t':
            out.append(' ')
        elif ord(ch) < 0x20 and ch != '\n':
            fallbacks[ch] = fallbacks.get(ch, 0) + 1   # controlo: cai fora, contado
        elif ch in ESPECIAIS:
            out.append(ESPECIAIS[ch])
        elif ch in M:
            out.append('\\ensuremath{%s}' % M[ch].rstrip())
        elif ch in T:
            out.append(T[ch])
        elif ord(ch) < 0x7F or ch in 'áàâãäéèêëíìîïóòôõöúùûüçñÁÀÂÃÄÉÈÊËÍÌÎÏÓÒÔÕÖÚÙÛÜÇÑ«»ºª—–…‘’“”§':
            out.append(ch)
        else:
            fallbacks[ch] = fallbacks.get(ch, 0) + 1
            out.append('\\texttt{U+%04X}' % ord(ch))
    return ''.join(out)


CAB = r'''%% !TeX program = pdflatex
%% papers/cristal_%(g)s.tex — GERADO por tools/cristal_tex.py; NÃO editar à mão.
%% Fonte: cristal/cristal.jsonl (broca-so, última versão por conceito).
%% Cada secção carrega o registo original em comentário %%CRISTAL — a volta é
%% tests/cristal_volta.js (resíduo 0 byte a byte contra a fonte).
\documentclass[11pt,a4paper]{article}
\usepackage[utf8]{inputenc}\usepackage[T1]{fontenc}\usepackage[brazil]{babel}
\usepackage{amsmath,amssymb}\usepackage{textcomp}
\usepackage[margin=2.4cm]{geometry}\usepackage{xcolor}
\definecolor{cinza}{gray}{0.35}
\newcommand{\prov}[1]{\par\smallskip\noindent{\small\color{cinza}#1}\par}
\setcounter{secnumdepth}{0}
\input{gkcapa}
\begin{document}
\gkcapa{O Cristal --- %(titulo)s}
       {%(sub)s}
       {corpus recuperado do broca-so; %(n)d conceitos; projeção verificável da fonte}
\maketitle
'''


def corpo_secao(r, hid):
    """As linhas visíveis de um conceito; hid é o endereço (id do registo na
    fonte — numa fusão, o id mantido)."""
    linhas = ['\\section{%s}' % tex_txt(r.get('titulo') or hid)]
    d = (r.get('descricao') or '').strip()
    if d:
        linhas.append(tex_txt(d))
    ex = [e.strip() for e in r.get('exemplos', []) if e.strip()]
    cx = [e.strip() for e in r.get('contraexemplos', []) if e.strip()]
    si = [e.strip() for e in r.get('sinonimos', []) if e.strip()]
    if ex:
        linhas.append('\\prov{exemplos: %s}' % tex_txt('; '.join(ex)))
    if cx:
        linhas.append('\\prov{contraexemplos: %s}' % tex_txt('; '.join(cx)))
    if si:
        linhas.append('\\prov{sinónimos: %s}' % tex_txt('; '.join(si)))
    ar = r.get('arestas') or []
    if ar:
        rel = '; '.join('%s %s' % (a.get('rel', '?'), a.get('alvo', '?')) for a in ar)
        linhas.append('\\prov{relações: %s}' % tex_txt(rel))
    meta = r.get('meta') or {}
    nv = HISTORIA.get(hid, 1)
    pv = ' \\textperiodcentered{} '.join(filter(None, [
        tex_txt(str(r.get('origem', ''))),
        tex_txt(str(meta.get('fonte', ''))),
        tex_txt(str(r.get('epistemico', ''))),
        'confiança %s' % r.get('confianca', '?'),
        'domínio %s' % tex_txt(str(meta.get('dominio', '(sem)'))),
        ('história %d versões no jornal' % nv) if nv > 1 else '',
    ]))
    linhas.append('\\prov{proveniência: %s}' % pv)
    return linhas


def secao(r):
    """Uma \\section por registo da fonte; o registo viaja no %CRISTAL. Um
    registo de fusão (tools/cristal_cura.py) desdobra-se: corpo da face
    mantida, segunda face à vista quando a prosa difere, nota de curadoria."""
    cab = ['%CRISTAL ' + json.dumps(r, ensure_ascii=False, sort_keys=True,
                                    separators=(',', ':'))]
    if 'fusao' not in r:
        return '\n'.join(cab + corpo_secao(r, r['id'])) + '\n'
    x, y = r['fusao']
    linhas = cab + corpo_secao(x, r['id'])
    dx = (x.get('descricao') or '').strip()
    dy = (y.get('descricao') or '').strip()
    if dy and dy != dx:
        linhas.append('\\prov{a segunda face --- %s:}' % tex_txt(y.get('id', '?')))
        linhas.append(tex_txt(dy))
        nota = ('fusão de curadoria (julgada): absorve %s --- as duas faces '
                'acima, intactas no registo') % y.get('id', '?')
    else:
        nota = ('fusão de curadoria (texto idêntico): absorve %s --- o mesmo '
                'conteúdo por dois esquemas de endereço') % y.get('id', '?')
    linhas.append('\\prov{%s}' % tex_txt(nota))
    return '\n'.join(linhas) + '\n'


HISTORIA = {}


def main():
    HISTORIA.update(le_historia())
    dom2grupo = {d: g for g, ds in GRUPOS.items() for d in ds}
    grupos = {g: [] for g in list(GRUPOS) + ['diversos']}
    with open(FONTE, encoding='utf-8') as f:
        for linha in f:
            r = json.loads(linha)
            meta = r.get('meta') or ('fusao' in r and r['fusao'][0].get('meta')) or {}
            dom = meta.get('dominio', '(sem)')
            grupos[dom2grupo.get(dom, 'diversos')].append(r)
    total = 0
    for g, rs in grupos.items():
        titulo, sub = TITULO[g]
        caminho = os.path.join(PAPERS, 'cristal_%s.tex' % g)
        with open(caminho, 'w', encoding='utf-8') as f:
            f.write(CAB % {'g': g, 'titulo': titulo, 'sub': sub, 'n': len(rs)})
            for r in rs:
                f.write('\n' + secao(r))
            f.write('\n\\end{document}\n')
        total += len(rs)
        print('papers/cristal_%s.tex: %d conceitos' % (g, len(rs)))
    print('total: %d conceitos em %d ficheiros' % (total, len(grupos)))
    if fallbacks:
        print('fallbacks U+XXXX (%d chars, %d ocorrências): %s' %
              (len(fallbacks), sum(fallbacks.values()),
               ' '.join('%s×%d' % (k, v) for k, v in
                        sorted(fallbacks.items(), key=lambda x: -x[1]))))
    else:
        print('fallbacks: nenhum — a tabela cobre tudo')
    return 0


if __name__ == '__main__':
    sys.exit(main())
