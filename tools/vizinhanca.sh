#!/bin/sh
# vizinhanca.sh — Bandas 1–7: erro → abreviação → oral → variação → ruído → continuidade → não-colagem.
# Teoria escondida. Português coloquial BR.
#
#   cd banco && ../tools/vizinhanca.sh .fala/<hex>
#
set -e
B="${1:?uso: ./vizinhanca.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
TEX="$ROOT/corpus/fala/conversa_vizinhanca.tex"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }
mkdir -p "$B"
aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

# ========== Banda 1 — erros leves ==========
aprende "muzica" "Gosto de música — deixa o ambiente mais leve."
aprende "gostas de muzica" "Gosto sim de música — deixa o dia mais leve."
aprende "cafeh" "Café quentinho sempre cai bem."
aprende "um cafeh" "Claro — um café a caminho."
aprende "canssado" "Compreendo. Um descanso curto ajuda."
aprende "estou canssado" "Compreendo. Um descanso curto ajuda."
aprende "estou canssado hoje" "Então talvez seja boa hora para descansar um pouco."
aprende "amanha" "Amanhã a gente retoma com calma."
aprende "até amanha" "Até amanhã. Cuida-te."
aprende "obrigadoo" "De nada. Estou por aqui."
aprende "bom diaa" "Bom dia! Em que posso ajudar?"
aprende "vc gosta de cafe" "Gosto sim, especialmente quando está bem quente."
aprende "q horas sao" "Pergunta «que horas são» — dessa forma eu leio o relógio."
aprende "to cansado hj" "Então pega leve hoje e tenta descansar."
aprende "muscia" "Gosto de música — deixa o ambiente mais leve."
aprende "musicaa" "Gosto de música — deixa o ambiente mais leve."
aprende "maestroo" "O maestro dá o compasso e a orquestra segue."
aprende "maesrto" "O maestro dá o compasso e a orquestra segue."
aprende "maestr" "O maestro dá o compasso e a orquestra segue."
aprende "partitrua" "A partitura é a música escrita — quem lê, ouve com os olhos."
aprende "orqestra" "Uma orquestra ao vivo é uma experiência e tanto."
aprende "motora" "Motor a trabalhar bem é meio caminho andado."

# ========== Banda 2 — abreviações ==========
aprende "vc" "Oi — diga lá."
aprende "tb" "Também! Conta mais."
aprende "tbm" "Também. Como posso ajudar?"
aprende "pq" "Boa pergunta — conta o contexto."
aprende "q" "Diz o que precisas."
aprende "blz" "Beleza. Em que posso ajudar?"
aprende "flw" "Falou! Até já."
aprende "obg" "De nada."
aprende "msg" "Podes mandar a mensagem — estou a ouvir."
aprende "hj" "Hoje está a correr bem por aqui. E contigo?"
aprende "dps" "Combinado — depois a gente fala."
aprende "vc tbm gosta de cafe?" "Gosto sim, especialmente quando está bem quente."
aprende "vc gosta de cafe?" "Gosto! Café passado na hora é difícil de recusar."
aprende "oq vc recomenda pra jantar?" "Uma massa simples ou um sanduíche caprichado já resolvem."
aprende "bom dia, td bem?" "Bom dia! Tudo bem por aqui, e contigo?"
aprende "td bem" "Tudo bem, obrigado. E tu?"
aprende "blz, valeu" "Por nada! Estou por aqui."
aprende "valeu" "De nada."
aprende "oq" "Diz o que precisas — estou a ouvir."
aprende "maes" "Diz-me mais — estou a ouvir."
# molduras com buraco: o corte moldura→parâmetro (Dual Sort; '_' recebe o resto da fala)
aprende "gosto de" "Boa escolha — ___ também me agrada. Conta mais!"
aprende "fala-me de" "Sobre ___? Diz-me primeiro o que te interessa nisso."
aprende "o que achas de" "De ___? Acho interessante — o que te chamou a atenção?"
aprende "metro" "O metro é prático para andar pela cidade. Ou falavas de outra coisa?"
aprende "part" "Conta-me mais — parte de quê?"
aprende "orq" "Conta mais — estou a ouvir."
aprende "comp" "Conta-me mais — estou a ouvir."
aprende "doc" "Falas de um documento? Conta-me mais."
aprende "info" "Diz-me que informação procuras."
aprende "config" "O que estás a configurar? Conta-me."

# ========== Banda 3 — fala oral (não corrigir o utilizador) ==========
aprende "to morrendo de fome" "Então está na hora de arrumar alguma coisa gostosa para comer."
aprende "Tô morrendo de fome." "Nesse caso, é melhor procurar alguma coisa rápida para comer."
aprende "ta frio ai?" "Parece que hoje está pedindo um café quentinho."
aprende "tá frio aí?" "Parece que hoje está pedindo um café quentinho."
aprende "nem dormi direito hj" "Aí o dia fica pesado mesmo. Tomara que consigas descansar mais tarde."
aprende "to cansado" "Então pega leve hoje e tenta descansar um pouco."
aprende "ta calor" "Água fresca e sombra ajudam. Queres outra ideia leve?"
aprende "num aguento mais" "Respira fundo. Queres falar de algo simples para aliviar?"
aprende "eai" "E aí! Tudo bem?"
aprende "e ai" "E aí! Como vai?"
aprende "falae" "Fala! Em que posso ajudar?"
aprende "to bem" "Que bom. Conta se precisares de algo."
aprende "to bm hj" "Que bom! Espero que o teu dia continue assim."
aprende "to com sonoo" "Parece que o corpo pede uma pausa."
aprende "vc ta bem?" "Estou bem! E você, como está?"

# ========== Banda 4 — mesma intenção, formulações ==========
# café
aprende "gosta de café?" "Gosto! Café passado na hora é difícil de recusar."
aprende "vc toma café?" "Tomo sim, sobretudo de manhã."
aprende "café ou chá?" "Depende do dia — café para energia, chá para acalmar."
aprende "bora um café?" "Bora. Quentinho fica ainda melhor."
aprende "queria tomar um cafezinho" "Ótima ideia. Curto ou com leite?"
aprende "to precisando de café" "Então um café agora pode ajudar."
aprende "cafezim agora ia bem" "Ia mesmo. Vamos a isso."
aprende "Que vc acha de um cafe?" "Acho uma ótima ideia, principalmente se estiver quentinho."
aprende "um cafe por favor" "Claro — um café quentinho a caminho."
aprende "oq vc acha de um cafe hj" "Acho óptimo. Café hoje cai bem."
# fome
aprende "Estou com fome." "Então vamos pensar em alguma coisa gostosa para comer."
aprende "estou com fome" "Então vamos pensar em alguma coisa gostosa para comer."
aprende "to com fome" "Quer uma sugestão rápida para comer?"
aprende "preciso comer" "Então bora resolver esse almoço ou lanche."
aprende "bora almoçar?" "Bora. Arroz, proteína e salada simples resolvem."
aprende "o que dá pra comer agora?" "Sanduíche, omelete ou fruta — rápido e bom."
aprende "tem alguma ideia pra jantar?" "Massa simples ou sanduíche caprichado já resolvem bem."
aprende "tem alguma coisa pra beliscar?" "Uma fruta, um pão ou um sanduíche simples podem quebrar o galho."
aprende "estou faminto" "Hora de comer algo simples e bom."
aprende "quero um lanche" "Sanduíche, fruta ou pão com queijo resolvem."
aprende "não aguento de fome" "Vamos tratar disso já — algo rápido."
aprende "oq tem pra almoco?" "Podes fazer um arroz com alguma proteína e uma salada simples."
aprende "to com fomr" "Então está na hora de pensar em alguma coisa gostosa para comer."

# ========== Banda 5 — ruído sem perder intenção ==========
aprende "vc gosta d cafe? kkk" "Gosto sim! Café quente é sempre bem-vindo."
aprende "to cansado pra caramba hj" "Então hoje é dia de ir com calma e descansar quando puderes."
aprende "bom dia meu amigo" "Bom dia! Que o dia seja leve."
aprende "q calor é esse hj mano" "Calor mesmo. Água e sombra ajudam."
aprende "me indica alguma coisa simples pra comer" "Sanduíche, omelete ou arroz com ovo — simples e resolve."
aprende "aff to morrendo de sono" "Uma pausa curta e água podem ajudar."
aprende "kkk eai blz" "Beleza! Em que posso ajudar?"
aprende "opa bom diaa" "Bom dia! Tudo bem?"
aprende "socorro to com fome kkk" "Vamos resolver: algo rápido tipo sanduíche ou fruta."
aprende "mano ta frio demais" "Café quentinho ou um cobertor — escolhe o teu lado."

# ========== Banda 6 — continuidade / histerese de contexto ==========
# âncoras de diálogo (cada turno é um par; a continuidade testa-se no bench com H_k)
aprende "to com fome" "Quer uma sugestão rápida para comer?"
aprende "sim" "Um sanduíche quente ou uma omelete são opções simples."
aprende "sem queijo" "Então dá para fazer uma omelete simples com tomate e ervas."
aprende "sem qjo" "Então dá para fazer uma omelete simples com tomate e ervas."
aprende "s/ queijo" "Então dá para fazer uma omelete simples com tomate e ervas."
aprende "n quero queijo" "Então dá para fazer uma omelete simples com tomate e ervas."
aprende "queijo nao" "Então dá para fazer uma omelete simples com tomate e ervas."
aprende "queijo não" "Então dá para fazer uma omelete simples com tomate e ervas."
# continuidade café
aprende "bora um café?" "Bora. Quentinho fica ainda melhor."
aprende "com leite" "Perfeito — café com leite a caminho."
aprende "sem acucar" "Combinado — sem açúcar."
aprende "sem açúcar" "Combinado — sem açúcar."
aprende "s/ acucar" "Combinado — sem açúcar."

# ========== Banda 7 — não-colagem (mudança clara de assunto) ==========
aprende "qual é a capital do Chile?" "A capital do Chile é Santiago."
aprende "capital do Chile" "A capital do Chile é Santiago."
aprende "qual a capital do brasil" "A capital do Brasil é Brasília."
aprende "quantos dias tem uma semana" "Uma semana tem 7 dias."
aprende "que horas são" "Não tenho relógio fiável aqui — mas posso continuar a conversa."

# ========== Banda 8 — ambiguidade controlada ==========
# sem inventar: pedir/usar contexto
aprende "banco" "Banco de sentar, de dinheiro, ou outra coisa? Diz o contexto."
aprende "manga" "Manga de fruta ou de camisa? Conta o que tinhas em mente."
aprende "jantar" "Boa — jantar. Queres uma ideia simples do que comer?"
aprende "frio" "Frio mesmo. Café quentinho ou um cobertor — o que preferes?"
aprende "cafe" "Café: queres um convite para tomar, ou só falavas do assunto?"
aprende "café" "Café: queres um convite para tomar, ou só falavas do assunto?"
# com contexto prévio (pares que o bench encadeia após comida/roupa)
aprende "quero manga pra comer" "Uma manga madura cai bem — doce e leve."
aprende "rasgou a manga" "Ai — a manga da camisa. Precisas de agulha ou outra peça?"
aprende "vou ao banco" "Combinado. Levas documentos ou só um levantamento rápido?"
aprende "sentar no banco" "Bom sítio para descansar um pouco."

# ========== Banda 9 — reparação (mover o estado, não insistir) ==========
aprende "nao, queria algo doce" "Ah, combinado — então algo doce: fruta, bolo simples ou chocolate."
aprende "não, queria algo doce" "Ah, combinado — então algo doce: fruta, bolo simples ou chocolate."
aprende "nao isso" "Ok, solto essa ideia. O que preferias em vez disso?"
aprende "na verdade nao" "Sem problema — mudamos. Diz o que queres agora."
aprende "esquece o cafe" "Esquecido. Em que posso ajudar então?"
aprende "melhor nao" "Tudo bem. Ficamos por aqui ou queres outra coisa?"
aprende "muda de assunto" "Claro. Sobre o que queres falar?"

# canónicos vizinhos
aprende "você gosta de música?" "Gosto sim! Música sempre deixa o ambiente mais leve."
aprende "Você gosta de muzica?" "Gosto sim! Música sempre deixa o ambiente mais leve."
aprende "estou cansado" "Compreendo. Um descanso curto ajuda."
aprende "dormi mal ontem" "Poxa, espero que consigas descansar melhor hoje."
aprende "tudo bem" "Tudo bem, obrigado. E tu?"
aprende "bom dia" "Bom dia! Espero que tenhas um dia leve. Em que posso ajudar?"

# tex + contagem
python3 "$D/ingere.py" "$TEX" | "$CV" "$B" - >/dev/null || true
n=$("$CV" "$B" responde "to com fome" 2>/dev/null | head -1)
echo "vizinhança bandas 1–9: base=$B"
echo "smoke to com fome → $n"
for q in "muzica" "banco" "manga" "nao, queria algo doce" "qual é a capital do Chile?"; do
  printf 'Q: %s → ' "$q"
  "$CV" "$B" responde "$q" | head -1
done
