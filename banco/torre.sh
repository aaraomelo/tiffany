#!/bin/bash
# torre.sh — A TORRE AUTÓNOMA, agora em DISCO: ele escreve sem parar, e um agente ACIMA revê.
#
# O Aarão: "não quero LLM rodando em memória, usa o disco e o processador multifractal pra
# simular RAM no disco — não torra aqui."
#
# NÍVEL 0   qwen2.5:1.5b   escreve o que sabe, sem ninguém a julgar
# NÍVEL 1   gemma2:2b      revê o que o nível 0 escreveu — e escreve a revisão dele
# NÍVEL 2   a vigília      lê várias revisões e sintetiza
#
# O QUE MUDOU, e porquê. A corrida de 01/08 escreveu ~400 conceitos e perdeu-os TODOS: o corpus
# vivia em /tmp, que nesta máquina é tmpfs — RAM. O reboot da manhã apagou tudo. E a mesma
# confusão tem uma segunda face, pior: a swap daqui é /dev/zram0, 8 GiB COMPRIMIDOS DENTRO DA
# RAM. Aqui não há para onde transbordar; quem pressiona a memória pressiona-a de novo ao
# transbordar. O disco tem de ser pedido de propósito, em dois lugares:
#
#   O CORPUS   vai para o banco multifractal do mmu.c — endereço b^n, a folha é o dado, e a
#              RAM do processo não cresce com as células (medido no §M5, contra controlo).
#   OS PESOS   use_mmap: as páginas do modelo passam a ser mapeadas do ficheiro. Sob pressão o
#              kernel DESCARTA-AS e relê do disco, de graça, porque estão limpas. Em 01/08 o
#              ollama fez o contrário — "disabling mmap due to host memory pressure" — e
#              desligou o disco exatamente quando ele era mais preciso.
#
#   O CONTEXTO  num_ctx=1024, e AQUI EU ERRAVA. Escrevi primeiro que isto cortava "a parte
#              irredutivelmente anónima da RAM", e a medida desmentiu-me: com ctx 4096 o
#              RssAnon do llama-server é 220260 kB, com 1024 é 220036 kB — 0,1%, nada.
#              O KV cache não está na RAM. Está na GPU:
#                  ctx 4096  ->  llama_kv_cache: CUDA0 KV buffer size = 112,00 MiB
#                  ctx 1024  ->  llama_kv_cache: CUDA0 KV buffer size =  28,00 MiB
#              São 84 MiB de VRAM, e é aí que dói: a placa é uma GTX 1650 de 4 GiB, e em
#              01/08 o ollama trabalhava com "available_vram 2.5 GiB" para dois modelos. Às
#              22:30 desistiu com "unable to refresh free memory" e "GPU discovery timed out".
#              O corte é a correção certa — pelo recurso escasso (VRAM), não pelo abundante
#              (23 GiB de RAM). Duas faces, dois remédios: mmap para a RAM, contexto para a
#              placa.
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
N0=${N0:-qwen2.5:1.5b}
N1=${N1:-gemma2:2b}
BANCO=${BANCO:-$CD/../.torre}
LOG=${LOG:-$BANCO/torre.log}
VOLTAS=${VOLTAS:-999}
REVISA_A_CADA=${REVISA_A_CADA:-5}
NUM_CTX=${NUM_CTX:-1024}
MMU=${MMU:-$CD/mmu}

mkdir -p "$BANCO"

# ── A LIÇÃO DE 01/08, COMO VERIFICAÇÃO E NÃO COMO COMENTÁRIO ───────────────────────────────
# Um banco em tmpfs não é um banco: é RAM com nome de ficheiro, e desaparece no reboot sem
# avisar ninguém. Isto recusa-se a arrancar em vez de descobrir a perda de manhã.
FS=$(stat -f -c %T "$BANCO" 2>/dev/null || echo desconhecido)
if [ "$FS" = "tmpfs" ] || [ "$FS" = "ramfs" ]; then
  echo "torre: RECUSO — o banco '$BANCO' está em $FS, que é RAM." >&2
  echo "       Foi assim que a corrida de 01/08 perdeu ~400 conceitos no reboot." >&2
  exit 1
fi
[ -x "$MMU" ] || { echo "torre: falta o banco — compile com 'make -C $CD mmu'" >&2; exit 1; }

# ── UMA TORRE DE CADA VEZ, e por LOCK e não por nome de processo ───────────────────────────
# `pgrep -f torre.sh` casa com qualquer linha de comando que contenha o nome — um tail, um
# editor, o próprio shell que a invocou. Era assim que o guarda via uma torre viva que não
# existia e nunca relançava nada. O lock não se engana: ou o ficheiro está segurado por um
# processo vivo, ou não está. E solta-se sozinho quando o processo morre, seja como for que
# morra — que é exatamente o caso que precisamos de detetar.
exec 9>"$BANCO/torre.lock"
if ! flock -n 9; then
  echo "torre: já há uma a correr (o lock está seguro) — saio sem duplicar" >&2
  exit 0
fi

echo "torre: banco em $BANCO ($FS), contexto $NUM_CTX, pesos por mmap"

python3 - "$N0" "$N1" "$BANCO" "$LOG" "$VOLTAS" "$REVISA_A_CADA" "$NUM_CTX" "$MMU" <<'PY'
import json, urllib.request, sys, time, os, subprocess
N0,N1,BANCO,LOG,VOLTAS,CADA,NCTX,MMU = (sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4],
                                        int(sys.argv[5]),int(sys.argv[6]),int(sys.argv[7]),sys.argv[8])

# ── O BANCO: uma invocação do mmu por acesso, e nada residente entre elas ──────────────────
# O processo nasce, lê o topo do disco, escreve a célula, e morre. É o oposto de um cache: o
# custo é um fork por registo, e o ganho é que a torre pode correr a noite inteira sem que a
# ocupação suba um único kB. Era a subida que matava.
def banco_env(nivel):
    e = dict(os.environ); e["MMU_RAIZ"] = os.path.join(BANCO, nivel); return e
def poe(nivel, linha):
    p = subprocess.run([MMU,"poe"], input=linha.encode()[:1024],
                       capture_output=True, env=banco_env(nivel))
    return int(p.stdout.strip() or 0) if p.returncode == 0 else -1
def topo(nivel):
    p = subprocess.run([MMU,"topo"], capture_output=True, env=banco_env(nivel))
    return int(p.stdout.strip() or 0) if p.returncode == 0 else 0
def le(nivel, addr):
    p = subprocess.run([MMU,"le",str(addr)], capture_output=True, env=banco_env(nivel))
    return p.stdout.decode("utf-8","replace") if p.returncode == 0 else ""

def api(r,c,to=600):
    d=json.dumps(c).encode()
    q=urllib.request.Request(f"http://localhost:11434/api/{r}",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(q,timeout=to).read())
def fala(mod,p,n=140,t=0.6):
    try:
        r=api("generate",{"model":mod,"prompt":p,"stream":False,
            "options":{"temperature":t,"num_predict":n,
                       "use_mmap":True,      # os pesos vêm do disco, e voltam para lá sob pressão
                       "num_ctx":NCTX}})["response"].strip()
        return r.replace("\n"," ")
    except Exception as e:
        return f"[erro: {type(e).__name__}]"
def hash_de(txt,m=6):
    bs=txt.encode("utf-8")
    a=sum(b*(i+1) for i,b in enumerate(bs)) or 1
    b=sum(x*x for x in bs) or 1
    o=[]
    while b and len(o)<m:
        q=a//b; a,b=b,a-q*b; o.append(q)
    return " ".join(map(str,o))
L=open(LOG,"a",buffering=1)
def log(m):
    s=time.strftime("%H:%M:%S")+" "+m
    print(s,flush=True); L.write(s+"\n")

log(f"=== TORRE — n0={N0}  n1={N1}  ate {VOLTAS} voltas, revisao a cada {CADA} ===")
log(f"=== banco {BANCO}  n0={topo('n0')} n1={topo('n1')} ja escritos (retoma) ===")

# Os temas ja vistos: leem-se do DISCO, e so os ultimos. Guardar o conjunto todo em memoria
# seria repor o indice que o banco existe para dispensar — e a lista so serve para o prompt.
def vistos_recentes(k=12):
    t = topo("n0"); out = []
    for a in range(max(1, t-k+1), t+1):
        linha = le("n0", a)
        if linha: out.append(linha.split("\t")[0])
    return out
# A COMPARAÇÃO É NORMALIZADA, e não literal. "Entropia Boltzmann" e "Entropia de Boltzmann"
# passaram as duas por serem strings diferentes — e são o mesmo conceito. Tira-se o acento, a
# caixa e as palavras de ligação, e o que resta são as palavras que carregam o sentido. É
# mecânico: não julga o conceito, só a forma.
import unicodedata
LIGACAO = {"de","da","do","das","dos","o","a","os","as","e","em","no","na","um","uma"}
def normaliza(s):
    s = unicodedata.normalize("NFD", s.lower())
    s = "".join(c for c in s if unicodedata.category(c) != "Mn")
    palavras = [p.strip(".,;:()[]") for p in s.split()]
    return " ".join(sorted(p for p in palavras if p and p not in LIGACAO))

def ja_existe(tema):
    alvo = normaliza(tema)
    if not alvo: return True                   # sem palavra com sentido, não é conceito
    t = topo("n0")
    for a in range(max(1, t-200), t+1):        # janela finita: o banco é grande, a RAM não
        linha = le("n0", a)
        if linha and normaliza(linha.split("\t")[0]) == alvo: return True
    return False

v=0
while v < VOLTAS:
    v+=1
    # ── NÍVEL 0: ele escolhe um tema que ainda não escreveu, e escreve
    # A DIVERSIDADE É MECÂNICA, não avaliativa: se se lhe pede "um conceito novo" ele fica
    # preso na sílaba do anterior (Neonismo, Neonopia, Neonocentrism...). Percorre-se um leque
    # de ÁREAS por rotação — nós não escolhemos o conceito, só de onde ele o vai buscar.
    AREAS=[("matemática","Teorema de Green"),      ("física","Efeito Casimir"),
           ("química","Ligação de hidrogénio"),    ("biologia","Mitocôndria"),
           ("computação","Tabela de dispersão"),   ("engenharia","Viga em balanço"),
           ("astronomia","Paralaxe estelar"),      ("linguística","Fonema"),
           ("música","Contraponto"),               ("economia","Custo marginal"),
           ("geologia","Falha transformante"),     ("medicina","Homeostase"),
           ("filosofia","Navalha de Occam"),       ("estatística","Regressão à média"),
           ("eletrónica","Espelho de corrente"),   ("criptografia","Curva elíptica"),
           ("termodinâmica","Ciclo de Otto"),      ("álgebra","Ideal maximal"),
           ("ótica","Difração de Fraunhofer"),     ("materiais","Recristalização")]
    area, _ = AREAS[v % len(AREAS)]
    # O EXEMPLO VEM DE OUTRA ÁREA, e isto é a correção de um defeito meu. Um exemplo fixo
    # ("Entropia de Shannon") resolveu o eco mas virou ATRATOR: 5 das 8 voltas seguintes foram
    # "Entropia Clausius", "Entropia Boltzmann", "Entropia de Boltzmann"... É o mesmo fenómeno
    # da sílaba (Neonismo → Neonopia) que o leque de áreas já tinha resolvido uma vez, e eu
    # reintroduzi-o pela porta do exemplo. Tirando-o de OUTRA área (deslocamento 7, primo com
    # 20, portanto percorre as vinte), ele ensina o FORMATO sem sugerir o CONTEÚDO.
    _, exemplo = AREAS[(v + 7) % len(AREAS)]
    recentes = vistos_recentes()
    ja=", ".join(recentes) if recentes else "(nada ainda)"
    # O PEDIDO POR EXEMPLO. Pedir "só o nome" não bastava: o modelo devolvia a forma da
    # pergunta ("O ciclo do carbono na Terra não está n..."), e o filtro recusava volta após
    # volta. Um modelo de 1,5 B obedece ao FORMATO que vê, não ao que se lhe descreve — então
    # mostra-se o formato em vez de o explicar, e diz-se o que NÃO escrever.
    t=fala(N0,f"Area: {area}.\nJa escrevemos sobre: {ja}\n\n"
              f"Escreva o NOME de um conceito de {area} que nao esteja na lista.\n"
              f"Responda so com o nome. Nao escreva frase, nao repita o pedido.\n\n"
              f"Exemplo de resposta boa: {exemplo}\n"
              f"Exemplo de resposta ma: Um conceito que nao esta na lista e o {exemplo}\n\n"
              f"Nome:",n=14,t=0.85)
    t=t.strip(" .-•\"\'\n:")[:44]
    # O FILTRO DO ECO. Em 01/08 passou "Claro! Aqui está um conceito novo que não es", e no
    # primeiro teste desta versão passaram outros três — "Eletricidade e magnetismo não estão
    # na sua l", "Conceitos Astronômicos Não Presentes na List". O prefixo não os apanha porque
    # o defeito não é o preâmbulo: é o modelo a DEVOLVER A FORMA DO PEDIDO em vez de responder.
    #
    # E a régua não é escolhida por mim — está no prompt. Pedimos "duas ou três palavras no
    # máximo", logo qualquer coisa acima disso já é outra coisa que não um conceito. O teto de
    # 5 dá folga para artigos ("o sistema circulatório") sem deixar passar uma frase.
    eco = ("não est","nao est","não present","nao present","não list","nao list",
           "não incluí","nao inclui","dessa lista","nessa lista","da lista")
    baixo = t.lower()
    if baixo.startswith(("claro","aqui","um conceito","o conceito","desculpe","certamente")) \
       or any(m in baixo for m in eco) or len(t.split()) > 5:
        log(f"[{v:3}] (eco do pedido, recusado: '{t[:38]}')")
        t=""
    elif not t or t.startswith("[erro") or ja_existe(t):
        log(f"[{v:3}] (repetiu ou falhou: '{t[:30]}') — segue")
        t=""
    if t:
        txt=fala(N0,f"Escreva o que voce sabe sobre {t}. Duas frases, sem preambulo.",n=140)
        h=hash_de(txt)
        addr=poe("n0", f"{t}\t{h}\t{len(txt)}\t{txt}")
        log(f"[{v:3}] n0 {t}  -> célula {addr}")
        log(f"      hash [{h}]  {len(txt)}B")
        log(f"      > {txt[:110]}")
    # ── NÍVEL 1: de tempos a tempos, o de cima lê o de baixo e revê
    #
    # ONDE ELE VAI é lido do DISCO, e não de uma variável. Na primeira versão isto era um
    # contador em memória que arrancava a zero — e como o banco retoma e o contador não, a
    # segunda corrida revia de novo as células 1..6 e nunca chegava às novas: 12 revisões para
    # 11 conceitos. É o erro de 01/08 outra vez, de outra roupa: o estado que sobrevive tem de
    # morar onde o corpus mora.
    #
    # E a correspondência é 1:1 POR CONSTRUÇÃO — cada célula do n0 produz exatamente uma no n1,
    # mesmo a ilegível, que gasta uma marca. É isso que faz de topo(n1) o índice de onde
    # recomeçar, sem tabela à parte. Se a marca fosse saltada, o alinhamento partia-se em
    # silêncio e o nível 1 passaria a rever o conceito errado.
    if v % CADA == 0:
        alvo = topo("n0")
        inicio = topo("n1") + 1
        for a in range(inicio, min(alvo, inicio + CADA - 1) + 1):
            linha = le("n0", a)
            p = linha.split("\t")
            if len(p) < 4:
                poe("n1", f"(ilegível)\t0\t0\tcélula {a} do n0 não tem os quatro campos")
                log(f"      ── n1 salta a célula {a} (ilegível), e gasta a marca")
                continue
            tema, texto = p[0], p[3]
            r=fala(N1,f"Um sistema menor escreveu isto sobre '{tema}':\n\n{texto}\n\n"
                      f"Reveja: o que esta correto, o que esta errado, e o que falta. "
                      f"Tres frases, sem preambulo.",n=200,t=0.3)
            hr=hash_de(r)
            poe("n1", f"{tema}\t{hr}\t{len(r)}\t{r}")
            log(f"      ── n1 revê {tema}  hash [{hr}]")
            log(f"         > {r[:106]}")
    time.sleep(0.5)
log(f"=== fim: {topo('n0')} no nivel 0, {topo('n1')} revistos ===")
PY
