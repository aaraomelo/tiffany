#!/bin/sh
# Corre depois de CADA renovação bem sucedida (certbot renewal-hook "deploy").
#
# É preciso porque o cert da Patria é wildcard e renova-se com o plugin `manual`: o certbot
# escreve o ficheiro novo e mais nada — não é o plugin `nginx`, não recarrega ninguém. Sem
# isto o nginx continuaria a servir o cert velho em memória até alguém reparar, que é uma
# maneira lenta de repetir o que aconteceu entre Julho e Agosto de 2026.
set -e
nginx -t 2>/dev/null || { echo "nginx -t falhou; NÃO recarrego"; exit 1; }
systemctl reload nginx
echo "nginx recarregado com $RENEWED_LINEAGE"
