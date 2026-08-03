#!/usr/bin/env bash
# mutacao.sh — O TESTE QUE DECIDE SE UM MEDIDOR MEDE.
#
# Uma asserção verde não prova nada: prova-se estragando o código e vendo se ela ACUSA.
# Se a mutação sobrevive, aquele bloco não está coberto — e a leitura não mostra isso.
#
# Em 03/08 este script apanhou um buraco que quatro revisores humanos e uma leitura minha
# não viram: eu tinha REMOVIDO uma asserção vazia do palavra.c §P9 e, com ela, a única
# cobertura sobre o código da decifra. Mutei `K11*cp` para `K11*cp + 1` e a bateria inteira
# ficou verde. A lição: uma asserção pode ser vazia como AFIRMAÇÃO e continuar a ser o único
# teste de regressão de um bloco. Apagá-la abre um buraco invisível.
#
#   uso:  tools/mutacao.sh                 corre o conjunto de mutações conhecidas
#         tools/mutacao.sh <ficheiro.c> '<sed-expr>' '<descrição>'    uma mutação avulsa
#
# Interpretação:
#   "matada"     — bom. A mutação foi apanhada; aquele código está coberto.
#   "SOBREVIVEU" — buraco. Ou falta asserção, ou a mutação é EQUIVALENTE (o código
#                  corrige-a sozinho). Verificar QUAL antes de escrever asserção nova:
#                  em 03/08, `d = p/q` → `p/q+1` sobreviveu por ser equivalente — o
#                  `if(r<0){ d--; r+=q; }` logo abaixo neutraliza-a exatamente.
set -u
cd "$(dirname "$0")"   # os medidores compilam-se a partir de tools/
TMP="${TMPDIR:-/tmp}/mut.$$"
mkdir -p "$TMP"; trap 'rm -rf "$TMP" ./_mut.c' EXIT

uma(){ # ficheiro-base, sed, descrição
  sinaliza "$1" || return
  cp "$1" ./_mut.c
  sed -i "$2" ./_mut.c
  if diff -q "$1" ./_mut.c >/dev/null; then
      printf "   ?  sed nao bateu  — %s\n" "$3"; return; fi
  if ! cc -O2 -std=c99 -w ./_mut.c -lm -o "$TMP/m" 2>/dev/null; then
      printf "   ?  nao compila    — %s\n" "$3"; return; fi
  # O VEREDICTO SAI DO CÓDIGO DE SAÍDA, e não do rodapé impresso.
  #
  # A 1.ª versão deste script lia `grep -oP '\d+(?= falha\(s\))'` do stdout. Só 16 dos 557
  # medidores imprimem esse rodapé — e são exatamente os que eu tinha mexido no dia em que
  # escrevi o script. Nos outros 541 o grep não casava, `${n:-0}` valia 0, e o script
  # imprimia "SOBREVIVEU" a TODAS as mutações, apanhadas ou não.
  #
  # É o pior tipo de defeito numa ferramenta de auditoria: falso ALARME em 97% dos casos,
  # e a apontar buracos que não existem. Um revisor externo apanhou-o com um caso concreto
  # (hopfield.c, 3 asserções a falhar e o script a dizer SOBREVIVEU) antes de me reportar
  # um buraco inventado. Ver a nota da bateria: um medidor com direito a falhar não é medido.
  timeout 120 "$TMP/m" >/dev/null 2>&1; rc=$?
  if [ "$rc" = "0" ]; then printf "   XX SOBREVIVEU    — %s\n" "$3"
  elif [ "$rc" -ge 124 ]; then printf "   ?  timeout/crash  — %s\n" "$3"
  else                    printf "   ok matada (exit %s) — %s\n" "$rc" "$3"; fi
}

# E o pré-requisito, que o script agora verifica em vez de supor: o medidor tem de SINALIZAR
# a falha no código de saída. 256 dos 557 têm `return falhas ...`; nos outros, uma mutação
# nunca pode ser matada — não porque esteja coberta, mas porque o medidor não tem como dizer.
sinaliza(){
  if ! grep -q 'return falhas' "$1"; then
    printf "   !! %s NAO SINALIZA falha no exit — mutação aqui é inconclusiva\n" "$1"
    return 1
  fi
  return 0
}

if [ $# -ge 2 ]; then uma "$1" "$2" "${3:-avulsa}"; exit 0; fi

echo "=== palavra.c — o degrau e a cifra"
uma palavra.c 's|L n00 = m00\*a\[i\] + m01, n01 = m00;|L n00 = m00*a[i] + 2*m01, n01 = m00;|' 'A_a: m01 -> 2*m01'
uma palavra.c 's|L esp = (k % 2) ? -1 : 1;|L esp = 1;|'                                      'det: (-1)^k -> +1'
uma palavra.c 's|\*p1=m00; \*p2=m01; \*q1=m10; \*q2=m11;|*p1=m01; *p2=m00; *q1=m10; *q2=m11;|' 'convergentes trocados'
uma palavra.c 's|L dp = ( K11\*cp - K01\*cq) / detK;|L dp = ( K11*cp - K01*cq) / detK + 1;|'  'decifra: +1 (o buraco de 03/08)'
uma palavra.c 's|q\[k\] = ak\*q\[k-1\] + q\[k-2\];|q[k] = ak*q[k-1];|'                        'q_k perde o termo q_{k-2}'

echo "=== continua.c — a continuacao"
uma continua.c 's|L t\[16\]; t\[0\]=2; t\[1\]=m;|L t[16]; t[0]=2; t[1]=m+1;|'                 't_1 = m -> m+1'
uma continua.c 's|double fech = -log(1.0 - m\*x - x\*x);|double fech = -log(1.0 - m*x + x*x);|' 'forma fechada: -x^2 -> +x^2'

echo "=== cantor.c — o gerador"
uma cantor.c 's|return s\*(s+1)/2 + b;|return s*(s+1)/2 + a;|'                                'pi: +b -> +a'
uma cantor.c 's|return p\*(2\*b+1) - 1;|return p*(2*b+1);|'                                   'rho: -1 removido'
uma cantor.c 's|L nx = (x>=0) ? 2\*x : -2\*x-1;|L nx = (x>=0) ? 2*x+1 : -2*x-1;|'              'involucao Z->N estragada'

echo "=== gauss.c — Z[i]"
uma gauss.c 's|return (G){x.a\*y.a - x.b\*y.b, x.a\*y.b + x.b\*y.a};|return (G){x.a*y.a + x.b*y.b, x.a*y.b + x.b*y.a};|' 'produto: i^2 = +1'
uma gauss.c 's|L qa = (2\*t.a + (t.a>=0 ? n : -n)) / (2\*n);|L qa = t.a / n;|'                 'divisao: arredondar -> truncar'

echo "=== nomeia.c — a ferramenta"
uma nomeia.c 's|L x00=1,x01=0,x10=0,x11=1;|L x00=1,x01=1,x10=0,x11=1;|'                        'mob: identidade errada'

echo "=== xx.c — x^x = x^n e a serie"
uma xx.c 's|A\[1\] = (Q){1,1};|A[1] = (Q){2,1};|'                         'serie: A_1 = 1 -> 2'
uma xx.c 's|double w = (z > -0.3) ? 0.5 : -0.9;|double w = 0.5;|'            'Lambert: palpite fixo'
uma xx.c 's|if(mid\*log(mid) < log(n)) lo = mid; else hi = mid;|if(mid*log(mid) > log(n)) lo = mid; else hi = mid;|' 'bissecao invertida'

echo "=== aurea.c — f' = f^-1 e o ouro"
uma aurea.c 's|const double A    = pow(phi, 1.0 - phi);|const double A    = pow(phi, phi - 1.0);|' 'coeficiente invertido'
uma aurea.c 's|const double phi  = (1.0 + raiz5)/2.0;|const double phi  = (1.0 + raiz5)/2.1;|' 'phi errado por pouco'

echo "=== selberg.c — a zeta em Z[sigma]"
uma selberg.c 's|Zs inv = {-m,1};|Zs inv = {-m,2};|'                              'sigma^{-1} errado'
uma selberg.c 's|r.b = x.a\*y.b + x.b\*y.a + x.b\*y.b\*m;|r.b = x.a*y.b + x.b*y.a;|' 'produto: sem o termo sigma^2'

echo "=== lambert.c — cartesiana, polar, monodromia"
uma lambert.c 's|double complex e = cexp(w), f = w\*e - z;|double complex e = cexp(w), f = w*e + z;|' 'W: sinal do residuo'
uma lambert.c 's|double re = exp(u)\*(u\*cos(v) - v\*sin(v));|double re = exp(u)*(u*cos(v) + v*sin(v));|' 'cartesiana: sinal trocado'
uma lambert.c 's|double m = cabs(w)\*exp(u);|double m = cabs(w)*exp(2*u);|' 'polar: modulo errado'

echo "=== os tectos — as guardas que descartavam em silencio"
uma palavra.c 's|#define KMAX 64|#define KMAX 4|'   'palavra: tecto baixado (forca truncamento)'
uma gauss.c   's|#define KMAX 40|#define KMAX 2|'   'gauss: tecto baixado (forca truncamento)'
