#!/usr/bin/env python3
"""Funde corpo_computacional.tex + computacional.tex → papers/arquitetura.tex."""
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[1]
corpo = (RAIZ / "papers/corpo_computacional.tex").read_text()
comp = (RAIZ / "papers/computacional.tex").read_text()

extra_macros = r"""
\newcommand{\sepcol}{\quad\penalty0$\big|$\quad\penalty0}
\newcommand{\colunas}[3]{%
\par\smallskip\noindent{\footnotesize\raggedright
\textbf{prova} (Universal): #1\sepcol
\textbf{realiza} (aqui): #2\sepcol
\textbf{verifica}: #3\par}\smallskip}
\newtheorem{lema}{Lema}
"""

corpo = corpo.replace(
    r"\newtheorem{definicao}[teorema]{Definição}",
    extra_macros + r"\newtheorem{definicao}[teorema]{Definição}",
)

corpo = corpo.replace(
    "%  papers/corpo_computacional.tex — O CORPO COMPUTACIONAL (era dualsort.tex, 18/08).\n"
    "%\n"
    "%  Compila sozinho:  pdflatex corpo_computacional.tex",
    "%  papers/arquitetura.tex — ARQUITETURA COMPUTACIONAL (fusão 08/2026).\n"
    "%\n"
    "%  Compila: pdflatex arquitetura.tex",
)

old_capa = (
    r"\gkcapa{A realização COMPUTACIONAL}"
    "\n       {o dual sort: ordenar é descer pela estaca}"
    "\n       {uma realização DENTRO do $\tau=-1$ --- não uma quarta casa}"
)
new_capa = (
    r"\gkcapa{A Arquitetura}"
    "\n       {dual sort · régua · torre · slots · aranha estigmérgica}"
    "\n       {realização dentro do $\tau=-1$ --- ordenar, banco e firma algébrica}"
)
corpo = corpo.replace(old_capa, new_capa)

abs_comp = comp.split(r"\begin{abstract}")[1].split(r"\end{abstract}")[0]
parts = abs_comp.split(r"\smallskip")
if len(parts) > 1:
    synth_abs = r"\smallskip\noindent" + parts[1]
    synth_abs = synth_abs.replace(
        r"\textbf{Não substitui} \emph{Corpo Computacional} (dual sort e banco). \textbf{Unifica}",
        r"\textbf{Unifica}",
    )
    corpo = corpo.replace(r"\end{abstract}", synth_abs + r"\end{abstract}")

start = corpo.find(r"\section{Envelopamento $E_{16}$")
if start != -1:
    end = corpo.find(r"\vfill", start)
    corpo = corpo[:start] + corpo[end:]

comp_body_start = comp.find(r"\section{O lema: régua antes do espaço}")
comp_body_end = comp.find(r"\section*{Inventário")
comp_body = comp[comp_body_start:comp_body_end]

estig = comp_body.find(r"\medskip\noindent\textbf{Estigmergia}")
if estig != -1:
    div = comp_body.find(r"\medskip\noindent\textbf{Divisor de águas}", estig)
    comp_body = comp_body[:estig] + comp_body[div:]

comp_body = comp_body.replace(
    r"\section{O lema: régua antes do espaço}",
    r"\part*{Firma algébrica: régua, torre e slots}"
    r"\addcontentsline{toc}{section}{Firma algébrica}"
    r"\n\n\section{O lema: régua antes do espaço}",
)
comp_body = comp_body.replace(r"\code{computacional}", r"este documento")
comp_body = comp_body.replace(
    r"\code{papers/computacional.tex}", r"\code{papers/arquitetura.tex}"
)
comp_body = comp_body.replace(
    r"ordenar / banco & \code{tests/dualsort.c} & ver \code{computacional} \\",
    r"ordenar / banco & \code{tests/dualsort.c} & ver §\ref{sec:pecas} \\",
)

spider = r"""
\section{Algoritmo da aranha estigmérgica}\label{sec:aranha}

\noindent A aranha não \emph{cria} o padrão: é \textbf{realização dinâmica da régua}.
O mapa não vive na cabeça do agente --- é \textbf{estado do ambiente}; a regra local
lê$\to$opera$\to$escreve e a geometria global emerge por retroacção
(decisão do coordenador, 08/2026).

\medskip\noindent\textbf{Ciclo estigmérgico.}
\begin{enumerate}\itemsep3pt
\item \textbf{Escrita} --- incrementa o traço na célula actual ($G[y,x]\mathrel{+}=1$):
      a operação \emph{materializa} a passagem no ambiente.
\item \textbf{Leitura} --- consulta as quatro vizinhanças relativas; fronteira
      $\Rightarrow$ obstáculo ($+\infty$).
\item \textbf{Decisão} --- prefere direcções com \emph{menor} histórico de visitação
      (gradiente de exploração): afasta-se das trilhas que acabou de pavimentar.
\item \textbf{Fecho} --- após $N_{\mathrm{expl}}$ passos, fase determinística de
      fechamento geométrico ($R^{4}=\mathrm{id}$): rotação à direita + marca forte.
\end{enumerate}

\medskip\noindent\textbf{Propriedades} (modelo \code{StigmergicSpiderSimulation},
decisão do coordenador):
\begin{itemize}\itemsep2pt
\item \emph{Feedback real de leitura:} o agente consulta \code{perceptions} gerado
      a partir de $G$ --- ``sente'' onde já esteve.
\item \emph{Evitação por gradiente:} dispersão simétrica antes do rotor de fechamento.
\item \emph{Persistência sem histórico interno:} a trajetória inteira suporta-se na
      matriz acumulativa --- memória $\neq$ armazém narrativo.
\end{itemize}

\medskip\noindent\textbf{Ponte com slots e banco.}
O mesmo ciclo lê$\to$opera$\to$escreve governa autómato, tradutor, \code{sql.c} e
\code{conversa.c}: a transformação $G\mapsto G'$ não é memória do agente --- é
transformação do espaço de trabalho (\S\ref{sec:slots}). No limite
$\mathrm{GF}(2)$, o agente reduz-se ao \textbf{ciclo de clock} do operador algébrico
sobre espaço restrito --- não por heurística, mas porque a descida fechou invariantes
(\S\ref{sec:descida}).

\medskip\noindent Em uma linha (decisão do coordenador):
\[
\boxed{\text{não é o agente que conhece o espaço; é a régua que torna o
espaço legível.}}
\]

\medido{Modelo conceptual em decisão do coordenador; ciclo de slots
\code{tests/reta.c} §R12d--§R12g; \code{tests/dualsort\_banco.c}; álgebra em
GF(2) \code{tests/cadeia.c} §K3.}
\colunas{decisão do coordenador (aranha estigmérgica); \code{algebrico def:cone} (retração)}
        {ciclo lê$\to$opera$\to$escreve; $G$ como estado geométrico}
        {\code{tests/cadeia.c} §K3; \code{tests/reta.c} §R12d--§R12g}

"""

ins = comp_body.find(r"\section{Descida de representação}")
comp_body = comp_body[:ins] + spider + comp_body[ins:]

inventario = comp[comp_body_end:].replace(
    r"\code{sintese} registado). PDF no site: \code{/docs/realizacao.pdf} (tradutor:"
    r"\code{papers/computacional.tex}). Números da tabela: \code{bash tools/bateria.sh}",
    r"\code{computacional}/\code{sintese} registados). PDF: \code{/docs/computacional.pdf}"
    r" (\code{papers/arquitetura.tex}). Números: \code{bash tools/bateria.sh}",
)
inventario = inventario.replace(
    r"Irmãos: \code{corpo\_algebrico.tex} · \code{corpo\_computacional.tex} ·",
    r"Irmãos: \code{corpo\_algebrico.tex} ·",
)

footer = r"""
\vfill
{\footnotesize\color{cinza}\noindent
Teoria e catálogo em
\href{https://goldenkingdom.patriatechnology.com}{goldenkingdom.patriatechnology.com}.
CC BY 4.0 (textos) $\cdot$ MIT (código).\par}

\end{document}
"""

vfill = corpo.find(r"\vfill")
out = corpo[:vfill] + comp_body + inventario + footer
(RAIZ / "papers/arquitetura.tex").write_text(out)
print(f"OK: {len(out.splitlines())} linhas → papers/arquitetura.tex")
