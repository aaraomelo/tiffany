## Mapa semântico — Mat2Q ↔ Qz ↔ banco

### Tipos

- **Mat2Q**: `{Qz a,b,c,d}` — [[a,b],[c,d]], por linhas, sobre Qz
- **V2Q**: `{Qz x, y}` — coluna (x,y) ∈ ℚ²
- **Qz**: `{int64_t p, q}` — p/q, reduzido, sinal no numerador, cabe em E₁₆ ou promove a int64
- **QzP**: `{I128 p, q}` — par I128, fora do int64
- **QzX**: `{Qz estreito, QzP promovido, int saturo}` — extensão para fora de E₁₆
- **Mat** (casa): `{int m, n; Qz a[LN_MAX][LN_MAX]}` — matriz n×n genérica sobre Qz
- **Vec** (casa): `{int n; Qz c[LN_MAX]}` — vector n sobre Qz

### Ponte (matriz2q_ponte.h)

| Direção | Função | Entrada | Saída | Observação |
|---|---|---|---|---|
| Mat2Q → Mat | `m2q_ponte_Mat` | Mat2Q (4 Qz) | Mat (2×2, Qz) | cópia direta, sem perda |
| Mat → Mat2Q | `m2q_ponte_Mat2Q` | Mat (2×2) | Mat2Q* | só se 2×2, senão 0 |
| V2Q → Vec | `m2q_ponte_Vec` | V2Q (2 Qz) | Vec (2, Qz) | cópia direta |
| Vec → V2Q | `m2q_ponte_V2Q` | Vec (2) | V2Q* | só se n==2 |

A ponte é **tradução sem perda, verificada por ida-e-volta** (§L: 39 travessias).

### Operações da casa sobre o objecto racional (§M)

| Operação | Função da casa | Objecto | Evidência |
|---|---|---|---|
| `A_q² = qA_q + I` | `mat_mult(A,A)` | Mat | 13 passagens |
| `det = −1` | `mat_det` | Mat | 13 passagens |
| `tr = q` | `esp_tr` | Mat | 13 passagens |
| `W² = ΔI` | `mat_mult(W,W)` | Mat | 13 passagens |
| `W⁻¹ = (1/Δ)W` | Gauss–Jordan da casa | Mat | q=15/4 |
| `B_q(u,v)` | `fb_av(W, u, v)` | Mat/Vec/Qz | §C |
| `Q_q(u)` | `fb_quadratica(W, u)` | Mat/Vec/Qz | §C |
| `A_qⁿ(1,0)` | `mat_aplica` | Mat | 65 passos |

### Onde Qz aparece no banco

**banco/sql.c:**
- `esp_disc(A)` (linha 12161): discriminante `tr²−4det` em inteiros — recusa se `tr.q≠1 || det.q≠1` (fora dos inteiros)
- `esp_racional(A, &l1, &l2)` (linha 12263): usa `esp_disc` + `raizi`; se falhar, SQL rejeita com mensagem "eigenvalues are not rational (disc %ld); use cifra(*)"
- `esp_diagonalizavel(J)` (linha 10373): em conversa.c, não em sql.c directamente

**banco/conversa.c:**
- `esp_racional` em múltiplos pontos (diagonalização, autovalores)
- `esp_diagonalizavel` como verificação

**lib/forma.h:**
- `esp_tr(Mat A)`: `qz_soma(A.a[0][0], A.a[1][1])`
- `esp_disc(Mat A)`: `tr.p*tr.p − 4*det.p` — **só em inteiros** (`tr.q!=1 || det.q!=1` → −1)
- `esp_racional`: usa `esp_disc` + `raizi` (raiz inteira da casa)
- `esp_diagonalizavel`: usa `esp_racional` + autovetores

**lib/linear.h:**
- `mat_mult`: convolução com índice interno sobre Qz
- `mat_det`: expansão, exacto para n≤4
- `mat_aplica`: aplica matriz a vector
- `mat_soma`, `mat_esc_neg`, `mat_transposta`, `mat_igual`

### Contrato documentado de forma.h

`esp_disc/esp_racional` **recusa** entradas não inteiras (`esp_disc = −1` quando `tr.q≠1 || det.q≠1`). A ponte não muda isso — é o ponto exacto onde a camada racional entra no lugar da máquina inteira (documentado em matriz2q_ponte.h:13-14).

### Setas do mapa

| Seta | Entrada | Transformação | Saída | Invariantes preservados | Evidência |
|---|---|---|---|---|---|
| Mat2Q → Mat | `{Qz a,b,c,d}` | ponte: cópia dos 4 Qz | `Mat{2,2,Qz[a][b;c;d]}` | cada coeficiente idêntico; operações da casa sobre o objecto racional reproduzem a camada | §L: 39 travessias ida-e-volta |
| Mat → Mat2Q | `Mat{2,2}` | ponte: inversa | `{Qz a,b,c,d}` | volta exacta; só 2×2 | §L: 39 travessias |
| V2Q → Vec | `{Qz x,y}` | ponte: cópia | `Vec{2,Qz[c0;c1]}` | cada coordenada idêntica | §L: 13 travessias |
| Vec → V2Q | `Vec{2}` | ponte: inversa | `{Qz x,y}` | volta exacta; só n==2 | §L: 13 travessias |
| Qz → Qz (casa) | `{p,q}` interno | operações da casa | `{p',q'}` | racional exacto; Qz é o tipo base de Mat e Vec | `linear.h` usa `qz_soma/mult/oposto/igual` |
| A_q (Mat2Q) → A_q (Mat) | `m2q_A(q)` | ponte | `Mat` | `A²=qA+I`, `det=−1`, `tr=q` | §M: 13 passagens |
| W_q (Mat2Q) → W_q (Mat) | `m2q_W(q)` | ponte | `Mat` | `W²=ΔI`, `det=−Δ` | §M: 13 passagens |
| B_q/Q_q (Mat2Q) → B_q/Q_q (casa) | `m2q_B/Q` | ponte + fb_av/fb_quadratica | `Qz` | `B_q(u,v)=uᵀW_qv`, `Q_q(u)=B_q(u,u)` | §M: fb_av/fb_quadratica |
| Split (Mat2Q) → Split (casa) | `m2q_split` | ponte → esp_disc/esp_racional | classificação | concordância nos inteiros | §S: inteiro concorda; racional recusa |

### A cadeia composta Word₈ → Mat2Q (consolidado em 27/09/2026)

`FECHADO/MEDIDO`. A cadeia que antes era **narrativa** — cada elo medido num
ficheiro diferente, sem que nenhum os ligasse — foi executada ponta a ponta. **Não
faltava ponte nenhuma**: todas as funções já existiam, e o medidor só revelou a
composição. **Adaptadores novos: zero.**

```
Word₈×Word₈ → E₁₆ → Qz → V2Q → Vec → Mat → Mat2Q      (a cadeia)
                                                   ↖ Mat (a volta, resíduo 0)
```

| Elo | Função | Ficheiro:linha | Consome a saída anterior? |
|---|---|---|---|
| Word₈×Word₈ → par E₁₆ | `w8_mult_larg(8,a,b)` | `palavra8.h:62` | — |
| par → valor de 16 bits | `lg_val(p,8)` | `largura.h:43` | sim |
| política de escrita | `w8_proj_sat(uint16)` | `naturais.h:155` | sim |
| E₁₆ → Qz | `qz_de_inteiro(n)` | `racionais.h:188` | sim |
| Qz,2 → V2Q | `m2q_v(x,y)` | `matriz2q.h:37` | sim |
| V2Q → Vec | `m2q_ponte_Vec(u)` | `matriz2q_ponte.h:31` | sim |
| Vec → Mat | `mat_de_colunas(v,k)` | `linear.h:231` | sim |
| Mat → Mat2Q | `m2q_ponte_Mat2Q(A,&M)` | `matriz2q_ponte.h:37` | sim |
| Mat2Q → Mat (resíduo) | `m2q_ponte_Mat(M)` | `matriz2q_ponte.h:24` | sim |

**Medição** (`tests/cadeia_word8_mat2q.c`, não rastreado): 16 pares · 16/16
exactos · 8 promoveram · 0 perdidos · 4 matrizes · resíduo 0 · **9 unidades, 0
falhas** · três execuções byte-idênticas · `git diff HEAD` vazio. O contador
`w8_saturou` concordou com a contagem independente de produtos acima de 255, e o
estreitamento para 16 bits é **total** (o maior produto de dois Word₈ é
255·255 = 65025 ≤ 65535).

**Controlo negativo:** perturbando uma referência da §4a numa cópia temporária, a
execução deu `exit 1` com **exactamente 1 falha**, a §4a. O teste pode falhar;
logo não é vazio.

#### Os cinco pontos semânticos que a cadeia obriga

1. **Isto NÃO é uma equivalência algébrica entre `Word₈` e `X₁ = Z/256Z`.** O que
   se mediu foi a composição de uma *cadeia de representações*. Nada aqui
   identifica as duas estruturas.

2. **O caso `128` separa-as executavelmente.** Medido:
   `w8_mul_f8(128,128) = 222` e `w8_inv_f8(128) = 238`, com `128·238 = 1` no
   `Word₈/GF(2⁸)` (`palavra8.h:43`, `:47`). Em `Z/256Z`, `128² ≡ 0 (mod 256)` e
   128 não tem inverso. Portanto o encontro em 256 continua a ser **encontro de
   suporte/rótulos**, não identificação estrutural. Não são só nomes diferentes:
   é o comportamento medido que diverge.

3. **O que compõe é a construção/representação, não a aritmética do corpo.** O
   produto que desce no caminho de promoção é o **numérico** (`lg_mult`, o mesmo
   de `promocao.c` §SP0/§SP3), **não** `w8_mul_f8`. São operações distintas e a
   cadeia não as atravessa uma na outra.

4. **`mat_de_colunas` põe as COLUNAS.** A Mat 2×2 sai `[[e0,e2],[e1,e3]]` — a
   coluna `v[j]` ocupa a coluna `j` (`linear.h:231`). Conferido de forma
   independente, escrito à mão, e não contra o que se esperava.

5. **`qz_de_inteiro` não introduz denominador arbitrário.** O contrato existente é
   a imersão `ℤ → ℚ`; o corpo da função **é** `qz(n,1)` (`racionais.h:188`), e é
   a mesma que `mat_de_inteiros` já usa (`linear.h:43`). O `q=1` não foi escolhido
   por esta medição — foi **herdado** da função que já existia.

#### `AINDA ABERTO` — e são problemas diferentes

| | Estado | Onde |
|---|---|---|
| Cadeia racional `Word₈ → … → Mat2Q` | **FECHADO / MEDIDO** | acima, 27/09/2026 |
| `X₁ = Z/256Z → X₂ = (Z/256Z)²` | **ABERTO** | `campos.tex:386`, `:479` |

O salto dimensional `ι₁ : X₁ ↪ X₂` (`campos.tex:479`) continua **sem implementação
e sem medição**, e é o primeiro degrau dimensional de `campos.tex`. **A cadeia
racional não é justificação para o implementar**: uma coisa é a composição de
representações que desce um valor, outra é a passagem que duplica a dimensão. Nada
no que foi medido toca em `ι_d`.

Continuam também por resolver, e **independentes desta frente**: `GF(2^16)` não
está construído (`BN_ANDARES 4`, `binario.h:42`, termina em `F₈`) e
`X₂ ≡ GF(2^16)` não é afirmado em lado nenhum. Mesmo cardinal (65536), naturezas
distintas: `(Z/256Z)²` é um módulo, `GF(2^16)` seria um corpo.

#### A colisão de nomes que explica o encontro em 256 (achado, NÃO aplicado)

O repositório usa **a mesma letra para estruturas diferentes**, e é isso que torna
o encontro em 256 enganador:

| Onde | O que o `X` / `F_8` é | Suporte |
|---|---|---|
| `can/reports/ADVERSARIAL_AUDIT_v2.md:138,186,256,269` | `X = Word_8 = F₈`, o átomo da torre | `arquitetura.tex:137` |
| `papers/racionais.tex:96` | `F₈ ≡ Word_8 = {0,…,255}` — o **corpo** | `:118` (as dobras de largura) |
| `redes/campos.tex:386` | `X_d := (Z/256Z)^d` — o **anel** | `:479` (`ι_d`) |

E há ainda um **terceiro** sentido do mesmo token: `campos.tex` escreve
`\mathcal F_8` para a **DFT-8** (`:193`, `:213`, `:230`).

Verificado por leitura: **`campos.tex` nunca menciona `Word_8`** nem o corpo
`F₈`; e **`racionais.tex` nunca menciona `Z/256`, `256Z` nem `mod 256`**. Não há
ponte textual entre os dois lados — e **também não há**, em nenhum dos dois
documentos, uma frase que diga que eles são diferentes. A medição do `128` (ponto
2 acima) torna essa não-identificação explícita e executável, mas fica **aqui**:
não foi escrita em `campos.tex`, em `racionais.tex` nem no audit, porque nenhum
deles foi tocado nesta etapa.

**Isto é um registo, não uma correcção.** O audit e os papers não foram tocados:
`ADVERSARIAL_AUDIT_v2.md` diz `BYTE = X = B⁸` **PROVADA**, e essa afirmação
refere-se ao `F₈` da torre — com a qual a medição concorda. O conflito, se existe,
é entre *dois documentos que usam a letra X para coisas diferentes*, e
desambiguá-lo é decisão de quem lê a teoria, não deste mapa.

#### Validação do próprio medidor (lição metodológica da auditoria)

**Não é falha da cadeia.** São três enganos do instrumento, e ficam registados
porque nenhum deles alterou o resultado — e porque dois deles teriam alterado se
eu não os tivesse visto:

1. **Falso verde no canal do relatório.** `ok()` **não aceita varargs**. Na
   primeira execução, dois textos de assert imprimiram `%d` literal — a
   condição era verdadeira, o veredicto era honesto, mas a linha de evidência
   prometia uma contagem que o mecanismo não consegue imprimir. Quem lesse essas
   linhas creria num número que nunca existiu. Já existia memória para isto
   (`feedback-o-numero-no-veredicto`); foi a **quarta** ocorrência. Ver [[nota]].
2. **O compilador já tinha dito, e eu li por cima.** O `gcc` emitiu
   `-Wformat` a dizer que faltava um argumento. Estava dentro de ~100 avisos de
   `-Wunused-function`, e o PowerShell tinha acabado de embrulhar o stream
   inteiro em `NativeCommandError` — de modo que a primeira leitura foi
   «isto é um erro», não «isto é um aviso de formato entre cem». Um teste que
   devolve verde não basta: **o mecanismo de medição também tem de ser validado.**
3. **Duas contagens que respondiam a perguntas diferentes.** `git status
   --porcelain` dá **126** e `--porcelain -uall` dá **127**: a segunda expande
   directórios que a primeira colapsa. O «125» registado antes não era
   contraditório, era a contagem colapsada. Nenhuma das duas é «o número» sem
   dizer **qual** pergunta se fez.

| | |
|---|---|
| Medidor | `tests/cadeia_word8_mat2q.c` (**não rastreado**) |
| SHA-256 do medidor | `EA0BF0315E1A2ABA995C025AF7B5EB5DE76109D24F06A4A34C9114A7C6A7EA1A` |
| SHA-256 da saída ×3 | `73307D81AFC5D80D793686523E61494BA66C639338DAD92B856C57D041E180E7` |
| blob git | `390d260db6dcb6128b78971b9755e3dea2296ccf` |
| HEAD | `70fc487e` |
| Compilar | `gcc -O2 -std=c99 -Ilib -o cadeia tests/cadeia_word8_mat2q.c -lm` |

[[nota]]: o medidor está **fora do git**. `git diff HEAD` vazio não significa
«nada mudou» — significa «nada versionado mudou», e este ficheiro, o `blob` acima
e o `SHA-256` são a única prova de que ele existiu. Ver
[[feedback-o-que-esta-no-disco-e-nao-no-git]].

### Distinções críticas

1. **Equivalência matemática**: Mat2Q ≅ subálgebra 2×2 de Mat(2,ℚ). Ambos representam o mesmo objecto algébrico. Isso é demonstrado pela ponte (ida-e-volta = identidade).

2. **Compatibilidade de representação**: Mat2Q e Mat partilham o mesmo tipo base (Qz). A ponte não converte tipos — copia Qz a Qz. A compatibilidade é total dentro do domínio Qz.

3. **Equivalência operacional dentro do banco**: `esp_disc`/`esp_racional`/`esp_diagonalizavel` operam sobre `Mat` (n×n), não sobre `Mat2Q`. A casa **recusa** racionais não inteiros em `esp_disc` (retorna −1). A Mat2Q **aceita** racionais. Há sobreposição no domínio inteiro, mas não equivalência total.

### O buraco (ABERTO #3)

O protocolo não pergunta: **"qual é a relação semântica exacta entre Mat2Q, Qz e o banco?"**

A evidência actual permite três respostas, não uma:

a) **Mat2Q é representação especializada** de uma classe de objectos que a casa já executa em Qz — a ponte preserva operações 2×2, mas a casa tem aritmética n×n genérica.

b) **Qz é o tipo base partilhado** — Mat2Q e Mat usam o mesmo Qz, mas Mat2Q restringe a 2×2 e preserva a distinção A_q/W_q/B_q/Q_q que a casa não impõe.

c) **A relação não é nem "ponte" nem "substituição"** — é uma **extensão de domínio**: Mat2Q cobre racionais não inteiros onde `esp_disc` recusa, e a ponte prova que, no domínio inteiro, são a mesma aritmética.

Nenhuma das três pode ser assumida sem contrato e medidor próprios.

### O que falta (para o próximo passo)

1. Contrato para a relação Mat2Q↔Qz↔banco (qual das três respostas, ou híbrida?)
2. Medidor que teste a sobreposição de domínio (inteiros vs racionais não inteiros)
3. Decisão sobre se Mat2Q substitui, estende, ou coexiste com a aritmética da casa
4. Teste da recusa `esp_disc=-1` para racionais não inteiros: é fronteira documentada ou lacuna?

**Isto não é a única frente aberta.** A cadeia
`Word₈ → E₁₆ → Qz → V2Q → Vec → Mat → Mat2Q` está **FECHADA e medida**
(ver *A cadeia composta*, 27/09/2026) e não altera nenhum dos quatro itens
acima — são da relação Mat2Q↔Qz↔banco. E o salto dimensional
`X₁ → X₂` (`campos.tex:479`) segue por medir, **sem implementação**.