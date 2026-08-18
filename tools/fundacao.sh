#!/bin/sh
# fundacao.sh — Cresce o corpus pela escada das fundações (como o casual).
#
#   Corpos → Dinâmica → Topologia → Hilbert/Clifford  (Peano só no fim)
#   Fontes: teoria.tex, corpus/docs/torre_fundacao.tex, tests/{topologia,hurwitz,hilbert_bidual}.c
#
#   cd banco && ../tools/fundacao.sh .fala/<hex>
#   ARVORE=1 ../tools/fundacao.sh .fala/<hex>   # + gabarito_arvore nas sementes
#
set -e
B="${1:?uso: ./fundacao.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }
mkdir -p "$B"
aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

# ========== Banda F0 — porta (onde está a escada) ==========
aprende "mostra a fundação" "Corpos→Dinâmica→Topologia→Hilbert/Clifford→Peano. corpus/docs/torre_fundacao.tex"
aprende "torre de fundação" "a escada antes do Corpo de Peano: corpus/docs/torre_fundacao.tex"
aprende "por onde começo a teoria" "pela fundação, não pelo teto: corpus/docs/torre_fundacao.tex depois teoria.tex"
aprende "o que vem antes do peano" "quatro fundações — corpos, dinâmica, topologia, Hilbert/Clifford. corpus/docs/torre_fundacao.tex"

# ========== Banda F1 — corpos (o que pode ser composto) ==========
aprende "o que é um corpo" "sítio com + e × onde os não-nulos invertem. Fundamento: teoria.tex e corpus/docs/torre_fundacao.tex"
aprende "o que é um corpo em algebra" "operações, inversores, extensão e redução. Ver teoria.tex (álgebra) e corpus/docs/torre_fundacao.tex"
aprende "o que é o corpo dual" "par (V,V*) com a identificação J — leis no corpo dual, não só num lado. teoria.tex"
aprende "fala do corpo dual" "ambiente (V,V*,J); os anti-autoadjuntos formam corpo algébrico. teoria.tex"
aprende "o que é inversor" "mapa a↦−a ou a↦a^{-1} (a≠0) — o que se inverte no corpo. corpus/docs/torre_fundacao.tex"
aprende "o que é extensão de corpos" "passar de K a L⊃K sem inventar axiomas novos. corpus/docs/torre_fundacao.tex"
aprende "o que é hurwitz" "só 4 álgebras de divisão normadas sobre R: R,C,H,O — N(xy)=N(x)N(y). tests/hurwitz.c e teoria.tex"
aprende "torre de hurwitz" "subindo R→C→H→O perde propriedades; descendo recupera. tests/hurwitz.c"
aprende "o que é a familia metalica" "borda x²−nx−1=0 — cada grau um corpo. teoria.tex §família metálica"
aprende "o que é a borda em algebra" "unidade do grau: x=n+1/x dá a borda. teoria.tex"
aprende "mostra a teoria" "formalização: teoria.tex — fundação primeiro: corpus/docs/torre_fundacao.tex"
aprende "explica corpos em uma frase" "o que pode ser composto com + e × e inverso. corpus/docs/torre_fundacao.tex"

# ========== Banda F2 — dinâmica (como evolui) ==========
aprende "o que é dinamica de sistemas" "X_{k+1}=F(X_k): órbitas, estados, volta. corpus/docs/torre_fundacao.tex"
aprende "o que é uma orbita" "a sequência {X_0,X_1,…} sob F. corpus/docs/torre_fundacao.tex"
aprende "o que é reversibilidade" "existência de volta F^{-1} ou involução — a volta pode fechar. teoria.tex / corpus/docs/torre_fundacao.tex"
aprende "o que é lyapunov" "taxas de expansão/contração; dualizado λ++λ−=0 na volta. corpus/docs/torre_fundacao.tex"
aprende "o que é estabilidade" "órbitas que permanecem perto. corpus/docs/torre_fundacao.tex"
aprende "o que é o relogio na teoria" "dinâmica dos pontos fixos — marca o tempo. teoria.tex §dinâmica do relógio"
aprende "como o corpo se move" "pela dinâmica F: estado→estado; a régua é de cada corpo. teoria.tex"
aprende "explica dinamica em uma frase" "como o estado evolui no tempo. corpus/docs/torre_fundacao.tex"

# ========== Banda F3 — topologia (onde ir sem perder continuidade) ==========
aprende "o que é topologia aqui" "vizinhança, continuidade, borda, retração. corpus/docs/torre_fundacao.tex"
aprende "o que é uma vizinhanca" "o que ainda conta como perto de x — N(x). corpus/docs/torre_fundacao.tex"
aprende "o que é a borda" "interface entre dentro e fora — ∂B. corpus/docs/torre_fundacao.tex e teoria.tex"
aprende "o que é retracao" "π∘ι=id — descer sem inventar. corpus/docs/torre_fundacao.tex"
aprende "o que é a topologia dos corpos" "distância entre réguas: d=|Δ1−Δ2|, Δ=B²−4C. tests/topologia.c"
aprende "como se mede distancia entre corpos" "pela assinatura Δ das réguas — zero nos isomorfos. tests/topologia.c"
aprende "o que é transporte entre corpos" "cisalhamento que preserva Δ — palavra na ISA. tests/topologia.c"
aprende "explica topologia em uma frase" "onde podes ir sem perder continuidade. corpus/docs/torre_fundacao.tex"

# ========== Banda F4 — Hilbert / Clifford (medir e orientar) ==========
aprende "o que é espaco de hilbert" "produto interno ⟨x,y⟩ e norma |x| — medir o passo. corpus/docs/torre_fundacao.tex"
aprende "o que é clifford" "álgebra com xy+yx=2⟨x,y⟩ — orientar, reflectir, inverter. corpus/docs/torre_fundacao.tex"
aprende "o que é a curva de hilbert aqui" "deriva das Leis 1 e 2; bidual π e ν. tests/hilbert_bidual.c"
aprende "o que é norma" "comprimento |x|; em Hurwitz N(xy)=N(x)N(y). tests/hurwitz.c"
aprende "o que é dualidade" "V e V* — formas lineares; no corpo dual com J. teoria.tex"
aprende "o que é reflexao" "volta métrica — espelho. corpus/docs/torre_fundacao.tex"
aprende "explica hilbert em uma frase" "como medir o movimento. corpus/docs/torre_fundacao.tex"
aprende "falemos de hurwitz" "Hurwitz: só R,C,H,O com N(xy)=N(x)N(y). tests/hurwitz.c e teoria.tex"
aprende "falemos de corpo" "corpo: + e × com inverso. teoria.tex e corpus/docs/torre_fundacao.tex"
aprende "falemos de topologia" "vizinhança e borda; nos corpos d=|Δ1−Δ2|. tests/topologia.c"
aprende "falemos de clifford" "orientar o movimento: xy+yx=2⟨x,y⟩. corpus/docs/torre_fundacao.tex"
aprende "falemos de hilbert" "produto interno e norma; curva: tests/hilbert_bidual.c"
aprende "falemos de dinamica" "X_{k+1}=F(X_k) — órbitas e volta. corpus/docs/torre_fundacao.tex"

# ========== Banda F1b — corpos (mais degraus) ==========
aprende "o que é o espectro dos corpos" "o que distingue um corpo de outro — régua e assinatura. teoria.tex §espectro"
aprende "a regua transporta?" "a régua não transporta; a volta transporta. teoria.tex §transporte"
aprende "o que é o corpo trial" "três estados do relógio — o dual deriva do trial. teoria.tex"
aprende "o que é a lei 1" "1†=−1 — dual/involução, período 2. teoria.tex e tests/hilbert_bidual.c"
aprende "o que é a lei 2" "T†=−T logo T²=−1 — rotor, período 4. teoria.tex e tests/hilbert_bidual.c"
aprende "o que é cayley dickson" "dobra que sobe R→C→H→O na torre de Hurwitz. tests/hurwitz.c"
aprende "porque hurwitz para em 8" "em 16 já há divisores de zero — o cristal N(xy)=N(x)N(y) parte. tests/hurwitz.c"
aprende "o que pode ser composto" "pergunta da fundação Corpos — corpus/docs/torre_fundacao.tex"

# ========== Banda F2b — dinâmica ==========
aprende "o que é expansao e contracao" "λ+ cresce, λ− encolhe; na volta dual λ++λ−=0. corpus/docs/torre_fundacao.tex"
aprende "o que é memoria na dinamica" "o estado seguinte depende do caminho, não só da posição. corpus/docs/torre_fundacao.tex"
aprende "o que é um ponto fixo" "X=F(X) — o relógio marca aí. teoria.tex §dinâmica do relógio"
aprende "como isso evolui" "pergunta da fundação Dinâmica — X_{k+1}=F(X_k). corpus/docs/torre_fundacao.tex"
aprende "o que é uma volta fechada" "órbita que regressa; o fecho métrico lê λ++λ−=0. corpus/docs/torre_fundacao.tex"
aprende "falemos de lyapunov" "taxas de expansão/contração; dualizado na volta. corpus/docs/torre_fundacao.tex"
aprende "falemos de orbita" "sequência de estados sob F. corpus/docs/torre_fundacao.tex"

# ========== Banda F3b — topologia ==========
aprende "o que é continuidade" "F leva vizinhos a vizinhos. corpus/docs/torre_fundacao.tex"
aprende "o que é projecao" "mapa que esquece; retração quando π∘ι=id. corpus/docs/torre_fundacao.tex"
aprende "o que é identificacao" "colar pontos sem destruir a estrutura local. corpus/docs/torre_fundacao.tex"
aprende "onde isso pode ir" "pergunta Topologia — vizinhança sem perder continuidade. corpus/docs/torre_fundacao.tex"
aprende "o que é delta da regua" "Δ=B²−4C — assinatura invariante ao transporte. tests/topologia.c"
aprende "dois corpos sao isomorfos quando" "quando d=|Δ1−Δ2|=0 — mesma classe. tests/topologia.c"
aprende "falemos de borda" "interface dentro/fora; na álgebra, unidade do grau. teoria.tex"
aprende "falemos de vizinhanca" "N(x) — o que ainda é perto. corpus/docs/torre_fundacao.tex"

# ========== Banda F4b — Hilbert/Clifford ==========
aprende "o que é produto interno" "⟨x,y⟩ — mede ângulo e comprimento. corpus/docs/torre_fundacao.tex"
aprende "o que é conservacao da norma" "N(xy)=N(x)N(y) — cristal de Hurwitz. tests/hurwitz.c"
aprende "o que é orientacao" "sentido do MOVE — Clifford ajuda a fixá-lo. corpus/docs/torre_fundacao.tex"
aprende "como medir e orientar" "pergunta Hilbert/Clifford — corpus/docs/torre_fundacao.tex"
aprende "o que é o bidual de hilbert" "π estica e ν contrai; ν∘π=id e π∘ν=id. tests/hilbert_bidual.c"
aprende "falemos de norma" "comprimento; em Hurwitz multiplica. tests/hurwitz.c"
aprende "falemos de reflexao" "volta métrica — espelho. corpus/docs/torre_fundacao.tex"

# ========== Banda F5 — ponte ==========
aprende "o que é gentil e hurwitz" "contar (Hurwitz) ↔ integrar (Gentil) pela medida — teoria.tex; depois da fundação"
aprende "quando falamos do peano" "depois das quatro fundações. corpus/docs/torre_fundacao.tex → papers/corpo_topologico.tex"
aprende "o que ainda nao se fala" "detalhe fino de Maestro/histerese — sobe o andar com calma. corpus/docs/torre_fundacao.tex"
aprende "o que é o teorema central" "Gentil↔Hurwitz pela medida — teoria.tex; só depois da escada"
aprende "posso falar do maestro ja" "sim, em voz baixa: projecta π_k. Fundações primeiro. papers/corpo_topologico.tex"

# ========== Banda F6 — andar Peano (leve; sobe a torre) ==========
aprende "o que é o corpo de peano" "torre com retrações π_k e d_{k+1}=2d_k — depois das fundações. papers/corpo_topologico.tex"
aprende "o que é a retracao pi_k" "π_k:A_{k+1}→A_k com π_k∘ι_k=id — desce um andar. papers/corpo_topologico.tex"
aprende "o que faz o maestro" "projecta: P_k=tick∘batuta∘Π realiza π_k. Não compõe o corpo. papers/corpo_topologico.tex"
aprende "o que faz o metronomo" "lê a volta: λ++λ−=0 atesta o fecho. papers/corpo_topologico.tex"
aprende "como sobe a torre peano" "A0←π0 A1←π1 … com d a dobrar. papers/corpo_topologico.tex"
aprende "mostra o corpo de peano" "papers/corpo_topologico.tex — depois de corpus/docs/torre_fundacao.tex"
aprende "falemos de peano" "andar da torre: retração, Maestro, Metrónomo. papers/corpo_topologico.tex"
aprende "falemos do maestro" "projecta o suporte; não inventa estrutura. papers/corpo_topologico.tex"
aprende "falemos do metronomo" "atesta a volta (resíduo). papers/corpo_topologico.tex"
aprende "o que e histerese aqui" "memória da borda sob a batuta I∈{-1,0,+1} — sem Lei 9. papers/corpo_topologico.tex §histerese"
aprende "o que e controle de histerese" "seleccionar variantes em V_k sem destruir o estado. papers/corpo_topologico.tex §controle-histerese"
aprende "o que e a rede dual" "P·ℋ·H: estaca W=-I, banda, Hopfield=memória Y; λ⁻ da conjugação. papers/corpo_topologico.tex §rede-dual"
aprende "o que e a conjugacao reversivel" "F_H=D∘F_P⁻¹∘D⁻¹; Hopfield≠inversão. papers/corpo_topologico.tex §rede-dual"
aprende "o que e o estado hibrido" "X=(x,h); F:(x,h)↦(x′,h′). papers/corpo_topologico.tex §rede-dual"
aprende "o que e retencao neural" "|u|≤Δ ⇒ h′=h; sequência na banda conserva h. papers/corpo_topologico.tex §rede-dual"
aprende "hopfield e a inversa?" "Não. Hopfield=memória da volta; λ⁻ da conjugação. papers/corpo_topologico.tex §rede-dual"
aprende "mostra a rede dual" "papers/corpo_topologico.tex §rede-dual — medidor tests/rede_dual.js"
aprende "falemos da rede dual" "P·ℋ·H: três operadores; conjugação fecha λ⁺+λ⁻=0. papers/corpo_topologico.tex §rede-dual"
aprende "a conjugacao fechou no experimento?" "Sim no modelo afim: DF_H DF_P=I e λ_P+λ_H=0 (tests/rede_dual.js §R5–§R7). Hopfield=memória Y."
aprende "a rede dual e o teorema central?" "Não: rede=controlo sob Maestro/Metrónomo. Central→Maestro→controlo→rede. papers/corpo_topologico.tex"
aprende "o que e o teorema central no peano" "Hurwitz↔Gentil pela soma reversível (estrela). Na torre: π_k → Maestro/Metrónomo → controlo. papers/corpo_topologico.tex"
aprende "como o teorema central sobe a torre" "π_k → Maestro/Metrónomo (λ⁺+λ⁻=0) → rede dual P·ℋ·H → Conservatório/Pera/Batuta. papers/corpo_topologico.tex"
aprende "o que e o conservatorio no peano" "estrutura superior conservada (Gentil, cone) — não a orquestra. papers/corpo_topologico.tex §musica"
aprende "o que e a pera na batuta" "base da batuta: cone em s=0; encaixe Conservatório↪Batuta. Não é I. papers/corpo_topologico.tex"
aprende "como a rede dual entra na musica" "Batuta canaliza I → ℱ(x,h) → Metrónomo atesta λ sob Π. papers/corpo_topologico.tex"
aprende "o que e a partitura pi" "Π=assinatura+notação: o quê do Maestro. Transporte, não lei. papers/corpo_topologico.tex; corpus/docs/partitura.tex"
aprende "como a partitura entra na assistente" "partituraFala→Π; I da batuta; ℱ escolhe ramo; Metrónomo lê r. app/src/maestro.js"
aprende "mostra a partitura" "corpus/docs/partitura.tex — partitura-wasm.pdf"

# ========== Ingestão estrutural ==========
echo "ingere teoria.tex…"
python3 "$D/ingere.py" "$ROOT/teoria.tex" | "$CV" "$B" - >/dev/null
echo "ingere torre_fundacao.tex…"
python3 "$D/ingere.py" "$ROOT/corpus/docs/torre_fundacao.tex" | "$CV" "$B" - >/dev/null
echo "ingere conversa_fundacao.tex…"
python3 "$D/ingere.py" "$ROOT/corpus/fala/conversa_fundacao.tex" | "$CV" "$B" - >/dev/null || true
echo "ingere conversa_andar_peano.tex…"
python3 "$D/ingere.py" "$ROOT/corpus/fala/conversa_andar_peano.tex" | "$CV" "$B" - >/dev/null || true
echo "ingere conversa_rede_dual.tex…"
python3 "$D/ingere.py" "$ROOT/corpus/fala/conversa_rede_dual.tex" | "$CV" "$B" - >/dev/null || true
echo "ingere conversa_arvore_n1_rede_dual.tex…"
python3 "$D/ingere.py" "$ROOT/corpus/fala/conversa_arvore_n1_rede_dual.tex" | "$CV" "$B" - >/dev/null || true
# corpo_topologico: só secções leves via paper conversa; não ingerir o paper inteiro ainda

echo "fundação+andar: base=$B"
for q in "o que é um corpo" "o que é a lei 1" "o que faz o maestro" "o que e histerese aqui" "o que e a rede dual" "mostra a fundação"; do
  echo "Q: $q"
  "$CV" "$B" responde "$q" | head -1
done

if [ "${ARVORE:-0}" = "1" ]; then
  echo "=== árvore ollama — sobe a torre ==="
  SYS="$ROOT/tools/.gabarito_sistema.txt"
  BAK="$ROOT/tools/.gabarito_sistema.bak"
  cp "$SYS" "$BAK"
  cp "$ROOT/tools/.gabarito_fundacao.txt" "$SYS"
  SEM1="corpo orbita vizinhanca norma hurwitz dualidade borda retracao lyapunov"
  SEM2="espectro transporte trial lei_1 lei_2 cayley gentil continuidade projecao produto_interno conservacao_norma"
  SEM3="peano retracao_pi maestro metronomo histerese controle_histerese torre_peano andar rede_dual conjugacao"
  SEMENTES="$SEM1" SUFIXO=_fundacao PROF=1 NIVEL=3 "$D/gabarito_arvore.sh" "$B" || true
  SEMENTES="$SEM2" SUFIXO=_fundacao2 PROF=1 NIVEL=1 "$D/gabarito_arvore.sh" "$B" || true
  SEMENTES="$SEM2" SUFIXO=_fundacao2 PROF=1 NIVEL=2 "$D/gabarito_arvore.sh" "$B" || true
  SEMENTES="$SEM2" SUFIXO=_fundacao2 PROF=1 NIVEL=3 "$D/gabarito_arvore.sh" "$B" || true
  # prompt peano leve
  cat > "$SYS" <<'EOF'
És assistente na escada Tiffany. Português BR, 1–3 frases completas com pontuação.
Já tens fundações. Podes falar do andar Peano com calma: π_k, Maestro (projecta), Metrónomo (lê), histerese (borda+I).
Rede dual: P·ℋ·H; Hopfield=memória Y; λ⁻ da conjugação F_H=D∘F_P⁻¹∘D⁻¹ (não do atrator).
Cita: corpus/docs/torre_fundacao.tex, papers/corpo_topologico.tex, corpus/fala/conversa_rede_dual.tex, teoria.tex, tests/hurwitz.c, tests/topologia.c, tests/hilbert_bidual.c, tests/rede_dual.js.
Lei 1/2 = álgebra Tiffany (involução/rotor), não leis do Brasil. Sem Lei 9. Sem emoji.
Não inventes teoremas de grafos para Hurwitz nem livros fictícios.
EOF
  SEMENTES="$SEM3" SUFIXO=_peano PROF=1 NIVEL=1 "$D/gabarito_arvore.sh" "$B" || true
  SEMENTES="$SEM3" SUFIXO=_peano PROF=1 NIVEL=2 "$D/gabarito_arvore.sh" "$B" || true
  mv "$BAK" "$SYS"
  for tex in \
    "$ROOT/corpus/fala/conversa_arvore_n3_fundacao.tex" \
    "$ROOT/corpus/fala/conversa_arvore_n1_fundacao2.tex" \
    "$ROOT/corpus/fala/conversa_arvore_n2_fundacao2.tex" \
    "$ROOT/corpus/fala/conversa_arvore_n3_fundacao2.tex" \
    "$ROOT/corpus/fala/conversa_arvore_n1_peano.tex" \
    "$ROOT/corpus/fala/conversa_arvore_n2_peano.tex"
  do
    [ -f "$tex" ] && python3 "$D/ingere.py" "$tex" | "$CV" "$B" - >/dev/null || true
  done
  "$D/limpa_secoes_corpus.sh" "$B" \
    "$ROOT"/corpus/fala/conversa_arvore_n3_fundacao.tex \
    "$ROOT"/corpus/fala/conversa_arvore_n*_fundacao2.tex \
    "$ROOT"/corpus/fala/conversa_arvore_n*_peano.tex 2>/dev/null || true
  ARVORE=0 "$D/fundacao.sh" "$B" || true
fi
