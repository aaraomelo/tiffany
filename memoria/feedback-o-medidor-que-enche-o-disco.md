---
name: feedback-o-medidor-que-enche-o-disco
description: O rodapé do unidade.h escreve para /tmp sem tecto — uma mutação que põe o medidor em ciclo enche o disco, e o timeout não impede.
metadata:
  type: feedback
---

# O medidor que enche o disco

17/08/2026. A meio de uma varredura do [[project-gume-automatico]] sobre a bateria inteira, o
`/tmp` chegou a **100% de 12 GB**. A causa:

    8,2 GB   /tmp/uni_354487.txt
    3,5 GB   /tmp/uni_232596.txt

O `unidade.h` escreve cada asserção para um ficheiro em `/tmp` **assim que ela acontece** — foi o
que resolveu os 12 KB de RAM por medidor, e está certo. Mas **não tem tecto**. Uma mutação do gume
pode pôr um laço a girar e a emitir unidades para sempre, e aí:

**o `timeout` mata o processo, mas o que ele já escreveu FICA.**

É a mesma família de [[feedback-o-teto-nao-verificado]] — um limite que ninguém verifica é
documentação, não limite — só que aqui o limite nem existia.

**A correcção, e ela vai no lado de quem CORRE e não de quem escreve:** o `gume.py` passou a correr
cada mutante com `RLIMIT_FSIZE` (64 MB) e `RLIMIT_AS` (2 GB) via `preexec_fn`, e a apagar os
`uni_*.txt` grandes que sobrem. A bateria já fazia metade disto (`ulimit -v 2000000`); faltava o
`-f`, e faltava no gume.

**A regra:** qualquer ferramenta que corra código MUTADO tem de o correr com tecto de escrita **e**
de memória. Código mutado não é código: é código com um defeito posto de propósito, e o defeito
pode ser «não termina».
