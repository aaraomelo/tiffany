#!/bin/bash
# protocolo.sh — O PROTOCOLO EM FASES: ele lista, abre, veste a túnica, e o painel valida.
#
# O Aarão: "faz um protocolo automatizado em fases: ele lista tudo que sabe e vai abrindo junto
# com a túnica, validando com o painel, mesmo procedimento. Aí ficamos observando nos logs. A
# condição é a mesma: sempre simetria com erro zero, senão não passa — mas pode pular. E ele pode
# consultar a base e escrever."
#
# AS FASES, e cada uma deixa rasto no log:
#
#   1. LISTAR     ele enumera o que sabe                     (nós não escolhemos os temas)
#   2. CONSULTAR  ele lê a base — o que já passou            (o lado de LER da túnica)
#   3. ABRIR      S₁, a afirmação sobre o tema
#   4. VESTIR     A = ν(S₁),  S₂ = ν(A)                      (a mesma operação, duas vezes)
#   5. VALIDAR    ν∘ν = id com resíduo 0?  PASSA : PULA      (o critério, e é duro)
#   6. ESCREVER   o que passa entra na base, com a cifra     (o lado de ESCREVER)
#
# O CRITÉRIO É DURO E O SALTO É PERMITIDO: erro 0 ou não passa — mas não pára o protocolo. O que
# pula fica no log com o resíduo que teve, e é isso que se observa.
#
# E O CONTROLO IMPEDE A FRAUDE MAIS ÓBVIA: se ele repetir S₁ em A (não indo a lado nenhum), o
# ν∘ν dá zero trivialmente. Por isso exige-se TAMBÉM que A esteja LONGE de S₁ — ida e volta, e
# não ficar parado.
set -e
GER=${GER:-qwen2.5:1.5b}
BASE=${BASE:-/tmp/protocolo_base.tsv}
LOG=${LOG:-/tmp/protocolo.log}
N=${N:-8}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
: > "$LOG"; [ -f "$BASE" ] || : > "$BASE"
echo "  protocolo: $GER · base $BASE · log $LOG"
python3 - "$GER" "$BASE" "$LOG" "$N" <<'PY'
import json, urllib.request, sys, math, time
GER, BASE, LOG, N = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
def api(r,c):
    d=json.dumps(c).encode()
    q=urllib.request.Request(f"http://localhost:11434/api/{r}",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(q,timeout=300).read())
def fala(p,n=80,t=0.0,cru=False):
    # O replace("\n"," ") era certo para as frases e DESTRUIU a listagem — ela vem uma por
    # linha, e eu apagava as linhas antes de as ler. Agora quem lista pede cru.
    r=api("generate",{"model":GER,"prompt":p,"stream":False,
        "options":{"temperature":t,"seed":7,"num_predict":n}})["response"].strip()
    return r if cru else r.replace("\n"," ")
def emb(t): return api("embeddings",{"model":"nomic-embed-text","prompt":t})["embedding"][:64]
def parimpar(x):
    n=len(x); return ([(x[k]+x[(n-k)%n])/2 for k in range(n)],[(x[k]-x[(n-k)%n])/2 for k in range(n)])
def r_id(a,b):
    pa,ia=parimpar(a); pb,ib=parimpar(b)
    num=sum((pb[k]-pa[k])**2 for k in range(len(pa)))+sum((ib[k]-ia[k])**2 for k in range(len(pa)))
    return math.sqrt(num/sum(pa[k]**2+ia[k]**2 for k in range(len(pa))))
def cifra(a,b,m=6):
    out=[]
    a,b=int(a),int(b)
    while b and len(out)<m:
        q=a//b; r=a-q*b
        out.append(q); a,b=b,r
    return " ".join(map(str,out)) if out else str(a)
L=open(LOG,"a")
def log(fase,msg):
    linha=f"[{fase:9}] {msg}"
    print("  "+linha[:112],flush=True); L.write(linha+"\n"); L.flush()

t0=time.time()
# ── FASE 1: LISTAR — ele diz o que sabe, e nós não escolhemos
log("1 LISTAR","a pedir-lhe a lista do que sabe")
bruto = fala(f"Liste {N} conceitos técnicos de matemática ou computação que você conhece bem. "
             f"Apenas os nomes, um por linha, sem números e sem explicação.", n=120, t=0.3, cru=True)
temas=[l.strip(" -•.0123456789\t") for l in bruto.split("\n") if 3<len(l.strip())<44][:N]
log("1 LISTAR",f"ele listou {len(temas)}: {', '.join(temas)[:88]}")

# O DUAL NÃO É O ANTÓNIMO DO DICIONÁRIO — é a INVERSÃO DOS ATRIBUTOS. O Aarão: "todos os temas
# têm dual. O antónimo de banco de dados: ele pressupõe uma coisa FINITA QUE GUARDA; o dual seria
# uma coisa INFINITA QUE CONTRAI, comprime o finito — como matéria e espaço, curvatura."
#
# Eu tinha concluído que "banco de dados não tem oposto" e estava errado: não tem oposto LEXICAL,
# e tem dual ESTRUTURAL. E o procedimento é ensinável — é o que vai no prompt agora.
# E O PAR É SEMPRE O MESMO, que é a segunda correção do Aarão: "sempre o CORPO e o AMBIENTE,
# duais, finito e infinito". Não são inversões de atributos avulsas — é UM par universal. Todo
# conceito é um CORPO (finito, delimitado, que guarda); o dual é o AMBIENTE dele (infinito, sem
# borda, que contrai e comprime). Matéria e espaço. É o mesmo par do dispositivo.c: o cérebro
# quente e o ambiente frio.
# A TABELA DOS PARES, e são todos o MESMO par visto de ângulos diferentes — o Aarão deu-os um a
# um: "sempre o corpo e o ambiente, duais, finito e infinito"; "o quente/frio e gradiente/
# divergente"; "reflexão/refração, produto direto/produto cruzado". Não são seis dualidades: é
# uma, com TREZE nomes, e cada um serve melhor um tipo de conceito.
PEDE=("Todo conceito é um CORPO; o dual dele é o AMBIENTE. É sempre o mesmo par, com treze nomes:\n"
      "  corpo <-> ambiente         finito <-> infinito\n"
      "  quente <-> frio            gradiente <-> divergente\n"
      "  reflexão <-> refração      produto direto <-> produto cruzado\n"
      "  guarda <-> contrai         mede <-> ordena\n"
      "  leitura <-> escrita        diferencial <-> integral\n"
      "  soma <-> produto           dilatação <-> erosão\n"
      "  PA (aritmética) <-> PG (geométrica)\n\n"
      "O corpo é finito, quente, guarda, reflete, mede. O ambiente é infinito, frio, contrai,\n"
      "refrata, ordena. Matéria e espaço; curvatura.\n\n"
      "Exemplo: um banco de dados é o CORPO (finito, guarda). O AMBIENTE dele é infinito e\n"
      "CONTRAI — comprime o finito.\n\n"
      "Afirmação: {a}\n\nSe ela descreve o CORPO, escreva a do AMBIENTE. Se já descreve o\n"
      "AMBIENTE, escreva a do CORPO. Uma frase só, sem preâmbulo e sem explicar o método.")
passou=pulou=0
for i,tema in enumerate(temas,1):
    # ── FASE 2: CONSULTAR a base
    ja=[l.split("\t")[0] for l in open(BASE) if l.strip()]
    log("2 CONSULTA",f"{tema}: a base tem {len(ja)} entradas" + (f"; já lá está" if tema in ja else ""))
    if tema in ja:
        log("2 CONSULTA",f"{tema}: PULA — já está na base"); pulou+=1; continue
    # ── FASE 3: ABRIR
    s1=fala(f"Responda em uma frase, sem preâmbulo: O que é {tema}?")
    log("3 ABRIR",f"{tema}: {s1[:82]}")
    # ── FASE 4: VESTIR a túnica — a mesma operação, duas vezes
    a=fala(PEDE.format(a=s1)); s2=fala(PEDE.format(a=a))
    log("4 VESTIR",f"{tema}: A={a[:44]} | S2={s2[:44]}")
    # ── FASE 5: VALIDAR
    x1,xa,x2=emb(s1),emb(a),emb(s2)
    r=r_id(x1,x2); ida=r_id(x1,xa)
    ok = (r<1e-9) and (ida>0.05)          # erro 0 E foi mesmo ao outro lado
    log("5 VALIDAR",f"{tema}: nu.nu={r:.6f}  ida={ida:.6f}  -> {'PASSA' if ok else 'PULA'}")
    if not ok:
        pulou+=1
        log("5 VALIDAR",f"{tema}: PULOU — {'a ida foi nula (nao saiu do sitio)' if ida<=0.05 else 'o residuo nao e zero'}")
        continue
    # ── FASE 6: ESCREVER na base, com a cifra como endereço
    a_c=round(sum(x1[0::2])*10000); b_c=round(sum(x1[1::2])*10000)
    with open(BASE,"a") as fb: fb.write(f"{tema}\t{cifra(a_c,b_c)}\t{r:.6f}\t{ida:.6f}\t{s1}\n")
    passou+=1
    log("6 ESCREVER",f"{tema}: cifra [{cifra(a_c,b_c)}] gravada na base")
log("FIM",f"{passou} passaram, {pulou} pularam, de {len(temas)} temas em {time.time()-t0:.1f}s")
L.close()
PY
