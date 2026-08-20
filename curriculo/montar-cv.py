#!/usr/bin/env python3
"""Monta cv.tex = índice + 3 versões com divisas. Fontes: cv-{executivo,parcerias,academico}.tex"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def body(name: str) -> str:
    text = (ROOT / name).read_text()
    start = text.index(r"\begin{document}") + len(r"\begin{document}")
    end = text.rindex(r"\end{document}")
    return text[start:end].strip()


def main() -> None:
    out = ROOT / "cv.tex"
    out.write_text(
        r"""% cv.tex — as três versões concatenadas (executivo + parcerias + académico).
% Gerado por montar-cv.py — não editar à mão; edite as fontes e rode: python3 montar-cv.py
% Fontes: cv-executivo.tex, cv-parcerias.tex, cv-academico.tex
\documentclass[11pt,a4paper]{article}
\usepackage[T1]{fontenc}
\usepackage[utf8]{inputenc}
\usepackage[portuguese]{babel}
\usepackage[margin=2.0cm]{geometry}
\usepackage{enumitem}
\usepackage{titlesec}
\usepackage{xcolor}
\usepackage[hidelinks]{hyperref}
\usepackage{booktabs}
\usepackage{amssymb}

\definecolor{cinza}{gray}{0.35}
\titleformat{\section}{\large\bfseries}{}{0pt}{}[\vspace{-6pt}\rule{\textwidth}{0.4pt}]
\titlespacing{\section}{0pt}{9pt}{4pt}
\newcommand{\lin}[2]{\noindent\textbf{#1}\hfill{\color{cinza}\small #2}\par}
\setlist[itemize]{leftmargin=1.2em,itemsep=1pt,topsep=2pt,parsep=0pt}
\pagestyle{empty}
\newcommand{\divisa}[3]{%
  \noindent\rule{\textwidth}{1.2pt}\\[6pt]
  {\large\bfseries #1}\\[2pt]
  {\color{cinza}\small #2}\\[4pt]
  {\small #3}\\[6pt]
  \noindent\rule{\textwidth}{0.4pt}\par
  \vspace{8pt}%
}
\begin{document}

% Proprietário · Copyright (c) 2026 Aarão Melo Lopes · ver LICENSE na raiz.

\newgeometry{margin=2.5cm}
\begin{center}
{\LARGE\bfseries Aarão Melo Lopes}\\[6pt]
{\color{cinza}Currículo --- três versões no mesmo ficheiro}\\[14pt]
\end{center}

\noindent Este PDF junta \textbf{três currículos distintos}, não três cópias.
Escolha a secção conforme o contexto; ignore as outras.

\medskip
\begin{center}
\begin{tabular}{@{}clp{8.2cm}@{}}
\toprule
\textbf{pág.} & \textbf{versão} & \textbf{para quem} \\
\midrule
2 & Executivo (1\,pág.) &
Recrutadores e empresas --- formação, competências, resultados. \\[4pt]
3--4 & Parcerias (2\,pág.) &
Primeiro contacto com laboratórios, indústria ou criadores. \\[4pt]
5--7 & Académico (3\,pág.) &
Professores, grupos de pesquisa e quem pediu detalhe. \\
\bottomrule
\end{tabular}
\end{center}

\medskip
\noindent{\small\color{cinza}
As versões também existem em separado:
\texttt{cv-executivo.pdf}, \texttt{cv-parcerias.pdf}, \texttt{cv-academico.pdf}.}
\par

\vfill
\begin{center}
{\footnotesize
\href{mailto:aarao.melo.lopes@gmail.com}{aarao.melo.lopes@gmail.com}
\,·\,
\href{https://goldenkingdom.patriatechnology.com}{goldenkingdom.patriatechnology.com}
\\[4pt]
Proprietário — contrato e pagamento (LICENSE)}
\end{center}

\newpage
% ═══ 1. EXECUTIVO ═══
\newgeometry{margin=1.6cm}
\titlespacing{\section}{0pt}{7pt}{3pt}
\titleformat{\section}{\normalsize\bfseries}{}{0pt}{}[\vspace{-5pt}\rule{\textwidth}{0.4pt}]
\setlist[itemize]{leftmargin=1.1em,itemsep=0pt,topsep=1pt,parsep=0pt}

\divisa{Versão 1 de 3 --- Executivo}
{1 página \,·\, para recrutadores e empresas}
{Resumo: formação, competências, sistemas e o que já se verifica.}

"""
        + body("cv-executivo.tex")
        + r"""

\newpage
% ═══ 2. PARCERIAS ═══
\newgeometry{margin=2.0cm}
\titlespacing{\section}{0pt}{9pt}{4pt}
\titleformat{\section}{\large\bfseries}{}{0pt}{}[\vspace{-6pt}\rule{\textwidth}{0.4pt}]
\setlist[itemize]{leftmargin=1.2em,itemsep=1pt,topsep=2pt,parsep=0pt}

\divisa{Versão 2 de 3 --- Parcerias}
{2 páginas \,·\, primeiro contacto}
{Para laboratórios, indústria e criadores. Pedido concreto; propostas I--III em forma curta.}

"""
        + body("cv-parcerias.tex")
        + r"""

\newpage
% ═══ 3. ACADÉMICO ═══
\newgeometry{margin=2.2cm}
\titlespacing{\section}{0pt}{12pt}{5pt}

\divisa{Versão 3 de 3 --- Académico}
{3 páginas \,·\, research statement resumido}
{Para professores e grupos de pesquisa. Mais prosa, propostas e ligação TCC$\to$teoria.}

"""
        + body("cv-academico.tex")
        + "\n\n\\end{document}\n"
    )
    print(f"escrito {out}")


if __name__ == "__main__":
    main()
