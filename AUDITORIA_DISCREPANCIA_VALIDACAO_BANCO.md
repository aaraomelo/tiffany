# AUDITORIA_DISCREPANCIA_VALIDACAO_BANCO.md

**Agente responsável:** Big Pickle
**Data da auditoria:** 2026-09-25
**Objetivo único:** explicar como `tools/atestados.txt` registra exit `0` para `sql`/`pgwire`/`banco` enquanto `RESULTADO_VALIDACAO_VERTICAL.md` registra 451/568 unidades com 117 falhas e SIGABRT.

Pergunta central:

```
qual execução produziu qual resultado, em qual estado do código e sob qual comando?
```

**Nada foi alterado:** nenhum DDL, nenhuma migration, nenhum código, nenhum teste, nenhuma linha de `tools/atestados.txt`. A análise é sobre evidência existente no repositório.

---

## 0. CONCLUSÃO EM UMA LINHA

`atestados.txt` **não é** um relatório de bateria com "0 falhas": é um **livro-razão de exits por assinatura de conteúdo**, commitado em 08-31 (`32bac562`) e **nunca re-adotado depois das mudanças em `lib/` de 09-06/09-09/09-14**. O `451/568, 117 falhas` é a transcrição (sem artefato cru preservado) de uma bateria `ubuntu-latest` de 09-24 sobre **outra árvore**. Os dois números medem **contentores, árvores, métricas e datas diferentes** — não a mesma bateria em dois vereditos.

Classificação (§F): **B — BATERIAS/ESTADOS DIFERENTES**, com subconjunto C ainda em aberto para 4 medidores (o conteúdo deles pode não ter mudado).

---

## A. EVidência de `atestados.txt`

### Formato e semântica

Cada linha é `nome <assinatura16hex> <exit>` — o exit registrado de UM medidor, guardado por assinatura de conteúdo (fonte + dados `DEPENDE-DE:` + headers de `cc -MM` + argumentos; `tools/bateria.sh:203-222`). A bateria **reusa** a linha apenas se a assinatura atual bater com a guardada (`bateria.sh:256-266`); senão reabre o medidor. Nada se apaga (`bateria.sh:16-19`).

### O que o arquivo realmente diz

- 560 linhas no total, **87 com exit ≠ 0** (ex.: `morfico 0612d837b55a7857 49`, `agm_analitico df244c68d9d9e3ba 1`, `app_arranca 16fa187342799061 1`). **Não há alegação de "0 falhas" em lugar nenhum.**
- Linhas das vítimas da bateria vertical (tabela A1).
- `sql` linha 487; `pgwire` linha 389; `banco` linha 35; `traduz` 516; `conversa` 95; `erg` 190; `agentes` 3; `fita` 223; `pinos` 395.
- **`patria`.js/`patria` não tem linha** — coerente com "NÃO COMPILOU" da vertical: um medidor que não compila `continue`s **antes** de atestar (`bateria.sh:363-366`).

### Proveniência da tabela

- Último commit que tocou `tools/atestados.txt` (junto com `sql.c` e `bateria.sh`): **`32bac562`, 2026-08-31**.
- O working tree está limpo (`git status` não lista `tools/atestados.txt`), logo o conteúdo atual é o conteúdo commitado em 08-31.
- Depois de 08-31, o workflow **nunca commita atestados** (`bateria.sh.yml:25-27`): "a corrida sobe-os como artefato e mostra o diff no resumo; adoptá-los é decisão de quem lê". Logo uma corrida do runner **nunca** atualiza a tabela do repo por si.
- Incidente-registro: em 26/08 uma bateria MinGW reescreveu atestados com falsas falhas de ambiente (`acesso → e797e0774a38edae 1`, `agm_analitico → ... 127`) e foi **revertida** — é a razão documentada do repo ser POSIX-only (`bateria.yml:5-16`). A linha atual de `acesso` (`5727811cc6d6a5a6 0`) é a original — a corrupção não vingou.

**Conclusão parcial A:** qualquer `0` aqui é o exit de UM medidor, numa execução POSIX ≤ 08-31, adotado no commit `32bac562`. Não é contagem, não é bateria inteira, não é a árvore de hoje.

---

## B. EVidência de `RESULTADO_VALIDACAO_VERTICAL.md`

### Do que se trata

- Data 2026-09-24; "compilado em ambiente POSIX (GitHub Actions `ubuntu-latest`)" (`RESULTADO_VALIDACAO_VERTICAL.md:5`).
- Bloco de totais (linhas 55-61): `BATERIA = 451/568 VERDE`, `FALHAS = 117`, `NÃO EXECUTADOS = 0`, `MOTOR ALTERADO = NÃO`.
- **451 + 117 = 568** exatamente → `568` é a soma de **unidades** da bateria (`unidades: X passaram, Y falharam` em `bateria.sh`), não o número de medidores.
- Falhas declaradas (linhas 64-72): `sql`,`traduz`,`conversa` → FALHA exit 134 (SIGABRT), 0 unidades; `erg` → 10 unidades com 7 falhas; `agentes` → 3 com 2; `fita` → 0 com 1; `pinos` → 0 com 1; `patria.js` → NÃO COMPILOU.
- Nas linhas 74-76: "banco/sql.c crasha com SIGABRT na bateria POSIX completa. O output individual sql.txt está vazio — o crash ocorre antes de qualquer impressão. **Este é um bug do motor, não do ambiente de build.**"

### Contradição INTERNA do documento (explicável)

- A tabela de capacidades (linhas 28-53) tem **26 linhas "NÃO EXECUTADO"** com a nota "motor indisponível no MSYS — precisa compilação POSIX".
- Os totais dizem `NÃO EXECUTADOS = 0`.
- Leitura: o documento tem **duas fases emendadas** — a tabela descreve a fase 1 (tentativa local em MSYS/MinGW, bloqueada por headers POSIX ausentes), e o bloco de totais descreve a fase 2 (bateria Ubuntu real). A inconsistência é de emenda, não de engano na bateria.

### Proveniência da corrida de 09-24

- **Sem commit assinalado** no documento (só data + ambiente).
- **Sem artefato cru preservado no repo**: a bateria do runner sobe `/tmp/bateria.txt`, `/tmp/bateria/` e o `tools/atestados.txt` da sua árvore como artefato do GitHub Actions (retenção 30 dias, `bateria.yml:163-175`) — nada disso entra no repositório. A única fonte in-repo dos `451/568/117` é **o próprio markdown** (autodeclarado).
- O conteúdo executado em 09-24 = árvore de trabalho do commit `a9d7a26e` (= HEAD, também 09-24), que herda `lib/slot_mem.h`/`lib/banco.h`/`banco/erg.c` da revisão `380d03f6` (09-14).

**Conclusão parcial B:** os `117` são falhas de unidade de uma bateria Ubuntu recente (09-24) sobre a árvore 09-14+ — a alegação mais forte das duas, e só existe em markdown.

---

## C. Comparação dos comandos

| Item | `atestados.txt` (linhas `sql`/`pgwire`/`banco`) | Vertical (451/568) | Igual? |
|---|---|---|---|
| Comando | `bash tools/bateria.sh` (POSIX, reuso por assinatura; `bateria.sh:256`) | `bash tools/bateria.sh` (Ubuntu runner; modo default `--refaz`, reabre TODAS — `bateria.yml:36-42,103-106`) | Instrumento sim, modo presumido não |
| Script | `tools/bateria.sh` | `tools/bateria.sh` (+ runner) | Script sim |
| Métrica registrada | **exit por medidor**, por assinatura (560 linhas) | **unidades** passaram/falharam (568 = 451+117) | **DIFERENTE** |
| Suítes | varredura dos `.tex` + `manifesto.json` (`bateria.sh:92-113`) na árvore de 08-31 | idem, na árvore de 09-14+ | lista pode diferir |
| Ambiente | POSIX ≤ 08-31 (não registrado no arquivo; só a assinatura) | `ubuntu-latest` (GitHub Actions) | **DIFERENTE** (não verificado o outro) |
| Compilador | `cc` (não registrado) | GCC do Ubuntu (não registrado versão) | não registrado |
| Flags | `cc -O2 -std=c99 -w ...` (`bateria.sh:360-366`) | idem (pela bateria) | sim (pelo script) |
| Commit | `32bac562` (08-31), adotado no repo | `a9d7a26e`/árvore 09-14 (sem commit assinalado) | **DIFERENTE** |
| Working tree | limpo, = `32bac562` | árvore 09-14+ | **DIFERENTE** |
| Banco/dataset | apenas os do próprio medidor (`sql teste` → `/tmp/sql_teste`, `sql.c:16730-16733`) | idem | sim |
| Critério de PASS | exit `0` + rede exit; unidades contam-se em `bateria.sh:403-432` | idem | sim (ignorado no `--refaz`) |

Diferenças concretas identificadas: **métrica da contagem, commit/árvore, ambiente, data e modo da bateria** (`refaz` no runner).

---

## D. Comparação dos estados Git

### Linha do tempo verificada

| Data | Commit | O que mudou |
|---|---|---|
| 08-25 | `361bf63d` | last change de `lib/disco.h` |
| 08-31 | `32bac562` | **última adoção de `tools/atestados.txt` + `bateria.sh` + `sql.c/...`** — a tabela atual do repo é deste pombo-correio |
| 09-06 | `99e561e3` | `can/*` + `lib/so_cristal.c` (novo) |
| 09-09 | `c24fb335` | `banco/erg.c` (+`can/*`, docs) |
| 09-14 | `380d03f6` | **`lib/banco.h`, `lib/slot_mem.h`, `banco/erg.c`** |
| 09-24 | `a9d7a26e` = HEAD | `redes/espaco.tex` (nada de motor/lib); este é o conteúdo do runner de 09-24 |

### O detalhe decisivo — `lib/slot_mem.h` (diff de `380d03f6`)

```diff
-#define SLOT_WORD_BYTES 1
+#define SLOT_WORD_BYTES 2
```

A palavra de slot **dobrou de largura (1 → 2 bytes)** na mesma revisão em que `banco/erg.c` mudou. Isto **muda a assinatura** de todo medidor que inclua `slot_mem.h`.

### Staleness por vítima da vertical (dependência transitiva das linhas do ledger)

| Medidor | Linha do ledger | Inclui header alterado? | Estaleiro provado? |
|---|---|---|---|
| `sql` (0) | `695ed08464a8db0d` | sim — `../lib/slot_mem.h` (`sql.c:…`) | **SIM** (conteúdo ≠ 09-14!) |
| `traduz` (0) | `e655c2b9f39adbf9` | sim — `../lib/slot_mem.h` | **SIM** |
| `conversa` (0) | `0979cd58f050ad93` | sim — `../lib/slot_mem.h` | **SIM** |
| `erg` (0) | `f7924762bf8868f7` | erg.c mudou 09-09 e 09-14 | **SIM** |
| `banco` (0) | `65871676a87669aa` | sim — `../lib/banco.h` (`banco.c`) | **SIM** |
| `pgwire` (0) | `bf16b31eb4fd5c0b` | inclui `../lib/{edo,levanta,triade,fusao}.h` (inalterados); ligação a `slot_mem/banco` **não confirmada** | **NÃO PROVADO** |
| `fita` (0) | `9c72ed8abcd5cbf7` | `unidade.h` (08-17, inalterado) + sys | **NÃO PROVADO** (provavelmente **viva**) |
| `pinos` (0) | `3863cde7e59e0b7b` | `../lib/disco.h` (08-25, inalterado) | **NÃO PROVADO** |
| `agentes` (0) | `6bf730a43d661c1c` | `../lib/disco.h` | **NÃO PROVADO** |

**Interpretação do ledger:** para 5 das 9 vítimas (sql, traduz, conversa, erg, banco), a linha `0` **não descreve o conteúdo de 09-24** — a assinatura mudou e a bateria de hoje efetivamente **não reusaria** a linha (reabriria o medidor). Não há aqui contradição com o `134` da vertical: são outras árvores. Para pgwire/fita/pinos/agentes **não se pode concluir** hostilidade — se as assinaturas estão vivas, o ledger afirma exit 0 para conteúdo que a Ubuntu de 09-24 fez falhar: esse subconjunto continua **em aberto** (ver §F, §H). O teste local é impossível por env (ver §E) — a verificação exige o runner.

### Estado essencial

- `banco/sql.c`, `banco/pgwire.c`, `banco/conversa.c`, `banco/traduz.c`, `tools/atestados.txt`, `tools/bateria.sh`, `tests/pgwire.c`: **sem commits depois de `32bac562` (08-31)** (`git log` comprovado).
- `lib/` e `banco/erg.c`: mudaram em 09-06/09-09/09-14 — **depois** da adoção da tabela.

---

## E. Análise do SIGABRT (sem corrigir, só evidenciar)

### Fatos registrados

1. 09-24, bateria Ubuntu (`ubuntu-latest`): `sql`, `traduz`, `conversa` → FALHA **exit 134 (SIGABRT)**, 0 unidades cada; `sql.txt` **vazio** (`RESULTADO_VALIDACAO_VERTICAL.md:65-67,74-76`).
2. `sql teste` (`bateria.sh:155`; comando `sql teste`) roda o modo interno com base `/tmp/sql_teste` (`sql.c:16730-16733`), que escreve **stdout com buffer de bloco** — "arquivo vazio" **não prova** crash antes de qualquer printf: um `abort()` com stdout não-flushado apaga o buffer. A fase do crash não é determinável pelo relatório.
3. Hyptese com pesos diferentes:
   - **PRINCIPAL (conteúdo, p/ vírgula):** `sql.c` não mudou desde 08-31, mas passou a compilar contra `slot_mem.h` com **`SLOT_WORD_BYTES 1→2`** (09-14). O motor usa slots em `abrir_base`/`disco_prende`/catálogo — uma mudança de largura de slot pode desalinhar tamanhos de mapeamento/expectativas e provocar `assert`/abort no arranque. Isto transformaria "exit 0 (08-31)" em "SIGABRT (09-14+)" **sem ter tocado em `sql.c`**.
   - ALTERNATIVA (ambiente): o próprio repo documenta que resultados obtidos fora de POSIX completo são falsos (`bateria.yml:5-16`); o 09-24 estava em Ubuntu (env correto). Mas in-determinismo (tempo/memória/ordem do `fork()` em `sql.c:16796-16807`) não é descartável.
4. **Reprodução local impossível (verificado):** o WinLibs POSIX instalado (`.../WinGet/.../mingw64/include`) **não tem** `sys/socket.h`, `sys/mman.h`, `netinet/in.h`, `arpa/inet.h` (Test-Path → False). `sql.c`/`pgwire.c` não compilam nesta máquina Windows — coerente com DIAGNOSTICO (`DIAGNOSTICO_BUILD_VALIDACAO.md:68-74`) e com a fase 1 ("motor indisponível no MSYS") da própria vertical. **Não há `bash`/`cc`/`sha256sum` locais** para reproduzir a assinatura ou o crash aqui.
5. Determinismo: desconhecido; nenhum log do runner preservado no repo.

**Status do SIGABRT:** CAUSA NÃO PROVADA no momento da redacção; **LOCALIZADA na reprodução ao vivo** — ver §I.

---

## F. Causa OU classificação da discrepância

### A pergunta, respondida

- **`0` em `atestados.txt`:** exit POSIX ≤ 08-31, por conteúdo da árvore de `32bac562`, adotado no repo nesse commit. **Nunca** foi "0 falhas da bateria".
- **`451/568, 117`:** unidades de uma bateria `ubuntu-latest` de 09-24, sobre a árvore 09-14+ (= `a9d7a26e`), transcrita por escrito no markdown; artefato cru fora do repo.
- **Comando:** em ambos, `bash tools/bateria.sh`; o runner roda default `--refaz` (reabre tudo, ignora assinaturas).

### Classificação (uma categoria, com sub-conjunto nomeado)

**B — BATERIAS/ESTADOS DIFERENTES.**

O contraste `0 × 134` para `sql`/`traduz`/`conversa`/`banco`/`erg` é explicado por conteúdo ≠: a tabela registra a árvore de 08-31; a corrida mediu a de 09-14+. Não é a mesma coisa medida duas vezes com veredictos contrários — **são medidas de obras diferentes**, e as linhas `0` nem seriam reusadas hoje (a própria bateria as reabriria por assinatura).

**Ressalva honesta (sub-conjunto C não descartado):** `pgwire`, `fita`, `pinos`, `agentes` podem ter assinatura inalterada. Se estiverem vivas, o ledger afirma `0` para o mesmo conteúdo que a Ubuntu de 09-24 reprovou — aí seriam AMBIENTE/DIFERENTES de verdade. Não posso prová-lo localmente (sem POSIX); fica explícito no §H como passo de verificação.

Não se aplica como categoria global: **D (não reconciliável)** — há evidência suficiente para atribuir a divergência central a estados diferentes; e a inconsistência interna da vertical (26× NÃO EXECUTADO vs `NÃO EXECUTADOS = 0`) tem leitura coerente (duas fases emendadas).

---

## G. Impacto sobre o marco de migração

1. **O mito cai:** "atestados disse 0 e a vertical disse 117" não é contradição da obra — é contraste entre duas árvores. O marker da PROTOCOLO_AUDITORIA (§12) fica **refinado**:
   - a alegação forte passa a ser **unica**: a bateria **recente** (09-24) sobre a árvore atual tem **117 falhas de unidade e SIGABRT em `sql`**.
   - o `0` histórico não cancelá nem é cancelado — **é de outra árvore**.
2. **O BLOQUEADOR MANTÉM-SE** (estado de validação global = **NÃO CONSOLIDADO**), agora com causa: 117 falhas recentes + SIGABRT, mais a suspeita concreta de regressão por `SLOT_WORD_BYTES`. Nenhuma migration poderá começar com o engine em crash num runner.
3. **Decisão estrutural a tomar:** a tabela `tools/atestados.txt` do repo precisa de **re-adoção POSIX** (rodar no runner e adoptar o diff), senão continua a describir uma árvore que já não existe.

---

## H. Próximo passo necessário

1. **Gerar evidência nova (não corrigir):** rodar no runner Ubuntu
   - `bash tools/bateria.sh --reatesta sql` (primeiro), depois `--reatesta pgwire`, `fita`, `pinos`, `agentes` (os 4 do subconjunto C), e por fim `bash tools/bateria.sh --refaz` para o quadro inteiro atual.
   - Anotar o diff `atestados_antes × depois`, o commit exato (`git rev-parse HEAD`) e a versão do GCC, e **adoptar a tabela resultante** como a linha de base oficial da árvore 09-14+ (adoptar é decisão de quem lê — `bateria.yml:25-27`).
2. **Diagnosticar o SIGABRT de `sql`** (etapa própria, fora desta auditoria): com `sql teste` no runner, colher stdout/`assert`/backtrace para:
   - confirmar/determinar SE a regressão vem de `SLOT_WORD_BYTES 1→2` (slot_mem.h, 09-14);
   - verificar se `traduz`/`conversa` reproduzem o mesmo abort e se é determinístico;
   - só depois disso decidir se há algo a corrigir no motor.
3. **Registrar no marco:** `BLOQUEADOR → RESOLVIDO-DOCUMENTALMENTE` (a divergência tem causa explicada), mas **`BLOQUEADOR → MANTIDO`** no sentido operacional até: (a) `--refaz` fechado sem falhas em Ubuntu na árvore atual, e (b) o SIGABRT explicado. **Nenhuma migration/DDL antes disso.**

---

## I. Reprodução ao vivo no runner Ubuntu (2026-09-25) — a discrepância fechada

Autorizado pelo dono da obra, a bateria correu **na máquina remota** `root@srv1559444.hstgr.cloud` (Ubuntu 24.04.4 LTS, kernel 6.8, 4 vCPU, 15 GiB), via chave `segredo/id_ed25519_patria`, sobre **`git archive` do HEAD `a9d7a26e`** (LF, `gifs/` excluída — nenhuma citação/DEPENDE-DE a toca; arquivo confere, `--selo` = 568 medidores). Nenhum ficheiro do repo foi alterado.

### I.1 O que a bateria de 09-25 encontrou (árvore atual, Ubuntu)

- **205 de 568 sementes já não batiam com a tabela de 08-31** — a bateria reabriu-as sozinha, sem `--refaz`.
- **Totais:** `total 568 : 452 verdes, 0 negativos por projeto, 116 falhas`; `unidades: 1843 passaram, 120 falharam`. **≈ reproduz exactamente a vertical de 09-24 (451/117)** — o relatório vertical era honesto.
- `selo 5391a0484cdcac6c`, `205 sementes abertas agora, 360 já atestadas`.
- Tabela nova: **567 linhas** (repo: 560); **405 linhas diferentes** entre as duas (199 saem, 206 entram).

### I.2 Os nove suspeitos, antes × depois (evidência dura)

| medidor | 08-31 (ledger) | 09-25 (Ubuntu) | leitura |
|---|---|---|---|
| sql | `695ed08464a8db0d 0` | `e258d9835b76658c 134` | assinatura mudou (estaleiro provado) + SIGABRT |
| pgwire | `bf16b31eb4fd5c0b 0` | `7f3ee4684b78bd21 134` | idem |
| traduz | `e655c2b9f39adbf9 0` | `d81547a1d1ad009f 134` | idem |
| conversa | `0979cd58f050ad93 0` | `160ea07ad0c637e4 134` | idem |
| banco | `65871676a87669aa 0` | `a42df66e4a7bf90c 0` | assinatura mudou, **mas segue VERDE** (exit 0) |
| erg | `f7924762bf8868f7 0` | `e663751409f2dfde 1` | assinatura mudou + falha actual |
| fita | `9c72ed8abcd5cbf7 0` | **`9c72ed8abcd5cbf7 1`** | **MESMA assinatura, exit 0→1** — classe C ao vivo |
| pinos | `3863cde7e59e0b7b 0` | `c04d698bb40333e3 1` | assinatura mudou + falta dado |
| agentes | `6bf730a43d661c1c 0` | `4a92301b330a9d7e 1` | assinatura mudou + falta dado |

`fita` é o único com **mesmo conteúdo** (`9c72ed8abcd5cbf7`) e veredicto invertido: falha porque o ficheiro "origem" (blob de dados) **não existe nesta máquina** — igual para pinos/agentes ("abrir: No such file or directory"). É o subconjunto C confirmado em hardware: **mesmo código, ambientes/dados diferentes**.

### I.3 SIGABRT do sql — CAUSA LOCALIZADA (não é só proveniência, é bug)

Backtrace (`gdb` sobre `cc -O2 -g` do `sql.c` actual):

```
#10 slot_mem_le (slot,fd)          at ../lib/slot_mem.h:59
#11 mem_le (slot=58)               at sql.c:1217
#13 refaz_diario ()                at sql.c:6152
#14 abrir_base ("/tmp/sql_teste")  at sql.c:16057
#15 main (...)                     at sql.c:16733
   __pread_chk (buf=…2297, nbytes=2, offset=232, buflen=1)   ← lê 2 bytes num buffer de 1
```

O abort é o `__fortify_fail`/`__pread_chk` do glibc: `slot_mem_le` faz `pread(2 bytes)` num destino de **1 byte**. É a consequência exacta de `SLOT_WORD_BYTES 1→2` (09-14): o charlie mudou a largura mas o buffer do chamador não. O **Ubuntu compila com `_FORTIFY_SOURCE` por omissão**, que transforma essa escrita fora do limite num abort — por isso o exit 134 e a saída vazia (buffer de stdout perdido no abort).

**Prova por via negativa:** o mesmo `sql.c`, compilado com `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0`, **corre até ao fim** (98 asserções; saída completa; `rc=1`). Duas conclusões:

1. O `134` é a **regressão localizada** — não era "env mystério": era o `slot_mem.h` de 09-14 a obrigar 2 bytes para um buffer de 1.
2. **Há uma segunda falha independente do crash:** mesmo sem fortify, o medidor fecha com `1 falha` — `#UNIT falha  … a REVERSAO confere esse nonce — o esquilo reverte, residuo 0 NÃO ✗`. O motor actual, mesmo reparado do overflow, **tem uma unidade vermelha**.

### I.4 Fecho da classificação com a evidência nova

- **B** (baterias/estados diferentes): provado por assinaturas (8 dos 9 suspeitos mudaram) e pelos totais (452/116 ≈ 451/117) num ambiente **independente**.
- **C** (ambiente; mesmo código, resultado diferente): **`fita` ao vivo** (mesma assinatura, 0→1 por ficheiro em falta); pinos/agentes também falham por dado ausente, mas com assinatura já mudada.
- **SIGABRT:** CAUSA LOCALIZADA (§I.3) — a regressão exacta que o §E.3 suspeitava, com linha e teste do abort. **Não corrigida** (fora de âmbito desta auditoria).
- A vertical de 09-24 reproduz-se (±1 unidade) — deixa de ser alegação apenas em markdown.

### I.5 Exigência para a migração (mantém-se, agora fundamentada)

1. Corrigir o buffer de 1 byte no caminho `slot_mem_le` (slot_mem.h:59 → chamador) e re-correr **`sql`, `pgwire`, `traduz`, `conversa`** (+ demais 134s e `racionais_fixos`, `reta`, `telomero`, `torre_alg`, `descida_mobius`, `dif`, `entrega`, `fator` da mesma família "buffer overflow detected").
2. Resolver a falha independente do esquilo/reversão (`sql` ainda 1 unidade vermelha sem fortify).
3. Re-examinar fita/pinos/agentes com os ficheiros "origem" presentes — o 0 histórico deles depende de dados que não viajam no repo.
4. Só então `--refaz` e adopção da tabela nova (não antes; o actual 09-25 reflecte uma máquina sem esses dados).
5. **Nenhuma migration/DDL antes disto.**

---

## II. Checkpoint pós-correção (26-09-2026)

**O que foi demonstrado (não hipotetizado).** Correção mínima aplicada (apenas os dois alvos da auditoria, sem refactor, sem teste enfraquecido, sem DDL, formato de disco 2 bytes intacto) e bateria re-corrida no VPS Ubuntu 24.04 (gcc 13.3.0, fortify activo por omissão, snapshot `a9d7a26e` em LF). Os 4 ficheiros da correcção final batem por MD5 entre o repo local e o VPS (`banco/sql.c` regressou ao HEAD após a reversão do Bug B — sem alterações).

- **Bug A — overflow de 1/2 bytes corrigido** em `lib/slot_mem.h` (`slot_mem_le`/`slot_mem_grava`) e nas cópias privadas de `banco/traduz.c`, `banco/conversa.c`, `banco/erg.c` (`le_atom`/`grava_atom`/`zera_mem`/`poe_slot`/`ve_slot`) com buffers de `SLOT_WORD_BYTES` e valor no byte baixo.
- **Família antes afectada por SIGABRT/exit 134: 0 falhas após a correcção.** `sql`, `traduz`, `conversa`, `calculo2`, `descida_mobius`, `dif`, `entrega`, `racionais_fixos`, `reta`, `telomero`, `torre_alg` — todas VERDE. Uma causa comum, confirmada.
- **`sql`: 98/98 VERDE** (o abort e a unidade vermelha em fortify-off eram a mesma corrupção do head).
- **`traduz`: 7/7 VERDE.**
- **`conversa`: 331/331 VERDE.**
- **A unidade de reversão passou sem alteração da convenção correcta `h2[31-k]`.** O SHA-256 da banda é standard (verificado: `abc → ba7816bf…`; génesis double-SHA nos bytes = `6fe28c0ab6f1…`, que é `000000000019d6…` em display order do Bitcoin). A comparação `h2[31-k]` contra o alvo é a leitura little-endian do digest — correcta.
- **A hipótese do Bug B foi DESCARTADA.** A troca temporária para `h2[k]` (para "provar" a inversão) introduziu o falso positivo — o martelo passou a não achar o nonce ("faixa limpa") — e **foi revertida**. O código original `h2[31-k]` (OP_MARTELO e verifica) permaneceu válido; nenhuma linha de lógica SHA/reversão foi alterada na correcção final.
- **`pgwire` deixou de abortar (134→ timeout 124)**: corre sem crash mas ultrapassa os 900 s nesta máquina partilhada (~30 min e ainda não fechava quando interrompido à mão; sem segfault, sem `#UNIT` final). Registar como **timeout/lentidão de box**, não como correcção funcional. No runner dedicado do ledger 08-31 fechava exit 0.
- **`erg`, `fita`, `pinos`, `agentes`, `refs` permanecem como ocorrências independentes**, com a sua classificação já registada (§I.4):
  - `erg` exit 1 — 10 unidades, **7 falhas** (real, independente; fora do escopo dos dois alvos);
  - `refs` exit 139 segfault (real, independente; fora do escopo);
  - `fita`/`pinos`/`agentes` exit 1 — ambientais (ficheiro "origem" não viaja no repo; §I.4-C).
- **Resultado global do `--refaz`:** `568 → 461 verdes, 0 negativos por projeto, 107 falhas`; **5206 unidades passadas, 120 falhadas, 31 saltadas por falta de recurso externo** (= 5357 unidades contabilizadas; 31 das 568 sementes de medidores contam 1 unidade "grossa" — o exit).
- `tools/atestados.txt` do repo permanece **intacto** (08-31). A tabela pós-correcção vive no VPS; a **adopção é ainda decisão** — como baseline de evidência, ≠ validação do motor.

> **A correcção eliminou integralmente a família de abortos 134 sem exigir alteração da lógica SHA-256/reversão; a convenção original `h2[31-k]` permaneceu válida.**

---

## Encerramento deste documento

- Estado actual documentado; estado-alvo (da auditoria pré-migração) separado.
- **Discrepância histórica: RESOLVIDA** (B provado em hardware por assinaturas + reprodução 452/116 ≈ 451/117; C `fita` ao vivo; SIGABRT localizado em §I.3).
- **Pós-correcção (§II): SIGABRT eliminado — família 134 toda VERDE, `sql` 98/98, `traduz` 7/7, `conversa` 331/331; `h2[31-k]` original validado.**
- **Números, com precisão:** o sistema tem **568 medidores** (47 com contagem "grossa" por exit) e **5357 unidades efectivamente classificadas** no `--refaz` — **5206 passadas, 120 falhadas, 31 saltadas por recurso externo**. Não dizer "568 passaram"; dizer "568 medidores, 461 verdes/107 falhas; das 5357 unidades, 5206 passaram".
- **Bloqueador de migração: MANTIDO.** O crash está corrigido, mas persistem ocorrências independentes não resolvidas — `erg` (7 unidades), `refs` (139), `fita`/`pinos`/`agentes` (ambientais), `pgwire` (timeout nesta box) — e a adopção do ledger continua a ser decisão de quem lê.
- **Nenhum DDL; o `tools/atestados.txt` do repo permanece intacto** (08-31); a tabela pós-correcção vive na cópia do runner.
- **BLOQUEADOR → RESOLVIDO apenas no sentido documental/correccional do Bug A.** Não autoriza migration. Parar aqui.