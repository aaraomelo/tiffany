#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""ingere.py — o sistema ensina o que ele próprio já escreveu.

O corpus não se inventa: ele já existe. A teoria tem 69 páginas, os papers têm mais, e tudo isso
é TeX — que no catálogo é um corpo, razão 3 e sinal +1, com a SECÇÃO por nível.

Então ingerir é DESCER: cada \\section e \\subsection é um nível, o título é a fala, e o que vem
a seguir até ao nível seguinte é a resposta. Não há extração heurística — há a estrutura que o
documento já declara.

    python3 ingere.py ../teoria.tex | ../banco/bin/conversa <base> -
"""
import re, sys, unicodedata

def limpa(t):
    t = re.sub(r'\\(section|subsection|subsubsection|item|textbf|emph|code|texttt)\*?\{', '', t)
    t = re.sub(r'\\begin\{verbatim\}.*?\\end\{verbatim\}', ' ', t, flags=re.S)
    t = re.sub(r'\\[a-zA-Z]+\*?', ' ', t)
    t = t.replace('}', ' ').replace('{', ' ').replace('$', '').replace('--', '—')
    t = re.sub(r'\s+', ' ', t)
    return t.strip()

def main():
    if len(sys.argv) < 2:
        print("uso: ingere.py <ficheiro.tex>", file=sys.stderr); return 2
    txt = open(sys.argv[1], encoding='utf-8', errors='replace').read()
    # comentário TeX não é conteúdo: cai antes da descida (o %CRISTAL das
    # projeções do cristal, e os % da própria teoria). O \% escapado fica.
    txt = re.sub(r'(?m)(?<!\\)%.*$', '', txt)
    # a descida: cada secção é um nível, e o título é a cabeça dele
    partes = re.split(r'\\(?:sub)*section\*?\{', txt)
    n = 0
    for corpo in partes[1:]:
        fecha = corpo.find('}')
        if fecha < 0: continue
        titulo = limpa(corpo[:fecha])
        resto = limpa(corpo[fecha+1:])[:1200]
        if len(titulo) < 4 or len(resto) < 40: continue
        t = titulo.replace("'", "").replace('"', '')
        r = resto.replace("'", "").replace('"', '')
        print("APRENDE '%s' '%s'" % (t, r))
        n += 1
    # E OS \item: na teoria, cada medidor entra como um item, e é ele a unidade que ensina.
    # O padrão é "\item \code{tools/x.c} --- **o que ele mostra**" ou "\item **título**".
    for m in re.finditer(r'\\item\s+(.{20,4000}?)(?=\\item |\\end\{|\\section)', txt, flags=re.S):
        bloco = m.group(1)
        corte = re.split(r'---|\.\s', limpa(bloco), maxsplit=1)
        if len(corte) < 2: continue
        titulo, resto = corte[0].strip()[:200], corte[1].strip()[:1200]
        # A FALA E A PARTE HUMANA. O item comeca por "tools/x.c" e ninguem pergunta assim — o
        # nome do ficheiro e etiqueta, nao pergunta. Fica o que vem depois dele.
        titulo = re.sub(r'^tools/\S+\s*[-—]*\s*', '', titulo).strip()
        if titulo.startswith('e '): titulo = titulo[2:]
        if len(titulo) < 6 or len(resto) < 40: continue
        t = titulo.replace("'", "").replace('"', '')
        r = resto.replace("'", "").replace('"', '')
        print("APRENDE '%s' '%s'" % (t, r))
        n += 1
    print("-- %d peça(s) ingerida(s) de %s" % (n, sys.argv[1]), file=sys.stderr)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
