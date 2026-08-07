#!/bin/sh
# segredo.sh — O PORTÃO: nenhum segredo entra neste repositório.
#
# O repo é PÚBLICO e é servido por dumb HTTP em goldenkingdom.patriatechnology.com.
# Uma chave privada commitada fica exposta para sempre: o histórico do git guarda-a
# mesmo depois de apagada, e o clone público distribui-a a quem a quiser.
#
# Este medidor não confia no .gitignore — VERIFICA. Um .gitignore certo com um
# `git add -f` por cima passa despercebido, e é assim que estas coisas acontecem.
#
# Corre-se antes de publicar:  sh tools/segredo.sh
# Devolve 0 se está limpo, 1 se há alguma coisa a vazar.

cd "$(dirname "$0")/.." || exit 2
falhas=0

diz(){ printf '  %s\n' "$*"; }
mal(){ printf '  ✗ %s\n' "$*"; falhas=$((falhas+1)); }
bem(){ printf '  ✓ %s\n' "$*"; }

echo
echo "  O PORTÃO DOS SEGREDOS — o que o git VÊ, e não o que devia ver"
echo "  ─────────────────────────────────────────────────────────────"

# ═══ 1. ficheiros de segredo RASTREADOS ═════════════════════════════════════
# `git ls-files` lista o que está mesmo no índice — é a pergunta certa.
rast=$(git ls-files | grep -E '(^|/)(\.env$|\.env\.[^e]|.*\.pem$|.*\.key$|id_rsa|id_ed25519|segredo/)' | grep -v '\.env\.exemplo$')
if [ -n "$rast" ]; then
    mal "há ficheiros de segredo RASTREADOS pelo git:"
    printf '      %s\n' $rast
else
    bem "nenhum ficheiro de segredo está rastreado"
fi

# ═══ 2. o .gitignore cobre mesmo? ═══════════════════════════════════════════
for f in .env segredo/id_rsa_patria segredo/qualquer.pem; do
    if git check-ignore -q "$f" 2>/dev/null; then
        bem "ignorado: $f"
    else
        mal "NÃO ignorado: $f"
    fi
done
# e o modelo TEM de passar — senão ninguém sabe o que pôr no .env
if git check-ignore -q .env.exemplo 2>/dev/null; then
    mal ".env.exemplo está ignorado, e devia entrar (é o modelo, não tem segredo)"
else
    bem ".env.exemplo entra (é o modelo)"
fi

# ═══ 3. chaves privadas no CONTEÚDO dos ficheiros rastreados ════════════════
# O nome pode enganar; o conteúdo não. Um `notas.txt` com uma chave lá dentro
# passa por todas as regras de nome.
# O padrao MONTA-SE em pedacos de proposito. Escrito inteiro, este ficheiro
# aparecia na sua propria varredura — e a saida certa NAO e' excluir o
# tools/ por nome: isso abria um buraco onde se podia esconder uma chave.
BEG="-----BEGIN"; KEY="PRIVATE KEY-----"
achou=$(git ls-files -z | xargs -0 grep -lI -- "$BEG .*$KEY" 2>/dev/null)
if [ -n "$achou" ]; then
    mal "há CHAVE PRIVADA no conteúdo de ficheiros rastreados:"
    printf '      %s\n' $achou
else
    bem "nenhuma chave privada no conteúdo dos ficheiros rastreados"
fi

# ═══ 4. e no HISTÓRICO, que é o que não se apaga ════════════════════════════
# Se alguma vez entrou, apagá-la agora não basta: continua nos objectos.
hist=$(git log --all --oneline --name-only --pretty=format: 2>/dev/null \
       | sort -u | grep -E '(^|/)(\.env$|.*\.pem$|.*\.key$|id_rsa|id_ed25519)' | grep -v '\.env\.exemplo$')
if [ -n "$hist" ]; then
    mal "estes ficheiros JÁ ESTIVERAM no histórico (apagá-los agora não os tira de lá):"
    printf '      %s\n' $hist
else
    bem "o histórico nunca teve ficheiros de segredo"
fi

echo "  ─────────────────────────────────────────────────────────────"
if [ "$falhas" = "0" ]; then
    echo "  LIMPO — pode publicar."
    echo
    exit 0
fi
echo "  $falhas problema(s). NÃO PUBLIQUE antes de resolver."
echo
exit 1
