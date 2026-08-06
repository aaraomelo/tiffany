#!/bin/sh
# semear.sh — o que a assistente sabe do sistema. Só factos verificáveis no código;
# nada aqui é inventado, e cada par corresponde a algo que a bateria mede.
#   ./semear.sh <base>
# corre de onde for chamado: os caminhos relativos daqui contam a partir de tools/
cd "$(dirname "$0")" || exit 1
B="${1:-/tmp/barr/tiffany}"
# a base tem de existir antes de se escrever nela: cria-se a pasta, e a ferramenta
# deixa de depender de alguem a ter montado antes.
mkdir -p "$(dirname "$B")" 2>/dev/null
# o conversa.c mudou-se para banco/ em 03/08 (é armazenamento, não medição).
# Compila-se de lá; o binário fica em banco/bin/.
A="../banco/bin/conversa"
[ -x "$A" ] || { mkdir -p ../banco/bin && cc -O2 -std=c99 -I../lib ../banco/conversa.c -lm -o "$A"; }
p(){ "$A" "$B" aprende "$1" "$2" >/dev/null; }

# --- os comandos do banco (os que o executa() despacha) ---
p "que comandos o banco tem" "CREATE TABLE, INSERT INTO, SELECT, UPDATE, DELETE, INSERT TEXTO, BUSCA TEXTO, ACHA TEXTO, DISTANCIA, CORPOS, CABECALHO, MARTELO, VERIFICA, MINERA e CONFIG."
p "como crio uma tabela" "sql <base> \"CREATE TABLE t (a,b,c)\" — sem tipo é INTEIRO; há RACIONAL, AUREO, MORFICO e CRISTALINO."
p "como insiro uma linha" "sql <base> \"INSERT INTO t VALUES (1,2,3)\""
p "como leio uma tabela" "sql <base> \"SELECT * FROM t\" — e o WHERE filtra."
p "como corro um script" "sql <base> - lê um comando por linha da entrada; linha vazia e -- são comentário."
p "como guardo uma configuração" "sql <base> \"CONFIG pool_user 'a chave'\" põe, e sql <base> \"CONFIG pool_user\" lê. Fica num ficheiro .conf a 0600."

# --- a cifra e a régua ---
p "o que é a cifra" "É a fração contínua, e é o único sistema de coordenadas do sistema: a mesma para um texto, um número, um corpo, um formato e uma linguagem."
p "como se compara duas coisas" "Anda-se o caminho comum das cifras e para-se no primeiro termo que diverge. A distância é 1/2^k com k o prefixo comum."
p "o que é a régua do rei" "É a do corpo áureo, [1;1,1,1,...] — sigma = 1 + 1/sigma. A indução tem essa cifra: base mais passo, e o passo é sempre o mesmo."
p "quantos corpos há no catálogo" "42 entradas em 18 lugares distintos. Formatos e linguagens entraram e nenhum abriu lugar novo."
p "que corpo é o json" "O áureo — razão 1, sinal -1. O parêntese abre e fecha, e as duas direções cancelam-se. C, Lisp, HTML e o ambiente do TeX caem no mesmo lugar."
p "o que é o decreto" "É o único método sem dual: o absorvente. É ele que diz não sei em vez de inventar."

# --- as réguas do mórfico ---
p "quais são as três réguas" "Erosão (o prefixo), dilatação (a subsequência) e torção (o entrelaçar). São as operações do corpo mórfico, e todas reversíveis."
p "o que é a erosão" "É o prefixo: a fala tal como veio, e o prefixo mais longo que existir. É o WHERE."
p "o que é a dilatação" "É a subsequência: acha a fala com ruído à frente ou no meio, e é o dual da erosão."
p "o que é a torção" "É o entrelaçar: duas falas no mesmo canal, desentrelaçadas. Reverte com o dual, que são os comprimentos."

# --- a mineração ---
p "como ligo a mineração" "CONFIG pool_host, CONFIG pool_user, CONFIG mina_ativa '1', e depois sql <base> MINERA."
p "como paro a mineração" "sql <base> \"CONFIG mina_ativa '0'\" — o worker vê a flag a cada volta e para sozinho. Não é preciso matar processo."
p "o que é o martelo" "MARTELO <de> <ate> varre a faixa de nonces; VERIFICA <nonce> confere. Um estica e o outro contrai — é por isso que o trabalho é prova."
p "o que é o midstate" "Os primeiros 64 bytes do cabeçalho não mudam com o nonce, então a dobra faz-se uma vez por job e não uma vez por nonce. Passou de 1,15 a 1,94 MH/s."
p "o que é a merkle" "É a dobra: pares que se juntam num, nível a nível, até sobrar um. A branch é o desdobramento — o caminho de uma folha até à raiz."

# --- as regras duras ---
p "posso usar ram" "Não. Zero malloc, calloc e realloc no toolkit. O que entra vai para o banco e é de lá que se lê."
p "onde ficam os dados" "No banco, em disco, por pread e pwrite. Nada em memória — nem tabela, nem índice, nem buffer que cresça."
p "o que é o barramento" "É a aplicação. Os bancos reagem ao que nele passa; ninguém é chamado pelo nome e não há registo de quem tem o quê."
p "como sei se algo está certo" "Corre-se a bateria: cada medidor devolve resíduo 0 ou falha. Não há aproximação e não há quase."
