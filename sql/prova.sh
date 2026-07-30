#!/usr/bin/env bash
# prova.sh — o esquema tem de RECUSAR o que os teoremas proíbem. Resíduo 0 ou falha.
# Um esquema de autorização não se prova mostrando que o certo passa: prova-se mostrando
# que o errado NÃO passa. Cada caso abaixo é uma tentativa de violação.
set -u
cd "$(dirname "$0")"
DB=/tmp/bai_prova.db; rm -f "$DB"
falhas=0
sqlite3 "$DB" < bai.sql || { echo "  o esquema nem carrega"; exit 2; }

exec_sql(){ sqlite3 "$DB" "PRAGMA foreign_keys=ON; $1" 2>&1; }
# aceita: tem de passar calado.  recusa: tem de abortar.
aceita(){ local o; o=$(exec_sql "$2"); if [ -n "$o" ]; then printf '      %-52s NÃO  ✗  (%s)\n' "$1" "$o"; falhas=$((falhas+1));
          else printf '      %-52s sim  ✓\n' "$1"; fi; }
recusa(){ local o; o=$(exec_sql "$2"); if [ -z "$o" ]; then printf '      %-52s NÃO  ✗  (passou!)\n' "$1"; falhas=$((falhas+1));
          else printf '      %-52s sim  ✓\n' "$1"; fi; }
vale(){ local g; g=$(exec_sql "$2"); if [ "$g" = "$3" ]; then printf '      %-52s sim  ✓\n' "$1";
        else printf '      %-52s NÃO  ✗  (deu %s, esperado %s)\n' "$1" "$g" "$3"; falhas=$((falhas+1)); fi; }

echo
echo "=== O ESQUEMA DA BAI: o errado tem de ser IMPOSSÍVEL ======================"
echo
echo "§E1  O retículo das condições, e a isomorfia (usuário e nó, o mesmo formalismo)."
echo
exec_sql "INSERT INTO tenant VALUES ('t_gk','goldenkingdom','Reino Dourado'),('t_out','outro','Tenant vizinho');"
exec_sql "INSERT INTO principal VALUES ('aarao','t_gk','usuario','Aarão'),('claude','t_gk','agente','Claude'),('gato','t_gk','no','o nó gato'),('estranho','t_out','usuario','Alguém do vizinho');"
exec_sql "INSERT INTO condicao VALUES ('tudo','o topo: qualquer contexto'),('teoria','os nós de teoria'),('gato_only','só o nó gato'),('outro','um ramo irmão');"
exec_sql "INSERT INTO refina VALUES ('teoria','tudo'),('gato_only','teoria'),('outro','tudo');"
vale "o fecho é reflexivo (toda condição refina a si)"        "SELECT COUNT(*) FROM refina_estrela WHERE filha=mae;" "4"
vale "o fecho é transitivo (gato_only refina tudo, 2 saltos)" "SELECT COUNT(*) FROM refina_estrela WHERE filha='gato_only' AND mae='tudo';" "1"
vale "e NÃO inventa: gato_only não refina o ramo irmão"       "SELECT COUNT(*) FROM refina_estrela WHERE filha='gato_only' AND mae='outro';" "0"
echo
echo "§E2  Não-escalada: não existe κ sem origem, e delegar só ESTREITA."
echo
aceita "raiz com autoridade intrínseca entra"          "INSERT INTO capacidade VALUES ('k0','aarao','corpus','tudo',3,1.0,NULL,1,'autoridade intrinseca');"
recusa "κ órfã (sem raiz e sem pai) é RECUSADA"        "INSERT INTO capacidade VALUES ('kx','claude','corpus','tudo',1,0.5,NULL,0,'do nada');"
aceita "delegação que estreita a condição entra"       "INSERT INTO capacidade VALUES ('k1','claude','corpus','teoria',2,0.8,'k0',0,'delegada por aarao');"
recusa "delegação que ALARGA a condição é RECUSADA"    "INSERT INTO capacidade VALUES ('k2','gato','corpus','tudo',1,0.5,'k1',0,'tentativa de escalada');"
recusa "delegação para um ramo IRMÃO é RECUSADA"       "INSERT INTO capacidade VALUES ('k3','gato','corpus','outro',1,0.5,'k1',0,'ramo lateral');"
recusa "ω que CRESCE ao descer é RECUSADO"             "INSERT INTO capacidade VALUES ('k4','gato','corpus','gato_only',1,0.9,'k1',0,'peso inflado');"
echo
echo "§E3  Propagação limitada: cada aresta gasta δ, e δ=0 não repassa."
echo
aceita "neto com δ decrementado entra"                 "INSERT INTO capacidade VALUES ('k5','gato','corpus','gato_only',1,0.7,'k1',0,'neto');"
recusa "δ que não decrementa é RECUSADO"               "INSERT INTO capacidade VALUES ('k6','gato','corpus','gato_only',2,0.5,'k1',0,'delta parado');"
aceita "bisneto com δ=0 entra (usa, mas não repassa)"  "INSERT INTO capacidade VALUES ('k7','aarao','corpus','gato_only',0,0.6,'k5',0,'bisneto');"
recusa "delegar a partir de δ=0 é RECUSADO"            "INSERT INTO capacidade VALUES ('k8','claude','corpus','gato_only',0,0.5,'k7',0,'fim da linha');"
vale  "a cadeia é consultável: k7 dista 3 da raiz"     "SELECT salto FROM cadeia WHERE id='k7' AND ancestral='k0';" "3"
echo
echo "§E3b  O TENANT: o usuário É o tenant, e atravessar a fronteira custa δ=0."
echo
recusa "cross-tenant com δ>0 é RECUSADO"               "INSERT INTO capacidade VALUES ('kt1','estranho','corpus','gato_only',1,0.5,'k1',0,'vazaria para o vizinho');"
aceita "cross-tenant com δ=0 entra (usa, não propaga)" "INSERT INTO capacidade VALUES ('kt2','estranho','corpus','gato_only',0,0.5,'k1',0,'compartilhamento controlado');"
recusa "e do δ=0 do vizinho ninguém herda"             "INSERT INTO capacidade VALUES ('kt3','estranho','corpus','gato_only',0,0.4,'kt2',0,'neto do vizinho');"
vale  "o tenant é consultável por capacidade"          "SELECT tenant FROM capacidade_tenant WHERE id='kt2';" "t_out"
echo
echo "§E4  Revogação em cascata: revogar a raiz alcança os netos."
echo
vale  "antes da revogação, 5 capacidades vivas"        "SELECT COUNT(*) FROM capacidade;" "5"
exec_sql "DELETE FROM capacidade WHERE id='k0';"
vale  "depois de revogar a raiz, sobra 0"              "SELECT COUNT(*) FROM capacidade;" "0"
echo
echo "§E5  Fail-closed: sem concessão, o colapso devolve ⊥ (nenhuma linha)."
echo
exec_sql "INSERT INTO capacidade VALUES ('r0','aarao','corpus','teoria',2,0.9,NULL,1,'raiz nova');"
vale  "quem tem concessão recebe resposta"             "SELECT COUNT(*) FROM colapso WHERE consulta='gato_only';" "1"
vale  "quem NÃO tem recebe ⊥, não recebe palpite"      "SELECT COUNT(*) FROM colapso WHERE consulta='outro';" "0"
vale  "e ⊥ não é erro: é conjunto vazio"               "SELECT COALESCE((SELECT capacidade FROM colapso WHERE consulta='outro'),'BOT');" "BOT"
echo
if [ "$falhas" -gt 0 ]; then echo "  FALHAS: $falhas"; echo; exit 1; fi
echo "  RESÍDUO 0 — o esquema recusa tudo o que os teoremas proíbem."
echo
