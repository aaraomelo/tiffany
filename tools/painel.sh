#!/bin/bash
# painel.sh — O PAINEL DO PILOTO: o estado numa tela, e o corpo que se fecha sozinho.
#
# O Aarão: "faz o painel também, paralelo ao manual. O painel conecta nos hooks e as operações
# são via contrato." E logo a seguir, a simplificação: "não há necessidade de contrato — fecha
# quando o corpo completa."
#
# PARALELO AO MANUAL, e a palavra é exata: o PILOTO.md diz o que as coisas são, o painel diz em
# que estado elas estão AGORA. Um não substitui o outro, e nenhum dos dois inventa — o painel
# lê, não decide.
#
# E O CONTRATO NÃO SE ASSINA: LIQUIDA-SE. O Aarão corrigiu duas vezes — primeiro "não há
# necessidade de contrato, fecha quando o corpo completa", e depois "pronto, smart contracts: é um
# contrato inteligente, chama agentes". O que morre é a ASSINATURA, não o contrato. O painel não
# pede quatro cláusulas assinadas; pede QUATRO NÚMEROS, e o resto deriva-se e EXECUTA-SE:
#
#     o piloto dá        alguns termos — o lado branco da torre
#     sai a RÉGUA        (B, C), por Cramer, exata em inteiros
#     sai o lado NEGRO   ν(a,b) = (a + B·b, −b), forçado
#     FECHOU             quando a reversão volta com resíduo 0
#     e CHAMA O AGENTE   que o Δ determina — gira, estica ou o limite
#
# Um corpo não promete fechar: ou fecha, ou os termos não eram de um corpo. Não há o que assinar,
# e é por isso que este painel tem um verbo a menos do que tinha.
#
# E CONECTA NOS HOOKS: a secção 4 do `session-start.sh` injeta o índice cifrado da memória. O
# painel mostra o mesmo índice e o estado do hook, para o piloto ver o que a assistente vê.
#
# Sem rede, sem ollama, sem RAM: tudo sai de ficheiros.
#
# OPERA-SE 100% DAQUI. O Aarão: "vê o que falta pra operar 100% via o painel; move tudo para os
# plugues do painel e alinha com o manual". Não há verbo do sistema que só exista fora:
#
#   o CORPO      fecha  polar                    dê os termos, e ele diz-se
#   os PLUGUES   asm bash git ssh nginx kernel sql tex   um por linha, todos com medidor
#   o ESTADO     hook  apps  memoria  bateria    o que está montado agora
#   os TERMINAIS terminais                       dois para fora, e a polaridade medida
#   o SERVIDOR   patria                          o disco, a memória e o fork — e SAI da máquina
#   TUDO         tudo                            corre os plugues todos, um a um
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
RAIZ="$(cd "$CD/.." && pwd)"
BANCO=${BANCO:-/tmp/painel_banco.dat}
Q=${Q:-12}
HOOK="$HOME/.claude/hooks/session-start.sh"

az(){ printf '\033[1m%s\033[0m\n' "$*"; }
linha(){ printf '  %-30s %s\n' "$1" "$2"; }

# ---------------------------------------------------------------- as operações, derivadas
# Não são declaradas: saem da régua, que sai dos termos. O painel não guarda uma lista.
clausula(){
  local nome=$1 a=$2 b=$3
  case "$nome" in
    SOMA)     echo $(( (a + b) % Q )) ;;                      # ⊕ Clifford — Kirchhoff
    PRODUTO)  echo $(( (a * b) % Q )) ;;                      # ⊗ La Hire — o ganho
    OPERADOR) python3 -c "print(pow($a or 1, $b or 1, $Q))" ;;# ∏ Pontryagin — exp∘Σ∘log
    DUAL)     echo $(( ((-a) % Q + Q) % Q )) ;;               # ν — o espelho, ordem 2
    *)        echo "ERRO" ;;
  esac
}

modo=${1:-estado}

case "$modo" in
fecha)
  # O VERBO PRINCIPAL DO PAINEL. O piloto dá os termos; o corpo diz-se inteiro ou recusa.
  shift
  [ -x /tmp/fecha ] || cc -O2 -std=c99 "$CD/fecha.c" -lm -o /tmp/fecha 2>/dev/null
  if [ $# -lt 4 ]; then
    az "FECHAR UM CORPO — dê pelo menos 4 termos (n+2 para grau 2)"
    echo "    ./painel.sh fecha 0 1 1 2 3 5      → ouro,  Δ = 5,  hiperbólico"
    echo "    ./painel.sh fecha 1 0 -1 0 1 0     → i,     Δ = −4, elíptico"
    echo "    ./painel.sh fecha 0 1 2 5 12 29    → prata"
    echo
    echo "  Não se declara nada: régua, borda, soma, produto, dual e Δ saem dos termos,"
    echo "  e a reversão verifica-se sozinha. Cabe ao piloto apenas decidir que termos dar."
    exit 0
  fi
  /tmp/fecha "$@"
  ;;

polar|cartesiana|forma)
  # AS DUAS FORMAS, e o painel usa as duas porque o piloto usa as duas.
  [ -x /tmp/polar ] || cc -O2 -std=c99 "$CD/polar.c" -lm -o /tmp/polar 2>/dev/null
  shift
  if [ $# -lt 4 ]; then
    az "AS DUAS FORMAS — dê a régua (B C) e o ponto (a b)"
    echo "    ./painel.sh polar  1 -1  3 2      o ouro, no ponto 3 + 2σ"
    echo "    ./painel.sh polar  0  1  1 1      o i, no ponto 1 + i"
    echo
    echo "  ALGÉBRICA  z = a + b·σ      soma bem   — é o produto DIRETO (mede)"
    echo "  POLAR      z = ρ·E(θ)       multiplica bem — é o CRUZADO (ordena)"
    echo "  e o espelho troca só o cruzado: a peça que mede é a mesma dos dois lados."
    exit 0
  fi
  /tmp/polar "$@"
  ;;

asm)
  shift; exec "$CD/plugue.sh" "${@:-ajuda}" ;;

git)
  # O GIT é o nosso banco: o endereço calcula-se do conteúdo e não se atribui.
  [ -x /tmp/gitb ] || cc -O2 -std=c99 "$CD/gitb.c" -o /tmp/gitb 2>/dev/null
  shift
  ( cd "$RAIZ" && /tmp/gitb "$@" )
  ;;

interroga|saber)
  # PERGUNTA TUDO O QUE ELE SABE, e roda o contrato sobre as respostas.
  [ -x /tmp/liquida_doador ] || cc -O2 -std=c99 "$CD/liquida_doador.c" -lm -o /tmp/liquida_doador 2>/dev/null
  [ -s /tmp/saber_pares.txt ] || "$CD/interroga.sh" || exit 2
  /tmp/liquida_doador
  ;;

dualcifra)
  # A DUALIDADE DA CIFRA no espaço semântico: ele soma, nós multiplicamos.
  [ -x /tmp/dualcifra ] || cc -O2 -std=c99 "$CD/dualcifra.c" -lm -o /tmp/dualcifra 2>/dev/null
  [ -s /tmp/frases.txt ] || "$CD/colhe_dualcifra.sh" || exit 2
  /tmp/dualcifra
  ;;

transfusao-real)
  # A TRANSFUSÃO REAL: precisa do doador ACORDADO (ollama a correr).
  [ -x /tmp/transfusao_real ] || cc -O2 -std=c99 -D_GNU_SOURCE "$CD/transfusao_real.c" -lm -o /tmp/transfusao_real 2>/dev/null
  [ -s /tmp/vetores.txt ] || "$CD/colhe_transfusao.sh" || exit 2
  /tmp/transfusao_real
  ;;

transfusao|transfusão)
  # A TRANSFUSÃO: não se hospeda o doador, colhe-se o corpo. E o resto vem da dualidade.
  [ -x /tmp/transfusao ] || cc -O2 -std=c99 "$CD/transfusao.c" -lm -o /tmp/transfusao 2>/dev/null
  /tmp/transfusao
  ;;

kernel)
  # O KERNEL: a syscall É a ISA do SO, o fd é o slot, e o VFS casa por prefixo mais longo.
  [ -x /tmp/kernelb ] || cc -O2 -std=c99 "$CD/kernelb.c" -o /tmp/kernelb 2>/dev/null
  /tmp/kernelb
  ;;

nginx)
  # O NGINX: a config é família 2, mas a REGRA DE CASAMENTO é o trie — prefixo mais longo.
  [ -x /tmp/nginxb ] || cc -O2 -std=c99 "$CD/nginxb.c" -o /tmp/nginxb 2>/dev/null
  shift
  ( cd "$CD" && /tmp/nginxb "$@" )
  ;;

patria)
  # O SERVIDOR. Isto SAI DA MÁQUINA — é a única coisa neste painel que precisa de rede, e
  # falhar aqui não é falhar o sistema: é não haver resposta.
  az "A PATRIA — o que temos lá"
  if ! timeout 25 ssh -o ConnectTimeout=15 -o BatchMode=yes patria true 2>/dev/null; then
    echo "  sem resposta do servidor (rede, ou a chave não está carregada)."
    echo "  Isto não é uma falha do sistema: é não haver resposta."
    exit 0
  fi
  # comandos simples lá, formatação AQUI — o escape de awk dentro de aspas dentro de ssh
  # comia-se a si próprio e o bloco saía vazio, o que é pior do que sair errado.
  dados=$(timeout 40 ssh -o ConnectTimeout=15 -o BatchMode=yes patria \
    'df -h / | tail -1; free -h | grep ^Mem:; nproc;
     du -sh /var/www/goldenkingdom 2>/dev/null | cut -f1;
     du -sh /root/tiffany-repo.git 2>/dev/null | cut -f1;
     du -sh /var/www/goldenkingdom/repo.git 2>/dev/null | cut -f1;
     git --git-dir=/var/www/goldenkingdom/repo.git rev-list --count HEAD 2>/dev/null' 2>/dev/null)
  set -- $(echo "$dados" | sed -n 1p)          # df:  fs size used avail use% mount
  linha "disco" "$4 livres de $2  ($5 usado)"
  set -- $(echo "$dados" | sed -n 2p)          # free: Mem: total used free shared buff avail
  linha "memória" "$7 disponíveis de $2"
  linha "cpus"            "$(echo "$dados" | sed -n 3p)"
  linha "o nosso site"    "$(echo "$dados" | sed -n 4p)"
  linha "o repo bare"     "$(echo "$dados" | sed -n 5p)"
  linha "o fork servido"  "$(echo "$dados" | sed -n 6p)"
  linha "commits no fork" "$(echo "$dados" | sed -n 7p)"
  echo
  echo "  a config servida está versionada em  tools/nginx/goldenkingdom.conf"
  echo "  e medida por  ./painel.sh nginx  — inclusive as três peças que impedem o clone de partir."
  ;;

ssh)
  # O SSH acoplado, e as voltas contadas contra o bump da nossa banda.
  [ -x /tmp/sshb ] || cc -O2 -std=c99 "$CD/sshb.c" -lm -o /tmp/sshb 2>/dev/null
  ( cd "$CD" && /tmp/sshb )
  ;;

sql)
  # SQL NO METAL: compila para a MESMA ISA, e a memória é o disco.
  [ -x /tmp/sqlb ] || cc -O2 -std=c99 "$CD/sql.c" -lm -o /tmp/sqlb 2>/dev/null
  shift
  if [ $# -eq 0 ]; then
    az "SQL NO METAL — a mesma ISA, e o disco é a memória"
    echo "    ./painel.sh sql /tmp/base \"CREATE TABLE t (a,b,c)\""
    echo "    ./painel.sh sql /tmp/base \"INSERT INTO t VALUES (7,8,9)\""
    echo "    ./painel.sh sql /tmp/base \"SELECT * FROM t WHERE a = 7\""
    exit 0
  fi
  /tmp/sqlb "$@"
  ;;

tex)
  # LaTeX → PDF sem dependência nenhuma, e a largura vem da CURVA.
  [ -x /tmp/texb ] || cc -O2 -std=c99 "$CD/tex.c" -lm -o /tmp/texb 2>/dev/null
  shift
  if [ $# -eq 0 ]; then
    az "O COMPILADOR DE .tex — sem pdflatex, sem dependência"
    echo "    ./painel.sh tex documento.tex saida.pdf"
    exit 0
  fi
  /tmp/texb "$@"
  ;;

memoria)
  shift
  exec "$CD/memoria_banco.sh" "${@:-ingere}" ;;

terminais)
  # OS TERMINAIS: dois para fora, com polaridade, e σσ' = −1 a conservar.
  az "OS TERMINAIS — dois para fora, e o resto dentro"
  echo
  linha "quantos" "2 — e não é escolha: a cifra dual dá o par"
  linha "a polaridade" "as duas raízes têm SINAIS OPOSTOS"
  linha "o que se conserva" "σ·σ' = −1  (o ganho de um é a perda do outro)"
  linha "ler e escrever" "a MESMA operação, o sentido trocado"
  echo
  az "A ALIMENTAÇÃO (por indução dual, pela liga)"
  linha "Seebeck com o céu" "993 mW      (arraytermico.c)"
  linha "RF ambiente, isotrópica" "21 µW       (colheita.c)"
  linha "e a conta que decide" "guardar é quase grátis; calcular não é"
  echo
  echo "  medido em  tools/plugs.c §P7  ·  tools/dispositivo.c  ·  tools/colheita.c"
  echo "  para correr:  ./painel.sh tudo   (inclui os três)"
  ;;

tudo)
  # CORRE OS PLUGUES TODOS, um a um. É o painel a provar-se — e lê-se o TOTAL, não a linha
  # das unidades: um medidor que não compila não falha, desaparece.
  az "OS PLUGUES, CORRIDOS UM A UM"
  echo
  printf '  %-16s %-11s %s\n' plugue estado veredito
  vivos=0; mortos=0
  for m in erg fecha polar smartcontract transfusao transfusao_real dualcifra liquida_doador gitb sshb nginxb kernelb chessb dominios prisma dispositivo plugs; do
    [ -f "$CD/$m.c" ] || { printf '  %-16s %-11s %s\n' "$m" "AUSENTE" "—"; mortos=$((mortos+1)); continue; }
    if ! cc -O2 -std=c99 "$CD/$m.c" -lm -o "/tmp/pn_$m" 2>/dev/null; then
      printf '  %-16s %-11s %s\n' "$m" "NAO COMPILA" "e isso não é falhar: é desaparecer"
      mortos=$((mortos+1)); continue
    fi
    saida=$( cd "$CD" && ulimit -v 1000000; timeout 300 "/tmp/pn_$m" 2>&1 )
    rc=$?
    # exit 2 = NÃO MEDIU: falta o objeto a medir, e isso NÃO é passar. Fica à vista, com a
    # razão e o remédio, e conta em "por resolver" — nunca numa lista de isentos.
    if [ "$rc" -eq 2 ]; then
      razao=$(printf '%s' "$saida" | grep -m1 -E 'NAO MEDIU|não mediu' | cut -c1-56)
      printf '  %-16s %-11s %s\n' "$m" "NAO MEDIU" "${razao:-falta o objeto a medir}"
      mortos=$((mortos+1)); continue
    fi
    v=$(printf '%s' "$saida" | grep -oE 'RESIDUO 0|RESÍDUO 0|resíduo 0|FALHOU|NAO FECHOU' | tail -1)
    u=$(printf '%s' "$saida" | grep -c '^#UNIT ok')
    f=$(printf '%s' "$saida" | grep -c '^#UNIT falha')
    if [ "${f:-0}" -eq 0 ] && [ -n "$v" ]; then
      printf '  %-16s %-11s %s\n' "$m" "VERDE" "$u unidade(s) — $v"; vivos=$((vivos+1))
    else
      printf '  %-16s %-11s %s\n' "$m" "FALHA" "$u ok, $f falha(s) — ${v:-SEM VEREDITO}"; mortos=$((mortos+1))
    fi
  done
  echo
  printf '  %d plugues vivos, %d por resolver\n' "$vivos" "$mortos"
  echo "  (a bateria inteira — 232 medidores — é  tools/bateria.sh)"
  ;;

op)
  nome=${2:?diga a operação: SOMA PRODUTO OPERADOR DUAL}
  a=${3:-0}; b=${4:-0}
  r=$(clausula "$nome" "$a" "$b")
  if [ "$r" = "ERRO" ]; then
    echo "  '$nome' não é uma das operações. São: SOMA PRODUTO OPERADOR DUAL —"
    echo "  e as quatro NÃO são declaradas: saem da régua. Para as ver derivadas de termos,"
    echo "  use  ./painel.sh fecha <termos>."
    exit 1
  fi
  echo "$r" >> "$BANCO"
  printf '  %s(%s, %s) = %s   sobre Z_%s   · o banco tem agora %s registos\n' \
         "$nome" "$a" "$b" "$r" "$Q" "$(wc -l < "$BANCO" 2>/dev/null || echo 0)"
  ;;

hook)
  az "O HOOK DE ENTRADA"
  if [ -f "$HOOK" ]; then
    linha "ficheiro" "$HOOK"
    linha "secções" "$(grep -c '^  echo "## ' "$HOOK" 2>/dev/null || echo '?')"
    if grep -q memoria_banco.sh "$HOOK" 2>/dev/null; then
      linha "a túnica" "LIGADA (secção 4 injeta o índice cifrado)"
    else
      linha "a túnica" "não ligada"
    fi
    if bash -n "$HOOK" 2>/dev/null; then linha "sintaxe" "ok"; else linha "sintaxe" "QUEBRADA"; fi
  else
    linha "ficheiro" "não existe — o painel corre na mesma"
  fi
  echo
  az "O QUE ELE INJETA (a memória cifrada)"
  if [ -s /tmp/memoria_banco.txt ]; then
    printf '  %s memórias, índice de %s bytes\n' \
           "$(wc -l < /tmp/memoria_banco.txt)" "$(wc -c < /tmp/memoria_banco.txt)"
    cut -f1,3 /tmp/memoria_banco.txt 2>/dev/null | head -6 | \
      awk -F'\t' '{printf "    %-38s %s\n", $1, $2}'
  else
    echo "  (nenhum índice em /tmp — corre  tools/memoria_banco.sh ingere)"
  fi
  ;;

apps)
  az "OS APPS DO PILOTO"
  if [ ! -d "$CD/apps" ]; then echo "  (ainda não há tools/apps/)"; exit 0; fi
  [ -x /tmp/erg ] || cc -O2 -std=c99 "$CD/erg.c" -o /tmp/erg 2>/dev/null
  printf '  %-24s %8s %8s   %s\n' "app" "linhas" "bytes" "monta?"
  for a in "$CD"/apps/*.erg; do
    [ -f "$a" ] || continue
    saida=$(/tmp/erg monta "$a" /tmp/painel_app.bin 2>&1)
    if echo "$saida" | grep -q bytes; then
      b=$(echo "$saida" | awk '{print $1}'); est="sim"
    else b="—"; est="NÃO: $saida"; fi
    printf '  %-24s %8s %8s   %s\n' "$(basename "$a")" "$(grep -cv '^\s*\(;.*\)\?$' "$a")" "$b" "$est"
  done
  ;;

bateria)
  az "A BATERIA"
  if [ -f /tmp/bateria_ultima.txt ]; then
    tail -4 /tmp/bateria_ultima.txt
  else
    echo "  (sem corrida registada — corre  tools/bateria.sh)"
    echo "  E LEIA O TOTAL, não a linha das unidades: um medidor que não compila não falha,"
    echo "  desaparece."
  fi
  ;;

*)
  az "PAINEL DO PILOTO — $(basename "$RAIZ")"
  echo
  az "1. A LIQUIDAÇÃO (o contrato não se assina: dê os termos, ele corre)"
  linha "o piloto dá" "alguns termos — 4 bastam"
  linha "sai a régua" "(B,C), por Cramer, exata em inteiros"
  linha "sai o dual" "ν(a,b) = (a + B·b, −b) — forçado"
  linha "e a soma e o produto" "também, da mesma régua"
  linha "FECHOU quando" "a reversão volta com resíduo 0"
  linha "e chama o agente" "que o Δ determina — gira, estica ou o limite"
  echo "  uso:  ./painel.sh fecha 0 1 1 2 3 5"
  echo
  az "1b. AS DUAS FORMAS (e o painel usa as duas)"
  linha "ALGÉBRICA  z = a + b·σ" "soma bem — é o produto DIRETO, e MEDE"
  linha "POLAR      z = ρ·E(θ)" "multiplica bem — é o CRUZADO, e ORDENA"
  linha "o regime" "Δ<0 gira · Δ>0 estica · Δ=0 o limite"
  linha "sob o espelho ν" "o direto FICA, o cruzado TROCA de sinal"
  echo "  uso:  ./painel.sh polar 1 -1 3 2"
  echo
  az "2. A MÁQUINA (a ISA ERG-64, sem RAM)"
  linha "opcodes expostos ao piloto" "$(grep -c '^    { "' "$CD/erg.c" 2>/dev/null || echo '?')"
  linha "registos" "A, B, R — e o pc"
  linha "memória" "um ficheiro, 16 bytes por slot, pread/pwrite"
  linha "as duas armadilhas" "STORE grava R · FL_ZERO é AMBOS zero"
  echo
  az "3. O BANCO DO PAINEL"
  if [ -f "$BANCO" ]; then
    linha "registos" "$(wc -l < "$BANCO")"
    linha "últimos" "$(tail -5 "$BANCO" | tr '\n' ' ')"
  else
    linha "registos" "0 (vazio — use  op  para escrever)"
  fi
  linha "o corpo" "Z_$Q — finito, logo todo percurso FECHA por gaiola"
  echo
  az "4. O HOOK DE ENTRADA"
  if [ -f "$HOOK" ] && grep -q memoria_banco.sh "$HOOK" 2>/dev/null; then
    linha "a túnica" "LIGADA — a assistente entra vestida"
  else
    linha "a túnica" "não ligada"
  fi
  if [ -s /tmp/memoria_banco.txt ]; then
    linha "índice cifrado" "$(wc -l < /tmp/memoria_banco.txt) memórias, $(wc -c < /tmp/memoria_banco.txt) bytes"
  else
    linha "índice cifrado" "ausente (tools/memoria_banco.sh ingere)"
  fi
  echo
  az "5. OS PLUGUES — e o verbo que os opera daqui"
  for p in "asm:erg.c:assembly ERG-64 — monta, corre, desmonta" \
           "bash:plugue.sh:os verbos do lado de dentro, inversíveis" \
           "git:gitb.c:o git JÁ é o nosso banco — o endereço é a cifra" \
           "ssh:sshb.c:o SSH acoplado, e as voltas contra o bump" \
           "nginx:nginxb.c:o location casa por prefixo mais longo — é o trie" \
           "kernel:kernelb.c:a syscall É a ISA do SO; o fd é o slot" \
           "transfusao:transfusao.c:colher o corpo, não hospedar o doador" \
           "transfusao-real:transfusao_real.c:o doador ACORDADO, e o que fecha de verdade" \
           "dualcifra:dualcifra.c:ele soma, nós multiplicamos — a cifra completada" \
           "interroga:liquida_doador.c:pergunta tudo, e o contrato roda sobre isso" \
           "sql:sql.c:SQL no metal — a mesma ISA, o disco é a memória" \
           "tex:tex.c:LaTeX → PDF, sem dependência nenhuma" \
           "memoria:memoria_banco.sh:a túnica — ler e escrever, adjuntos" \
           "—:dominios.c:PTX — a GPU escreve na mesma janela" \
           "—:chessb.c:WASM e Node — a pilha do wasm na nossa ISA" \
           "—:prisma.c:o corpo prismático — o triângulo que enche"; do
    v=${p%%:*}; r=${p#*:}; f=${r%%:*}; d=${r#*:}
    if [ -f "$CD/$f" ]; then printf '  %-10s %-20s %s\n' "$v" "$f" "$d"
    else printf '  %-10s %-20s %s  (ausente)\n' "$v" "$f" "$d"; fi
  done
  echo "  (o traço = tem medidor mas ainda não tem verbo próprio; corre em  ./painel.sh tudo)"
  echo
  echo
  echo "  o manual:  PILOTO.md"
  echo "  os verbos: fecha · polar · asm · bash · git · ssh · nginx · kernel · sql · tex"
  echo "             memoria · transfusao · terminais · patria · hook · apps · bateria · tudo"
  ;;
esac
