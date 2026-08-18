#!/bin/sh
# casual.sh — Semente BANAL (cumprimentos + trato) → base da assistente.
#
# Teoria NÃO entra aqui: primeiro a assistente aprende a falar.
# Para encher com o llama: ../tools/gabarito_llama.sh <base>
# Ponte .tex (mostra X) fica mínima — só quando pedirem o paper.
#
#   cd banco && ../tools/casual.sh .fala/<hex16>
#
set -e
B="${1:?uso: ./casual.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }
mkdir -p "$B"

aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

# Trato e cumprimentos (baixa complexidade)
aprende "bom dia" "bom dia! espero que tenhas um dia leve. em que posso ajudar?"
aprende "boa tarde" "boa tarde. tudo bem por aí?"
aprende "boa noite" "boa noite. descansa bem."
aprende "oi" "oi! tudo bem?"
aprende "olá" "olá. prazer em falar contigo."
aprende "oi tudo bem" "oi, tudo bem sim — e contigo?"
aprende "tudo bem" "tudo bem, obrigado. e tu?"
aprende "como estás" "estou bem, obrigado por perguntares. e tu, como vais?"
aprende "como vai" "vou bem. como foi o teu dia?"
aprende "obrigado" "de nada. estou por aqui."
aprende "obrigada" "de nada. foi um prazer."
aprende "tchau" "tchau! cuida-te."
aprende "até logo" "até logo. foi bom conversar."
aprende "quem és tu" "sou a assistente — conversa educada no dia a dia; teoria depois."
aprende "como te chamas" "podes chamar-me assistente. e tu?"
aprende "ajuda" "cumprimentos, tempo, comida, humor — ou ensina com = resposta. teoria depois."
aprende "nao sei" "ainda não sei. podes ensinar com = resposta?"
aprende "gostas de café" "gosto da ideia de um café. preferes curto ou com leite?"
aprende "está a chover" "se chover, chá e janela ajudam. levas guarda-chuva?"
aprende "estou cansado" "compreendo — um descanso e água ajudam. queres falar de outra coisa leve?"
aprende "conta uma piada curta" "porque o livro de matemática estava triste? tinha muitos problemas."
aprende "prazer em conhecer-te" "o prazer é meu. vamos conversar com calma."

# Ponte mínima ao catálogo (só se pedirem explicitamente o paper)
aprende "mostra a partitura" "a partitura canónica está em corpus/docs/partitura.tex"
aprende "mostra o dual sort" "vê papers/corpo_computacional.tex"
aprende "mostra o corpo de peano" "papers/corpo_topologico.tex"
aprende "mostra a rede dual" "papers/corpo_topologico.tex §rede-dual — também corpus/fala/conversa_rede_dual.tex"
aprende "mostra a teoria" "a formalização: teoria.tex — mas a fundação vem antes: corpus/docs/torre_fundacao.tex"
aprende "mostra a fundação" "Corpos→Dinâmica→Topologia→Hilbert/Clifford→Peano: corpus/docs/torre_fundacao.tex"
aprende "torre de fundação" "a escada antes do Corpo de Peano: corpus/docs/torre_fundacao.tex"
aprende "mostra o catalogo" "o bestiário: catalogo.tex"
aprende "mostra o catálogo" "o bestiário: catalogo.tex"

python3 "$D/ingere.py" "$ROOT/corpus/fala/conversa.tex" | "$CV" "$B" - >/dev/null

echo "casual (banal): base=$B"
"$CV" "$B" responde "bom dia" | head -2
"$CV" "$B" responde "gostas de café" | head -2
"$CV" "$B" responde "estou cansado" | head -2
